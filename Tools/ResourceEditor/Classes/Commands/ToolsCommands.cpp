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

#include "ToolsCommands.h"

#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../Qt/Main/QtMainWindowHandler.h"


using namespace DAVA;


//Beast
CommandBeast::CommandBeast()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_BEAST)
{
}


void CommandBeast::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->ProcessBeast();
    }
}


CommandConvertToShadow::CommandConvertToShadow(DAVA::Entity * entity)
:   Command(Command::COMMAND_UNDO_REDO, CommandList::ID_COMMAND_CONVERT_TO_SHADOW)
{
	affectedEntity = entity;
    changedRenderBatch = 0;
}

void CommandConvertToShadow::Execute()
{
	if(affectedEntity)
	{
		RenderObject * ro = GetRenderObject(affectedEntity);
		if (NULL == ro)
		{
			// Yuri Coder, 2013/05/17. This Entity doesn't have Render Object and can't be converted to Shadow.
			// See also DF-1184.
			return;
		}

		if(ro->GetRenderBatchCount() == 1 && typeid(*(ro->GetRenderBatch(0))) == typeid(DAVA::RenderBatch))
		{
			SceneDataManager::Instance()->SceneGetActive()->SelectNode(0);

			ShadowVolume * shadowVolume = ro->CreateShadow();

            changedRenderBatch = ro->GetRenderBatch(0);
            changedRenderBatch->Retain();

			ro->RemoveRenderBatch(changedRenderBatch);
			ro->AddRenderBatch(shadowVolume);

			affectedEntity->SetLocalTransform(affectedEntity->GetLocalTransform());//just forced update of worldTransform

			SceneDataManager::Instance()->SceneGetActive()->SelectNode(affectedEntity);
		}
	}
}

void CommandConvertToShadow::Cancel()
{
    if(changedRenderBatch && affectedEntity)
    {
        RenderObject * ro = GetRenderObject(affectedEntity);
        if(ro && ro->GetRenderBatchCount() == 1 && typeid(*(ro->GetRenderBatch(0))) == typeid(DAVA::ShadowVolume))
        {
            SceneDataManager::Instance()->SceneGetActive()->SelectNode(0);

            RenderBatch * shadowVolume = ro->GetRenderBatch(0);
            ro->RemoveRenderBatch(shadowVolume);
            shadowVolume->Release();

            ro->AddRenderBatch(changedRenderBatch);
            changedRenderBatch->Release();

            affectedEntity->SetLocalTransform(affectedEntity->GetLocalTransform());

            SceneDataManager::Instance()->SceneGetActive()->SelectNode(affectedEntity);
        }
    }
}

DAVA::Set<DAVA::Entity*> CommandConvertToShadow::GetAffectedEntities()
{
	Set<Entity*> entities;
	if(affectedEntity)
	{
		entities.insert(affectedEntity);
	}

	return entities;
}
