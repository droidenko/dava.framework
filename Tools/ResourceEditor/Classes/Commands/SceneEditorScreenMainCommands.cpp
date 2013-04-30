#include "SceneEditorScreenMainCommands.h"
#include "SceneEditor/SceneEditorScreenMain.h"
#include "SceneEditor/EditorBodyControl.h"
#include "Qt/Scene/SceneDataManager.h"
#include "Qt/Scene/SceneData.h"

CommandCreateNodeSceneEditor::CommandCreateNodeSceneEditor(DAVA::Entity* node)
:	Command(COMMAND_UNDO_REDO)
{
	commandName = "Create Node";
	this->node = SafeRetain(node);
}

CommandCreateNodeSceneEditor::~CommandCreateNodeSceneEditor()
{
	SafeRelease(node);
}

void CommandCreateNodeSceneEditor::Execute()
{
	if (node)
	{
		SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
		if(screen)
		{
			screen->FindCurrentBody()->bodyControl->AddNode(node);
		}
	}
	else
	{
		SetState(STATE_INVALID);
	}
}

void CommandCreateNodeSceneEditor::Cancel()
{
	if (node)
	{
		SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
		if(screen)
		{
			SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
			if (activeScene)
				activeScene->RemoveSceneNode(node);
		}
	}
}
