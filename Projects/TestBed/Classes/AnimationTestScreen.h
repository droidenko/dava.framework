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


#ifndef __ANIMATION_TEST_SCREEN_H__
#define __ANIMATION_TEST_SCREEN_H__

#include "DAVAEngine.h"

using namespace DAVA;

class AnimationTestScreen : public UIScreen, public UIHierarchyDelegate
{
protected:
	~AnimationTestScreen(){}
public:
	void ButtonPressed(BaseObject * owner, void * v);
	
	virtual void LoadResources();
	virtual void UnloadResources();
	virtual void WillAppear();
	virtual void WillDisappear();
	
	virtual void Update(float32 timeElapsed);
	virtual void Draw(const UIGeometricData &geometricData);
	virtual void Input(UIEvent * touch);
	
    
    virtual bool IsNodeExpandable(UIHierarchy *forHierarchy, void *forNode);
    virtual int32 ChildrenCount(UIHierarchy *forHierarchy, void *forParent);
    virtual void *ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index);
    virtual UIHierarchyCell *CellForNode(UIHierarchy *forHierarchy, void *node);
    virtual void OnCellSelected(UIHierarchy *forHierarchy, UIHierarchyCell *selectedCell);

    
    UIHierarchy *hierarchy;
    
	// 3d engine scene
	Scene * scene;
	Camera * activeCamera;
    
	float32 currentTankAngle;
	bool inTouch;
	Vector2 touchStart;
	Vector2 touchCurrent;
	float32 touchTankAngle;
	float32 rotationSpeed;
	
	float32 startRotationInSec;

	Vector3 originalCameraPosition;

	UIJoypad * positionJoypad;
    UIJoypad * angleJoypad;
    UI3DView * scene3dView;
    
    Matrix4 aimUser;
    Vector2 oldTouchPoint;
    float32 viewXAngle, viewYAngle;
    Camera * cam;
    
    Vector3 targetPosition;
};

#endif // __ANIMATION_TEST_SCREEN_H__