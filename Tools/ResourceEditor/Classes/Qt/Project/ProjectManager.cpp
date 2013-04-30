#include <QFileDialog>
#include "Qt/Project/ProjectManager.h"
#include "Qt/Main/QtUtils.h"
#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/SceneValidator.h"
#include "SceneEditor/EditorConfig.h"
#include "SceneEditor/SceneEditorScreenMain.h"

ProjectManager::ProjectManager()
	: curProjectPath("")
{

}

ProjectManager::~ProjectManager()
{

}

bool ProjectManager::IsOpened()
{
	return (curProjectPath != "");
}

QString ProjectManager::CurProjectPath()
{
	return curProjectPath;
}

QString ProjectManager::CurProjectDataSourcePath()
{
	return QString(EditorSettings::Instance()->GetDataSourcePath().GetAbsolutePathname().c_str());
}

QString ProjectManager::ProjectOpenDialog()
{
	return QFileDialog::getExistingDirectory(NULL, QString("Open Project Folder"), QString("/"));
}

void ProjectManager::ProjectOpen(const QString &path)
{
	if(path != curProjectPath)
	{
		ProjectClose();

		curProjectPath = path;

		if(!curProjectPath.isEmpty())
		{
			DAVA::FilePath projectPath = PathnameToDAVAStyle(path);
            projectPath.MakeDirectoryPathname();

			DAVA::FilePath dataSource3Dpathname = projectPath + "DataSource/3d/";

			EditorSettings::Instance()->SetProjectPath(projectPath);
			EditorSettings::Instance()->SetDataSourcePath(dataSource3Dpathname);
			EditorSettings::Instance()->Save();

			SceneValidator::Instance()->CreateDefaultDescriptors(dataSource3Dpathname);
			SceneValidator::Instance()->SetPathForChecking(projectPath);

			EditorConfig::Instance()->ParseConfig(projectPath + "EditorConfig.yaml");
		}

		SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
		if(screen)
		{
			screen->UpdateModificationPanel();
		}

		emit ProjectOpened(curProjectPath);

		// TODO: 
		// DAVA::FilePath::SetProjectPathname(curProjectPath.toStdString());
	}
}

void ProjectManager::ProjectClose()
{
	if("" != curProjectPath)
	{
		emit ProjectClosed(curProjectPath);
		curProjectPath = "";
		
		// TODO:
		// DAVA::FilePath::SetProjectPathname(curProjectPath.toStdString());
	}
}
