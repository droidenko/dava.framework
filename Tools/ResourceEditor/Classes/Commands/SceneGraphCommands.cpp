#include "SceneGraphCommands.h"

#include "DAVAEngine.h"
#include "Qt/Scene/SceneDataManager.h"
#include "Qt/Scene/SceneData.h"
#include "EditorScene.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "CommandsManager.h"

#include "SceneEditor/SceneEditorScreenMain.h"
#include "SceneEditor/EditorBodyControl.h"

using namespace DAVA;

#define REMOVE_ROOT_NODES_COMMON_PROPERTY "editor.referenceToOwner"

CommandRemoveRootNodes::CommandRemoveRootNodes()
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandRemoveRootNodes::Execute()
{
	SceneData* activeScene = SceneDataManager::Instance()->SceneGetActive();
	Entity *node = activeScene->GetSelectedNode();

	CommandsManager::Instance()->ExecuteAndRelease(new CommandInternalRemoveSceneNode(node, true));
}


CommandLockAtObject::CommandLockAtObject()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandLockAtObject::Execute()
{
    SceneData * activeScene = SceneDataManager::Instance()->SceneGetActive();
    Entity *node = activeScene->GetSelectedNode();
    if(node)
    {
        activeScene->LockAtSelectedNode();
    }
    else
    {
        SetState(STATE_INVALID);
    }
}



CommandRemoveSceneNode::CommandRemoveSceneNode()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandRemoveSceneNode::Execute()
{
	SceneData* activeScene = SceneDataManager::Instance()->SceneGetActive();
	Entity *node = activeScene->GetSelectedNode();

	CommandsManager::Instance()->ExecuteAndRelease(new CommandInternalRemoveSceneNode(node, false));
}


CommandInternalRemoveSceneNode::CommandInternalRemoveSceneNode(Entity* node, bool removeSimilar)
:	Command(Command::COMMAND_UNDO_REDO)
{
	commandName = "Remove Object";

	if (removeSimilar)
		commandName += "s";

	if (!node || !node->GetParent())
		return;

	if (removeSimilar)
	{
		String referenceToOwner;
		Entity* nodeParent = node->GetParent();

		KeyedArchive *customProperties = node->GetCustomProperties();
		if(customProperties && customProperties->IsKeyExists(REMOVE_ROOT_NODES_COMMON_PROPERTY))
		{
			referenceToOwner = customProperties->GetString(REMOVE_ROOT_NODES_COMMON_PROPERTY);
		}

		nodesForDeletion.reserve(nodeParent->GetChildrenCount());

		for (int32 i = 0; i < nodeParent->GetChildrenCount(); ++i)
		{
			Entity* child = nodeParent->GetChild(i);

			customProperties = child->GetCustomProperties();
			if (customProperties && customProperties->IsKeyExists(REMOVE_ROOT_NODES_COMMON_PROPERTY))
			{
				if (customProperties->GetString(REMOVE_ROOT_NODES_COMMON_PROPERTY) == referenceToOwner)
				{
					RemoveNodeRec removeNodeRec;
					removeNodeRec.node = SafeRetain(child);
					removeNodeRec.nodeParent = nodeParent;

					int32 i = GetNodeIndex(removeNodeRec);
					if (i >= 0 && i < nodeParent->GetChildrenCount() - 1)
						removeNodeRec.insertBeforeNode = nodeParent->GetChild(i + 1);

					nodesForDeletion.push_back(removeNodeRec);
				}
			}
		}
	}
	else
	{
		RemoveNodeRec removeNodeRec;
		removeNodeRec.node = SafeRetain(node);
		removeNodeRec.nodeParent = node->GetParent();

		int32 i = GetNodeIndex(removeNodeRec);
		if (i >= 0 && i < removeNodeRec.nodeParent->GetChildrenCount() - 1)
			removeNodeRec.insertBeforeNode = removeNodeRec.nodeParent->GetChild(i + 1);
		nodesForDeletion.push_back(removeNodeRec);
	}

	selectedNode = node;
}

CommandInternalRemoveSceneNode::~CommandInternalRemoveSceneNode()
{
	for (uint32 i = 0; i < nodesForDeletion.size(); ++i)
	{
		SafeRelease(nodesForDeletion[i].node);
	}
	nodesForDeletion.clear();
}

void CommandInternalRemoveSceneNode::Execute()
{
	if (nodesForDeletion.size() == 0)
	{
		SetState(STATE_INVALID);
		return;
	}

	for (uint32 i = 0; i < nodesForDeletion.size(); ++i)
	{
		nodesForDeletion[i].nodeParent->RemoveNode(nodesForDeletion[i].node);
	}

	SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
	activeScene->RebuildSceneGraph();
}

void CommandInternalRemoveSceneNode::Cancel()
{
	if (nodesForDeletion.size() == 0)
		return;

	for (Vector<RemoveNodeRec>::reverse_iterator rIt = nodesForDeletion.rbegin(); rIt != nodesForDeletion.rend(); ++rIt)
	{
		if (rIt->insertBeforeNode)
			rIt->nodeParent->InsertBeforeNode(rIt->node, rIt->insertBeforeNode);
		else
			rIt->nodeParent->AddNode(rIt->node);
	}

	SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
	activeScene->RebuildSceneGraph();

	if (selectedNode)
	{
		SceneEditorScreenMain* screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
		DVASSERT(screen);

		EditorBodyControl* bodyControl = screen->FindCurrentBody()->bodyControl;
		if (bodyControl && !bodyControl->LandscapeEditorActive())
			activeScene->SelectNode(selectedNode);
	}
}

int32 CommandInternalRemoveSceneNode::GetNodeIndex(const RemoveNodeRec& nodeRec)
{
	if (!nodeRec.node || !nodeRec.nodeParent)
		return -1;

	int32 i = 0;
	for (; i < nodeRec.nodeParent->GetChildrenCount(); ++i)
	{
		if (nodeRec.nodeParent->GetChild(i) == nodeRec.node)
			return i;
	}

	return -1;
}


CommandDebugFlags::CommandDebugFlags()
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandDebugFlags::Execute()
{
    SceneData * activeScene = SceneDataManager::Instance()->SceneGetActive();
    Entity *node = activeScene->GetSelectedNode();
    if(node)
    {
        if (node->GetDebugFlags() & DebugRenderComponent::DEBUG_DRAW_ALL)
        {
            node->SetDebugFlags(0, true);
        }
        else
        {
            node->SetDebugFlags(DebugRenderComponent::DEBUG_DRAW_ALL, true);
        }
    }
    else
    {
        SetState(STATE_INVALID);
    }
}


