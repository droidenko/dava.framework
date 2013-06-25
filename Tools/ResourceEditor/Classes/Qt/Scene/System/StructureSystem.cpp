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

#include "Scene/System/StructureSystem.h"
#include "Scene/SceneSignals.h"
#include "Scene/SceneEditor2.h"

#include "Commands2/EntityAddCommand.h"
#include "Commands2/EntityInsertCommand.h"
#include "Commands2/EntityMoveCommand.h"
#include "Commands2/EntityRemoveCommand.h"

StructureSystem::StructureSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
{

}

StructureSystem::~StructureSystem()
{

}

void StructureSystem::Add(DAVA::Entity *entity, DAVA::Entity *parent /*= NULL*/)
{

}

void StructureSystem::Remove(DAVA::Entity *entity)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		sceneEditor->Exec(new EntityRemoveCommand(entity));
	}
}

void StructureSystem::Move(DAVA::Entity *entity, DAVA::Entity *newParent)
{

}

void StructureSystem::Update(DAVA::float32 timeElapsed)
{

}

void StructureSystem::Draw()
{

}

void StructureSystem::ProcessUIEvent(DAVA::UIEvent *event)
{

}

void StructureSystem::PropeccCommand(const Command2 *command, bool redo)
{

}

void StructureSystem::AddEntity(DAVA::Entity * entity)
{
	SceneSignals::Instance()->EmitAdded((SceneEditor2 *) GetScene(), entity);
}

void StructureSystem::RemoveEntity(DAVA::Entity * entity)
{
	SceneSignals::Instance()->EmitRemoved((SceneEditor2 *) GetScene(), entity);
}
