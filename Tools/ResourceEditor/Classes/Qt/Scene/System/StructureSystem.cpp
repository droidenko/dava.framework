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

#include "Commands2/EntityMoveCommand.h"
#include "Commands2/EntityRemoveCommand.h"

StructureSystem::StructureSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
	, locked(false)
{

}

StructureSystem::~StructureSystem()
{

}

void StructureSystem::Init()
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		// mark solid all entities, that has childs as solid
		for(DAVA::int32 i = 0; i < sceneEditor->GetChildrenCount(); ++i)
		{
			CheckAndMarkSolid(sceneEditor->GetChild(i));
			CheckAndMarkLocked(sceneEditor->GetChild(i));
		}
	}
}

void StructureSystem::Move(DAVA::Entity *entity, DAVA::Entity *newParent, DAVA::Entity *before)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		sceneEditor->Exec(new EntityMoveCommand(entity, newParent, before));
	}
}

void StructureSystem::Move(const EntityGroup *entityGroup, DAVA::Entity *newParent, DAVA::Entity *newBefore)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor && NULL != entityGroup)
	{
		EntityGroup toMove = *entityGroup;

		if(toMove.Size() > 1)
		{
			sceneEditor->BeginBatch("Move entities");
		}

		for(size_t i = 0; i < toMove.Size(); ++i)
		{
			sceneEditor->Exec(new EntityMoveCommand(toMove.GetEntity(i), newParent, newBefore));
		}

		if(toMove.Size() > 1)
		{
			sceneEditor->EndBatch();
		}
	}
}

void StructureSystem::Remove(DAVA::Entity *entity)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		sceneEditor->Exec(new EntityRemoveCommand(entity));
	}
}

void StructureSystem::Remove(const EntityGroup *entityGroup)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor && NULL != entityGroup)
	{
		EntityGroup toRemove = *entityGroup;

		if(toRemove.Size() > 1)
		{
			sceneEditor->BeginBatch("Remove entities");
		}

		for(size_t i = 0; i < toRemove.Size(); ++i)
		{
			sceneEditor->Exec(new EntityRemoveCommand(toRemove.GetEntity(i)));
		}

		if(toRemove.Size() > 1)
		{
			sceneEditor->EndBatch();
		}
	}
}

void StructureSystem::Lock()
{
	locked = true;
}

void StructureSystem::Unlock()
{
	locked = false;
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
	if(!locked)
	{
		DAVA::Entity *before = NULL;
		DAVA::Entity *parent = entity->GetParent();

		if(NULL != parent)
		{
			before = parent->GetNextChild(entity);
		}

		SceneSignals::Instance()->EmitMoved((SceneEditor2 *) GetScene(), entity);
	}
}

void StructureSystem::RemoveEntity(DAVA::Entity * entity)
{
	if(!locked)
	{
		SceneSignals::Instance()->EmitRemoved((SceneEditor2 *) GetScene(), entity);
	}
}

void StructureSystem::CheckAndMarkSolid(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
		if(entity->GetChildrenCount() > 0)
		{
			entity->SetSolid(true);

			for(DAVA::int32 i = 0; i < entity->GetChildrenCount(); ++i)
			{
				CheckAndMarkSolid(entity->GetChild(i));
			}
		}
		else
		{
			entity->SetSolid(false);
		}
	}
}

void StructureSystem::CheckAndMarkLocked(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
		// mark lod childs as locked
		if(NULL != entity->GetComponent(DAVA::Component::LOD_COMPONENT))
		{
			for(int i = 0; i < entity->GetChildrenCount(); ++i)
			{
				MarkLocked(entity->GetChild(i));
			}
		}
		else
		{
			for(int i = 0; i < entity->GetChildrenCount(); ++i)
			{
				CheckAndMarkLocked(entity->GetChild(i));
			}
		}
	}
}

void StructureSystem::MarkLocked(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
		entity->SetLocked(true);

		for(int i = 0; i < entity->GetChildrenCount(); ++i)
		{
			MarkLocked(entity->GetChild(i));
		}
	}
}
