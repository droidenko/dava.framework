#include "TextureOptionsCommands.h"
#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/SceneValidator.h"
#include "Qt/Scene/SceneDataManager.h"

using namespace DAVA;

ReloadTexturesAsCommand::ReloadTexturesAsCommand(ImageFileFormat format)
    :   Command(COMMAND_CLEAR_UNDO_QUEUE)
    ,   fileFormat(format)
{
}

void ReloadTexturesAsCommand::Execute()
{
    Texture::SetDefaultFileFormat(fileFormat);
    
    EditorSettings::Instance()->SetTextureViewFileFormat(fileFormat);
    EditorSettings::Instance()->Save();
    
    SceneDataManager::Instance()->TextureReloadAll(fileFormat);
    SceneValidator::Instance()->EnumerateSceneTextures();
}

