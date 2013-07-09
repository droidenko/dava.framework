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

#include <QFileDialog>
#include "Project/ProjectManager.h"
#include "Main/QtUtils.h"
#include "Classes/SceneEditor/EditorSettings.h"
#include "Classes/SceneEditor/SceneValidator.h"
#include "Classes/SceneEditor/EditorConfig.h"
#include "Classes/SceneEditor/SceneEditorScreenMain.h"

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
