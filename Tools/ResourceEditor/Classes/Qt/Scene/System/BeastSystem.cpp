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


#include "BeastSystem.h"


#define DEFAULT_STATICLIGHT_ENABLE_STATE false
#define DEFAULT_STATICLIGHT_CAST_SHADOWS_STATE false
#define DEFAULT_STATICLIGHT_RECEIVE_SHADOWS_STATE false

BeastSystem::BeastSystem(Scene* scene):SceneSystem(scene)
{}

BeastSystem::~BeastSystem(){}

void BeastSystem::AddEntity(Entity * entity)
{
	SetDefaultPropertyValues(entity);
}

void BeastSystem::SetDefaultPropertyValues(Entity * entity)
{
	DAVA::KeyedArchive* propertyList = entity->GetCustomProperties();
	
	if(!propertyList->IsKeyExists("editor.staticlight.enable"))
	{
		propertyList->SetBool("editor.staticlight.enable", DEFAULT_STATICLIGHT_ENABLE_STATE);
	}
	
	if(!propertyList->IsKeyExists("editor.staticlight.castshadows"))
	{
		propertyList->SetBool("editor.staticlight.castshadows", DEFAULT_STATICLIGHT_CAST_SHADOWS_STATE);
	}
	
	if(!propertyList->IsKeyExists("editor.staticlight.receiveshadows"))
	{
		propertyList->SetBool("editor.staticlight.receiveshadows", DEFAULT_STATICLIGHT_RECEIVE_SHADOWS_STATE);
	}
}

