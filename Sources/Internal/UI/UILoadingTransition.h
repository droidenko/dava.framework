/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#ifndef __DAVAENGINE_UI_LOADING_TRANSITION_H__
#define __DAVAENGINE_UI_LOADING_TRANSITION_H__

#include "Base/BaseTypes.h"
#include "Platform/Thread.h"
#include "UI/UIScreenTransition.h"

namespace DAVA
{

class UILoadingTransition : public UIScreenTransition
{
public:
	UILoadingTransition();
	virtual ~UILoadingTransition();
	
	// Setup of default loading screen
	void	SetInTransition(UIScreenTransition * transition);
	void	SetOutTransition(UIScreenTransition * transition);
	
	void	SetBackgroundSprite(Sprite * sprite);
	void	SetAnimationSprite(Sprite * animationSprite);
	void	SetAnimationDuration(float32 durationInSeconds);
	
	virtual void DidAppear();
	virtual void StartTransition(UIScreen * _prevScreen, UIScreen * _nextScreen);
	virtual void Update(float32 timeElapsed);
	virtual void Draw(const UIGeometricData &geometricData);
	virtual int32 GetGroupId();
	virtual void WillAppear();
	virtual void WillDisappear();
	
	inline UIScreenTransition * GetInTransition();	
	
	bool IsLoadingTransition();
	bool IsTransitionInProcess();
	
protected:
	Sprite * backgroundSprite;
	Sprite * animationSprite;
	float32 animationTime;
	float32 animationDuration;
    bool transitionInProcess;

	UIScreenTransition * inTransition;
	UIScreenTransition * outTransition;

	void	ThreadMessage(BaseObject * obj, void * userData, void *callerData);
	Thread * thread;

private:
	virtual void SetDuration(float32 timeInSeconds);
};
	
inline UIScreenTransition * UILoadingTransition::GetInTransition()	
{
	return inTransition;
}

};



#endif // __DAVAENGINE_UI_LOADING_TRANSITION_H__