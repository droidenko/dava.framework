#include "CommandsManager.h"
#include "Command.h"

#include "Qt/Main/QtMainWindowHandler.h"

using namespace DAVA;

CommandsManager::CommandsManager()
{
}

CommandsManager::~CommandsManager()
{
	ClearAllQueues();
}

void CommandsManager::ClearAllQueues()
{
	for (QUEUE_MAP::iterator it = queueMap.begin(); it != queueMap.end(); ++it)
	{
		ClearQueue((*it).second);
		SafeDelete((*it).second);
	}
	queueMap.clear();
}

void CommandsManager::ClearQueue(UndoQueue* queue)
{
	DVASSERT(queue);

	queue->commandIndex = -1;
	for_each(queue->commands.begin(),
			 queue->commands.end(),
			 SafeRelease<Command>);
	queue->commands.clear();
}

void CommandsManager::SceneReleased(SceneData* scene)
{
	QUEUE_MAP::iterator it = queueMap.find(scene);

	if (it != queueMap.end())
	{
		ClearQueue((*it).second);
		SafeDelete((*it).second);
		queueMap.erase(it);
	}
}

void CommandsManager::ClearQueueTail(UndoQueue* queue)
{
	DVASSERT(queue);

	if ((queue->commandIndex >= -1) &&
		(queue->commandIndex < (int32)queue->commands.size()))
	{
        DVASSERT(queue->commandIndex <= (UNDO_QUEUE_SIZE - 1)); //Queue cannot be more than UNDO_QUEUE_SIZE
        if(queue->commandIndex == UNDO_QUEUE_SIZE - 1)
        {
            SafeRelease(queue->commands[0]);
            queue->commands.erase(queue->commands.begin());

            --queue->commandIndex;
        }

		int32 newCount = queue->commandIndex + 1;
		for_each(queue->commands.begin() + newCount,
				 queue->commands.end(),
				 SafeRelease<Command>);
		queue->commands.resize(newCount);
	}
}


void CommandsManager::Execute(Command *command, SceneData* forScene)
{
	UndoQueue* queue = GetQueueForScene(forScene);
	DVASSERT(queue);

    command->Execute();
    
    switch (command->Type()) 
    {
        case Command::COMMAND_WITHOUT_UNDO_EFFECT:
            break;
            
        case Command::COMMAND_UNDO_REDO:
            
            if(Command::STATE_VALID == command->State())
            {
                //TODO: VK: if need only UNDO_QUEUE_SIZE commands at queue you may add code here
				ClearQueueTail(queue);
				queue->commands.push_back(SafeRetain(command));
				++queue->commandIndex;
            }
            break;

        case Command::COMMAND_CLEAR_UNDO_QUEUE:
            if(Command::STATE_VALID == command->State())
            {
                ClearQueue(queue);
            }
            break;

        default:
            Logger::Warning("[CommandsManager::Execute] command type (%d) not processed", command->Type());
            break;
    }

	QtMainWindowHandler::Instance()->UpdateUndoActionsState();
}

void CommandsManager::ExecuteAndRelease(Command* command, SceneData* forScene)
{
	Execute(command, forScene);
	SafeRelease(command);
}

void CommandsManager::Undo(SceneData* forScene)
{
	UndoQueue* queue = GetQueueForScene(forScene);
	DVASSERT(queue);

	if ((queue->commandIndex >= 0) &&
		(queue->commandIndex < (int32)queue->commands.size()))
	{
		//TODO: need check state?
		queue->commands[queue->commandIndex]->Cancel();
		--queue->commandIndex;

		QtMainWindowHandler::Instance()->UpdateUndoActionsState();
	}
}

void CommandsManager::Redo(SceneData* forScene)
{
	UndoQueue* queue = GetQueueForScene(forScene);
	DVASSERT(queue);

	if ((queue->commandIndex >= -1) &&
		(queue->commandIndex < (int32)queue->commands.size() - 1))
	{
		//TODO: need check state?
		++queue->commandIndex;
		queue->commands[queue->commandIndex]->Execute();
	}
}

int32 CommandsManager::GetUndoQueueLength(SceneData* forScene)
{
	UndoQueue* queue = GetQueueForScene(forScene);
	DVASSERT(queue);

	return queue->commandIndex + 1;
}

int32 CommandsManager::GetRedoQueueLength(SceneData* forScene)
{
	UndoQueue* queue = GetQueueForScene(forScene);
	DVASSERT(queue);

	return queue->commands.size() - queue->commandIndex - 1;
}

String CommandsManager::GetUndoCommandName(SceneData* forScene)
{
	UndoQueue* queue = GetQueueForScene(forScene);
	DVASSERT(queue);

	if ((queue->commandIndex >= 0) &&
		(queue->commandIndex < (int32)queue->commands.size()))
	{
		return queue->commands[queue->commandIndex]->commandName;
	}

	return "";
}

String CommandsManager::GetRedoCommandName(SceneData* forScene)
{
	UndoQueue* queue = GetQueueForScene(forScene);
	DVASSERT(queue);

	if ((queue->commandIndex >= -1) &&
		(queue->commandIndex < (int32)queue->commands.size() - 1))
	{
		return queue->commands[queue->commandIndex + 1]->commandName;
	}

	return "";
}

CommandsManager::UndoQueue* CommandsManager::GetQueueForScene(SceneData *scene)
{
	if (!scene)
	{
		scene = SceneDataManager::Instance()->SceneGetActive();
	}
	DVASSERT(scene);

	UndoQueue* res;

	QUEUE_MAP::iterator it = queueMap.find(scene);
	if (it != queueMap.end())
	{
		res = it->second;
	}
	else
	{
		res = new UndoQueue(scene);
		queueMap[scene] = res;
	}

	return res;
}