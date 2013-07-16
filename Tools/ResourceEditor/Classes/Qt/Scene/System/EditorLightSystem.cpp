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

#include "Scene/System/EditorLightSystem.h"
#include "Qt/Scene/SceneEditor2.h"
#include "Qt/Scene/SceneSignals.h"
#include "Commands2/CommandID.h"
#include "StringConstants.h"
#include "Constants.h"

using namespace DAVA;

EditorLightSystem::EditorLightSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
{
	Light *light = new Light();
	light->SetType(Light::TYPE_DIRECTIONAL);
		
	cameraLight = new DAVA::Entity();
	cameraLight->SetName(ResourceEditor::EDITOR_CAMERA_LIGHT);
	cameraLight->AddComponent(new LightComponent(light));
	light->Release();

	lightCountOnScene = 0;
	SetRequiredComponents(Component::LIGHT_COMPONENT);

	SetCameraLightEnabled(true);
}

EditorLightSystem::~EditorLightSystem()
{
	SafeRelease(cameraLight);
}


void EditorLightSystem::Update( DAVA::float32 /* timeElapsed */)
{
	if(isEnabled)
	{
		UpdateCameraLightPosition();
	}
}

void EditorLightSystem::ProcessCommand( const Command2 *command, bool redo )
{
}

void EditorLightSystem::UpdateCameraLightState()
{
	if(isEnabled && (lightCountOnScene == 0))
	{
		AddCameraLightOnScene();
	}
	else
	{
		RemoveCameraLightFromScene();
	}
}


void EditorLightSystem::UpdateCameraLightPosition()
{
	if(GetScene() && cameraLight && cameraLight->GetParent())
	{
//		SceneCameraSystem *cameraSystem	= ((SceneEditor2 *) GetScene())->cameraSystem;
//		Camera *camera = cameraSystem->curSceneCamera;
        Camera *camera = GetScene()->GetCurrentCamera();
		if(!camera) return;

		Matrix4 m = Matrix4::MakeTranslation(camera->GetPosition() + camera->GetLeft() * 20.f + camera->GetUp() * 20.f);
		cameraLight->SetLocalTransform(m);
	}
}

void EditorLightSystem::SetCameraLightEnabled( bool enabled )
{
	isEnabled = enabled;

	UpdateCameraLightState();
}


void EditorLightSystem::AddCameraLightOnScene()
{
//	SceneEditor2 *sc = (SceneEditor2 *)GetScene();
//	if(sc)
//	{
//		sc->AddEditorEntity(cameraLight);
//	}
    
    AddEditorEntity(cameraLight);
}


void EditorLightSystem::RemoveCameraLightFromScene()
{
	if(cameraLight && cameraLight->GetParent())
	{
		cameraLight->GetParent()->RemoveNode(cameraLight);
	}
}


void EditorLightSystem::AddEntity( DAVA::Entity * entity )
{
	DVASSERT(entity);

	lightCountOnScene += CountLightsForEntityRecursive(entity);

	if(isEnabled && (lightCountOnScene > 0))
	{
		RemoveCameraLightFromScene();
	}
}

void EditorLightSystem::RemoveEntity( DAVA::Entity * entity )
{
	DVASSERT(entity);

	lightCountOnScene -= CountLightsForEntityRecursive(entity);
	DVASSERT(lightCountOnScene >= 0);

	if(isEnabled && (lightCountOnScene == 0))
	{
		AddCameraLightOnScene();
	}
}

DAVA::int32 EditorLightSystem::CountLightsForEntityRecursive( DAVA::Entity *entity)
{
	int32 lightsCount = 0;
	if(entity != cameraLight && GetLight(entity))
	{
		++lightsCount;
	}

	int32 childrenCount = entity->GetChildrenCount();
	for(int32 i = 0; i < childrenCount; ++i)
	{
		lightsCount += CountLightsForEntityRecursive(entity->GetChild(i));
	}

	return lightsCount;
}

void EditorLightSystem::AddEditorEntity(DAVA::Entity *entity)
{
    Scene *sc = GetScene();
    
    if(sc->GetChildrenCount())
	{
		sc->InsertBeforeNode(entity, sc->GetChild(0));
	}
	else
	{
		sc->AddNode(entity);
	}
}
