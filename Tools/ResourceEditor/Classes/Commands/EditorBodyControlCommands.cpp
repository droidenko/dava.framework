#include "EditorBodyControlCommands.h"
#include "SceneEditor/EditorBodyControl.h"
#include "SceneEditor/SceneEditorScreenMain.h"

#include "CommandsManager.h"

#include "Qt/Scene/SceneDataManager.h"
#include "Qt/Scene/SceneData.h"
#include "Qt/Scene/SceneEditorProxy.h"
#include "Qt/Scene/System/CollisionSystem.h"

CommandTransformObject::CommandTransformObject(DAVA::Entity* node, const DAVA::Matrix4& originalTransform, const DAVA::Matrix4& finalTransform)
:	Command(COMMAND_UNDO_REDO)
{
	commandName = "Transform Object";

	undoTransform = originalTransform;
	redoTransform = finalTransform;
	this->node = node;
}

void CommandTransformObject::Execute()
{
	if (node)
	{
		node->SetLocalTransform(redoTransform);
		UpdateCollision();
	}
	else
		SetState(STATE_INVALID);
}

void CommandTransformObject::Cancel()
{
	if (node)
	{
		node->SetLocalTransform(undoTransform);
		UpdateCollision();
	}
}

void CommandTransformObject::UpdateCollision()
{
	SceneEditorProxy *sep = dynamic_cast<SceneEditorProxy *>(node->GetScene());
	if(NULL != sep && NULL != sep->collisionSystem)
	{
		// make sure that worldtransform is up to date
		sep->transformSystem->Process();

		// update bullet object
		sep->collisionSystem->UpdateCollisionObject(node);
	}
}

CommandCloneObject::CommandCloneObject(DAVA::Entity* node, EditorBodyControl* bodyControl, btCollisionWorld* collisionWorld)
:	Command(COMMAND_UNDO_REDO)
,	clonedNode(NULL)
,	collisionWorld(collisionWorld)
{
	commandName = "Clone Object";

	originalNode = node;
	this->bodyControl = bodyControl;
}

CommandCloneObject::~CommandCloneObject()
{
	SafeRelease(clonedNode);
}

void CommandCloneObject::Execute()
{
	if (originalNode && bodyControl)
	{
		if (!clonedNode)
		{
			clonedNode = originalNode->Clone();

			if (collisionWorld)
				UpdateCollision(clonedNode);
		}

		originalNode->GetParent()->AddNode(clonedNode);
		bodyControl->SelectNode(clonedNode);
	}
	else
		SetState(STATE_INVALID);
}

void CommandCloneObject::Cancel()
{
	if (originalNode && clonedNode)
	{
		clonedNode->GetParent()->RemoveNode(clonedNode);

		// rebuild scene graph after removing node
		SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
		activeScene->RebuildSceneGraph();

		if (bodyControl)
			bodyControl->SelectNode(originalNode);
	}
}

void CommandCloneObject::UpdateCollision(DAVA::Entity *node)
{
	DVASSERT(node && collisionWorld);
	
	BulletComponent* bc = dynamic_cast<BulletComponent*>(node->GetComponent(Component::BULLET_COMPONENT));
	if (bc && !bc->GetBulletObject())
	{
		bc->SetBulletObject(ScopedPtr<BulletObject>(new BulletObject(node->GetScene(),
																	 collisionWorld,
																	 node,
																	 node->GetWorldTransform())));
	}
	
	for(int32 i = 0; i < node->GetChildrenCount(); ++i)
	{
		UpdateCollision(node->GetChild(i));
	}
}

Entity* CommandCloneObject::GetClonedNode()
{
	return clonedNode;
}


CommandCloneAndTransform::CommandCloneAndTransform(DAVA::Entity* originalNode,
												   const DAVA::Matrix4& finalTransform,
												   EditorBodyControl* bodyControl,
												   btCollisionWorld* collisionWorld)
:	MultiCommand(COMMAND_UNDO_REDO)
,	clonedNode(0)
,	cloneCmd(0)
,	transformCmd(0)
{
	commandName = "Clone Object";

	this->originalNode = originalNode;
	this->bodyControl = bodyControl;
	this->collisionWorld = collisionWorld;
	this->transform = finalTransform;
}

CommandCloneAndTransform::~CommandCloneAndTransform()
{
	SafeRelease(transformCmd);
	SafeRelease(cloneCmd);
}

void CommandCloneAndTransform::Execute()
{
	if (!cloneCmd)
	{
		cloneCmd = new CommandCloneObject(originalNode, bodyControl, collisionWorld);
	}
	ExecuteInternal(cloneCmd);

	if (GetInternalCommandState(cloneCmd) != STATE_VALID)
	{
		SetState(STATE_INVALID);
		return;
	}

	if (!transformCmd)
	{
		transformCmd = new CommandTransformObject(cloneCmd->GetClonedNode(), originalNode->GetLocalTransform(), transform);
		// Need to apply transform only once when creating command
		ExecuteInternal(transformCmd);

		if (GetInternalCommandState(transformCmd) != STATE_VALID)
		{
			SetState(STATE_INVALID);
			return;
		}
	}
}

void CommandCloneAndTransform::Cancel()
{
	// No need to undo transform command, removed node will appear at the same place where it was removed
	if (cloneCmd)
	{
		CancelInternal(cloneCmd);
	}
}


CommandPlaceOnLandscape::CommandPlaceOnLandscape(DAVA::Entity* node, EditorBodyControl* bodyControl)
:	Command(COMMAND_UNDO_REDO)
,	node(node)
,	bodyControl(bodyControl)
{
	commandName = "Place On Landscape";

	redoTransform.Identity();

	if (node)
		undoTransform = node->GetLocalTransform();
}

void CommandPlaceOnLandscape::Execute()
{
	if (node && bodyControl)
	{
		redoTransform = node->GetLocalTransform() * bodyControl->GetLandscapeOffset(node->GetWorldTransform());
		node->SetLocalTransform(redoTransform);
	}
	else
		SetState(STATE_INVALID);
}

void CommandPlaceOnLandscape::Cancel()
{
	if (node)
	{
		node->SetLocalTransform(undoTransform);
	}
}


CommandRestoreOriginalTransform::CommandRestoreOriginalTransform(DAVA::Entity* node)
:	Command(COMMAND_UNDO_REDO)
,	node(node)
{
	commandName = "Restore Original Transform";

	if (node)
	{
		StoreCurrentTransform(node);
	}
}

void CommandRestoreOriginalTransform::Execute()
{
	if (node)
	{
		node->RestoreOriginalTransforms();
	}
	else
		SetState(STATE_INVALID);
}

void CommandRestoreOriginalTransform::Cancel()
{
	if (node)
	{
		RestoreTransform(node);
	}
}

void CommandRestoreOriginalTransform::StoreCurrentTransform(DAVA::Entity *node)
{
	if (node)
	{
		undoTransforms[node] = node->GetLocalTransform();

		for (int32 i = 0; i < node->GetChildrenCount(); ++i)
			StoreCurrentTransform(node->GetChild(i));
	}
}

void CommandRestoreOriginalTransform::RestoreTransform(DAVA::Entity *node)
{
	if (node)
	{
		Map<Entity*, Matrix4>::iterator it = undoTransforms.find(node);
		if (it != undoTransforms.end())
		{
			node->SetLocalTransform((*it).second);
		}

		for (int32 i = 0; i < node->GetChildrenCount(); ++i)
		{
			RestoreTransform(node->GetChild(i));
		}
	}
}
