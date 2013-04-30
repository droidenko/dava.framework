#include "CommandReloadTextures.h"

#include "DAVAEngine.h"
#include "Qt/Scene/SceneDataManager.h"
#include "SceneEditor/EditorSettings.h"

using namespace DAVA;


CommandReloadTextures::CommandReloadTextures()
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandReloadTextures::Execute()
{
    SceneDataManager::Instance()->TextureReloadAll((ImageFileFormat)EditorSettings::Instance()->GetTextureViewFileFormat());
}

