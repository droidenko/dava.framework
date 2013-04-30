#include "FileCommands.h"

#include "DAVAEngine.h"
#include "SceneEditor/SceneEditorScreenMain.h"
#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/EditorConfig.h"
#include "SceneEditor/SceneValidator.h"

#include "Qt/Main/QtUtils.h"
#include "Qt/Main/QtMainWindowHandler.h"
#include "Qt/Scene/SceneData.h"
#include "Qt/Scene/SceneDataManager.h"

#include <QFileDialog>
#include <QString>

using namespace DAVA;

#if 0
//Open Project
CommandOpenProject::CommandOpenProject()
    :   Command(Command::COMMAND_CLEAR_UNDO_QUEUE)
{
}


void CommandOpenProject::Execute()
{
    QString path = QFileDialog::getExistingDirectory(NULL, QString("Open Project Folder"), QString("/"));
    
    if(0 < path.size())
    {
		String projectPath = PathnameToDAVAStyle(path);
		if('/' != projectPath[projectPath.length() - 1])
        {
            projectPath += '/';
        }
        
        EditorSettings::Instance()->SetProjectPath(projectPath);
        String dataSource3Dpathname = projectPath + String("DataSource/3d/");
        EditorSettings::Instance()->SetDataSourcePath(dataSource3Dpathname);
		EditorSettings::Instance()->Save();

        SceneValidator::Instance()->CreateDefaultDescriptors(dataSource3Dpathname);
		SceneValidator::Instance()->SetPathForChecking(projectPath);

		EditorConfig::Instance()->ParseConfig(projectPath + "EditorConfig.yaml");
		
		SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
        if(screen)
        {
            screen->UpdateModificationPanel();
		}
		
		/* #### dock -->
		SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
        if(activeScene)
        {
            activeScene->ReloadLibrary();
        }
		<-- */
	}

	QtMainWindowHandler::Instance()->RestoreDefaultFocus();
}
#endif


//Open scene
CommandOpenScene::CommandOpenScene(const DAVA::FilePath &scenePathname/* = DAVA::FilePath() */)
    :   Command(Command::COMMAND_CLEAR_UNDO_QUEUE)
    ,   selectedScenePathname(scenePathname)
{
}


void CommandOpenScene::Execute()
{
    if(selectedScenePathname.IsEmpty())
    {
        FilePath dataSourcePath = EditorSettings::Instance()->GetDataSourcePath();
        selectedScenePathname = GetOpenFileName(String("Open Scene File"), dataSourcePath, String("Scene File (*.sc2)"));
    }
    
    if(!selectedScenePathname.IsEmpty())
    {
        SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
        if(screen)
        {
            screen->NewScene();
            
            EditorSettings::Instance()->AddLastOpenedFile(selectedScenePathname);
            screen->OpenFileAtScene(selectedScenePathname);
            
			QtMainWindowHandler::Instance()->SelectMaterialViewOption(Material::MATERIAL_VIEW_TEXTURE_LIGHTMAP);

            //GUIState::Instance()->SetNeedUpdatedFileMenu(true);
        }
        
        QtMainWindowHandler::Instance()->ShowStatusBarMessage(selectedScenePathname.GetAbsolutePathname());
    }
}

//New
CommandNewScene::CommandNewScene()
:   Command(Command::COMMAND_CLEAR_UNDO_QUEUE)
{
}


void CommandNewScene::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->NewScene();
        SceneValidator::Instance()->EnumerateSceneTextures();
    }
}


//Save
CommandSaveScene::CommandSaveScene()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandSaveScene::Execute()
{
    SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
    if(activeScene->CanSaveScene())
    {
        SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
        
		FilePath currentPath;
		if(!screen->CurrentScenePathname().IsEmpty())
		{
			currentPath = screen->CurrentScenePathname();    
		}
		else
		{
			currentPath = EditorSettings::Instance()->GetDataSourcePath();
		}

        QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save Scene File"), QString(currentPath.GetAbsolutePathname().c_str()),
                                                        QString("Scene File (*.sc2)")
                                                        );
        if(0 < filePath.size())
        {
			FilePath normalizedPathname = PathnameToDAVAStyle(filePath);

            EditorSettings::Instance()->AddLastOpenedFile(normalizedPathname);

			SaveParticleEmitterNodes(activeScene->GetScene());
            screen->SaveSceneToFile(normalizedPathname);
        }
    }

	QtMainWindowHandler::Instance()->RestoreDefaultFocus();
}

void CommandSaveScene::SaveParticleEmitterNodes(EditorScene* scene)
{
	if (!scene)
	{
		return;
	}

	int32 childrenCount = scene->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		SaveParticleEmitterNodeRecursive(scene->GetChild(i));
	}
}

void CommandSaveScene::SaveParticleEmitterNodeRecursive(Entity* parentNode)
{
	bool needSaveThisLevelNode = true;
	ParticleEmitter * emitter = GetEmitter(parentNode);
	if (!emitter)
	{
		needSaveThisLevelNode = false;
	}

	if (needSaveThisLevelNode)
	{
		// Do we have file name? Ask for it, if not.
		FilePath yamlPath = emitter->GetConfigPath();
		if (yamlPath.IsEmpty())
		{
			QString saveDialogCaption = QString("Save Particle Emitter \"%1\"").arg(QString::fromStdString(parentNode->GetName()));
			QString saveDialogYamlPath = QFileDialog::getSaveFileName(NULL, saveDialogCaption, "", QString("Yaml File (*.yaml)"));

			if (!saveDialogYamlPath.isEmpty())
			{
				yamlPath = PathnameToDAVAStyle(saveDialogYamlPath);
			}
		}

		if (!yamlPath.IsEmpty())
		{
			emitter->SaveToYaml(yamlPath);
		}
	}

	// Repeat for all children.
	int32 childrenCount = parentNode->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		SaveParticleEmitterNodeRecursive(parentNode->GetChild(i));
	}
}

//Export
CommandExport::CommandExport(ImageFileFormat fmt)
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
    ,   format(fmt)
{
}


void CommandExport::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->ExportAs(format);
    }
}


//Save to folder with childs
CommandSaveToFolderWithChilds::CommandSaveToFolderWithChilds()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandSaveToFolderWithChilds::Execute()
{
	QString path = QFileDialog::getExistingDirectory(NULL, QString("Open Folder"), QString("/"));
	
    if(0 < path.size())
    {
		FilePath folderPath = PathnameToDAVAStyle(path);
        folderPath.MakeDirectoryPathname();

		SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
		if(screen)
		{
			screen->SaveToFolder(folderPath);
		}
	}
}
