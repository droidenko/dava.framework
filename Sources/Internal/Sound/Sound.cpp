/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Sound/Sound.h"
#include "Sound/SoundSystem.h"
#include "Sound/SoundGroup.h"
#include "Sound/FMODUtils.h"

namespace DAVA
{
FMOD_RESULT F_CALLBACK SoundInstanceEndPlaying(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2);

Sound * Sound::Create(const FilePath & fileName, eType type, const FastName & groupName, int32 priority /* = 128 */)
{
	return CreateWithFlags(fileName, type, groupName, 0, priority);
}

Sound * Sound::Create3D(const FilePath & fileName, eType type, const FastName & groupName, int32 priority)
{
	return CreateWithFlags(fileName, type, groupName, FMOD_3D, priority);
}

Sound * Sound::CreateWithFlags(const FilePath & fileName, eType type, const FastName & groupName, int32 flags, int32 priority)
{
	Sound * sound = new Sound(fileName, type, priority);

	if(flags & FMOD_3D)
		sound->is3d = true;

    File * file = File::Create(fileName, File::OPEN | File::READ);
    int32 fileSize = file->GetSize();
    sound->soundData = new uint8[fileSize];
    file->Read(sound->soundData, fileSize);
    SafeRelease(file);

    FMOD_CREATESOUNDEXINFO exInfo;
    memset(&exInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    exInfo.length = fileSize;

	switch (type)
	{
	case TYPE_STATIC:
        FMOD_VERIFY(SoundSystem::Instance()->fmodSystem->createSound((char *)sound->soundData, FMOD_LOOP_NORMAL | FMOD_OPENMEMORY | flags, &exInfo, &sound->fmodSound));
        SafeDelete(sound->soundData);
		break;
	case TYPE_STREAMED:
		FMOD_VERIFY(SoundSystem::Instance()->fmodSystem->createStream((char *)sound->soundData, FMOD_LOOP_NORMAL | FMOD_OPENMEMORY | flags, &exInfo, &sound->fmodSound));
		break;
	}

	sound->SetSoundGroup(groupName);
	sound->SetLoopCount(0);

	FMOD_VERIFY(SoundSystem::Instance()->fmodSystem->createChannelGroup(0, &sound->fmodInstanceGroup));

	return sound;
}
Sound::Sound(const FilePath & _fileName, eType _type, int32 _priority)
:	fileName(_fileName),
	type(_type),
	priority(_priority),
	is3d(false),
    soundData(0)
{
}

Sound::~Sound()
{
    SafeDeleteArray(soundData);

	FMOD_VERIFY(fmodInstanceGroup->release());
	FMOD_VERIFY(fmodSound->release());
}

void Sound::SetSoundGroup(const FastName & groupName)
{
	SoundGroup * soundGroup = SoundSystem::Instance()->CreateSoundGroup(groupName);
	if(soundGroup)
	{
		FMOD_VERIFY(fmodSound->setSoundGroup(soundGroup->fmodSoundGroup));
	}
}

void Sound::Play()
{
	FMOD::Channel * fmodInstance = 0;
	FMOD_VERIFY(SoundSystem::Instance()->fmodSystem->playSound(FMOD_CHANNEL_FREE, fmodSound, true, &fmodInstance)); //start sound paused
	FMOD_VECTOR pos = {position.x, position.y, position.z};
	FMOD_VERIFY(fmodInstance->setPriority(priority));
	FMOD_VERIFY(fmodInstance->setCallback(SoundInstanceEndPlaying));
	FMOD_VERIFY(fmodInstance->setUserData(this));
	FMOD_VERIFY(fmodInstance->setChannelGroup(fmodInstanceGroup));

	if(is3d)
		FMOD_VERIFY(fmodInstance->set3DAttributes(&pos, 0));

	FMOD_VERIFY(fmodInstance->setPaused(false));
}

void Sound::SetPosition(const Vector3 & _position)
{
	position = _position;
}

void Sound::UpdateInstancesPosition()
{
	if(is3d)
	{
		FMOD_VECTOR pos = {position.x, position.y, position.z};
		int32 instancesCount = 0;
		FMOD_VERIFY(fmodInstanceGroup->getNumChannels(&instancesCount));
		for(int32 i = 0; i < instancesCount; i++)
		{
			FMOD::Channel * inst = 0;
			FMOD_VERIFY(fmodInstanceGroup->getChannel(i, &inst));
			FMOD_VERIFY(inst->set3DAttributes(&pos, 0));
		}
	}
}

Sound::eType Sound::GetType() const
{
	return type;
}

void Sound::SetVolume(float32 volume)
{
	FMOD_VERIFY(fmodInstanceGroup->setVolume(volume));
}

float32	Sound::GetVolume()
{
	float32 volume = 0;
	FMOD_VERIFY(fmodInstanceGroup->getVolume(&volume));
	return volume;
}

void Sound::Pause(bool isPaused)
{
	FMOD_VERIFY(fmodInstanceGroup->setPaused(isPaused));
}

bool Sound::IsPaused()
{
	bool isPaused = false;
	FMOD_VERIFY(fmodInstanceGroup->getPaused(&isPaused));
	return isPaused;
}

void Sound::Stop()
{
	FMOD_VERIFY(fmodInstanceGroup->stop());
}

int32 Sound::GetLoopCount() const
{
	int32 loopCount;
	FMOD_VERIFY(fmodSound->getLoopCount(&loopCount));
	return loopCount;
}

void Sound::SetLoopCount(int32 loopCount)
{
	FMOD_VERIFY(fmodSound->setLoopCount(loopCount));
}

void Sound::PerformPlaybackComplete()
{
	eventDispatcher.PerformEvent(EVENT_SOUND_COMPLETED, this);
}

FMOD_RESULT F_CALLBACK SoundInstanceEndPlaying(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2)
{
	if(type == FMOD_CHANNEL_CALLBACKTYPE_END)
	{
		FMOD::Channel *cppchannel = (FMOD::Channel *)channel;
		Sound * sound = 0;
		FMOD_VERIFY(cppchannel->getUserData((void**)&sound));
		sound->PerformPlaybackComplete();
	}

	return FMOD_OK;
}

};
