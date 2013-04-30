#include "CommandCreateNode.h"

#include "DAVAEngine.h"
#include "SceneEditor/SceneEditorScreenMain.h"

using namespace DAVA;


CommandCreateNode::CommandCreateNode(ResourceEditor::eNodeType type)
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
    ,   nodeType(type)
{
}


void CommandCreateNode::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->CreateNode(nodeType);
    }
}
