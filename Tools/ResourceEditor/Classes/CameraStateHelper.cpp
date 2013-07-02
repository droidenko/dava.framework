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

#include "CameraStateHelper.h"
#include "StringConstants.h"

namespace DAVA {

CameraStateHelper::CameraStateHelper(Scene* scene)
{
	mainCamera = NULL;
	debugCamera = NULL;

	activeScene = scene;
	PushDebugCamera();
}

CameraStateHelper::~CameraStateHelper()
{
	PopDebugCamera();
}

void CameraStateHelper::PushDebugCamera()
{
	if (!activeScene)
	{
		return;
	}
	
	mainCamera = activeScene->FindByName(ResourceEditor::EDITOR_MAIN_CAMERA);
	if (mainCamera)
	{
		SafeRetain(mainCamera);
		activeScene->RemoveNode(mainCamera);
	}
		
	debugCamera = activeScene->FindByName(ResourceEditor::EDITOR_DEBUG_CAMERA);
	if (debugCamera)
	{
		SafeRetain(debugCamera);
		activeScene->RemoveNode(debugCamera);
	}

	RemoveDeepCamera();
}
	
void CameraStateHelper::PopDebugCamera()
{
	if (!activeScene)
	{
		return;
	}

	if (mainCamera)
	{
		activeScene->AddNode(mainCamera);
		SafeRelease(mainCamera);
	}

	if (debugCamera)
	{
		activeScene->AddNode(debugCamera);
		SafeRelease(debugCamera);
	}
		
	mainCamera = 0;
	debugCamera = 0;
}

void CameraStateHelper::RemoveDeepCamera()
{
	if (!activeScene)
	{
		return;
	}

	Entity * cam;
	cam = activeScene->FindByName(ResourceEditor::EDITOR_MAIN_CAMERA);
	while (cam)
	{
		cam->GetParent()->RemoveNode(cam);
		cam = activeScene->FindByName(ResourceEditor::EDITOR_MAIN_CAMERA);
	}

	cam = activeScene->FindByName(ResourceEditor::EDITOR_DEBUG_CAMERA);
	while (cam)
	{
		cam->GetParent()->RemoveNode(cam);
		cam = activeScene->FindByName(ResourceEditor::EDITOR_DEBUG_CAMERA);
	}
}

};
