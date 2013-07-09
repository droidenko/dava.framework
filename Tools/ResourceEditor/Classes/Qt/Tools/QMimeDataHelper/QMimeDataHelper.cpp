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

#include "QMimeDataHelper.h"
#include "../Qt/DockSceneTree/SceneTreeModel.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "Main/mainwindow.h"

#include <QList>
#include <QUrl>
#include <QFileInfo>


#define MIME_HANDLER(data, retList, name, method)\
    if(data->hasFormat(name)){\
        return method(data, retList);}\

const QString QMimeDataHelper::supportedMimeFormats[] =
{
	"application/dava.entity",
	"application/dava.emitter",
	"text/uri-list"
};


bool QMimeDataHelper::IsMimeDataTypeSupported(const QMimeData* mimeData)
{
	for(int32 i = MIME_ENTITY; i < MIME_TYPES_COUNT; ++i)
	{
		if(mimeData->hasFormat(QString(supportedMimeFormats[i])))
		{
			if(i == MIME_FILE_PATH)
			{
				QList<QUrl> droppedUrls = mimeData->urls();
		
				Q_FOREACH(QUrl url, droppedUrls)
				{
					FilePath filePath(url.toLocalFile().toStdString());
					if(!(filePath.Exists() && filePath.GetExtension() == ".sc2"))
					{
						return false;
					}
				}
			}
			
			return true;
		}
	}
	return false;
}

bool QMimeDataHelper::GetItemNamesFromMime(const QMimeData* mimeData, List<String>& nameList)
{
	if(!IsMimeDataTypeSupported(mimeData))
	{
		return false;
	}

	MIME_HANDLER(mimeData, nameList, supportedMimeFormats[MIME_ENTITY],	GetItemNamesFromSceneTreeMime)
	MIME_HANDLER(mimeData, nameList, supportedMimeFormats[MIME_FILE_PATH],	GetItemNamesFromFilePathMime)
	
	return false;
}

bool QMimeDataHelper::ConvertToMime(List<FilePath>& filePathList, QMimeData* mimeData)
{
	if(mimeData == NULL)
	{
		return false;
	}
	
	mimeData->clear();
	QList<QUrl> list;
	for(List<FilePath>::const_iterator it = filePathList.begin(); it != filePathList.end(); ++it)
	{
		list.append(QUrl(QString(it->GetAbsolutePathname().c_str())));
	}
	mimeData->setUrls(list);
	return true;
}

bool QMimeDataHelper::ConvertToMime(List<Entity*>& entityList, QMimeData* mimeData)
{
	if(mimeData == NULL)
	{
		return false;
	}
	
	mimeData->clear();
	QByteArray encodedData;
	
	QDataStream stream(&encodedData, QIODevice::WriteOnly);
	foreach(Entity* entity, entityList)
	{
		stream.writeRawData((char *) &entity, sizeof(DAVA::Entity*));
	}
	
	mimeData->setData(SceneTreeModel::mimeFormatEntity, encodedData);
	
	return true;
}

bool QMimeDataHelper::ConvertQMimeDataFromSceneTree(const QMimeData* mimeData, List<Entity*>& retList)
{
	retList.clear();
	QByteArray encodedData = mimeData->data(SceneTreeModel::mimeFormatEntity);
	QDataStream stream(&encodedData, QIODevice::ReadOnly);

	while(!stream.atEnd())
	{
		Entity *entity = NULL;
		stream.readRawData((char *) &entity, sizeof(Entity*));
		if(NULL != entity)
		{
			retList.push_back(entity);
		}
	}
	
	return retList.size()>0;
}

bool QMimeDataHelper::ConvertQMimeDataFromFilePath(const QMimeData* mimeData,List<Entity*>& retList)
{
	retList.clear();
	if(!mimeData->hasUrls())
	{
		return false;
	}

	SceneEditor2 *curSceneEditor = QtMainWindow::Instance()->GetUI()->sceneTabWidget->GetCurrentScene();
	
	if(curSceneEditor == NULL)
	{
		return false;
	}
	
	QList<QUrl> droppedUrls = mimeData->urls();

	Q_FOREACH(QUrl url, droppedUrls)
	{
		FilePath filePath(url.toLocalFile().toStdString());
		if(!(filePath.Exists() && filePath.GetExtension() == ".sc2"))
		{
			continue;
		}
		
		Entity * entity = curSceneEditor->GetRootNode(filePath);//!!!
		
		retList.push_back(entity);
	}
	
	return retList.size()>0;
}

bool QMimeDataHelper::GetItemNamesFromSceneTreeMime(const QMimeData* mimeData, List<String> & nameList)
{
	List<Entity*> entityList;
	if(!ConvertQMimeDataFromSceneTree(mimeData, entityList))
	{
		return false;
	}
	
	nameList.clear();
	Q_FOREACH(Entity* entity, entityList)
	{
		nameList.push_back(entity->GetName());
	}
	return true;
}

bool QMimeDataHelper::GetItemNamesFromFilePathMime(const QMimeData* mimeData, List<String> & nameList)
{
	nameList.clear();
	
	QList<QUrl> droppedUrls = mimeData->urls();
	Q_FOREACH(QUrl url, droppedUrls)
	{
		nameList.push_back(url.toLocalFile().toStdString());
	}
	
	return nameList.size()>0;
}

bool QMimeDataHelper::ConvertQMimeData(const QMimeData* mimeData, List<Entity*>& retList)
{
	MIME_HANDLER(mimeData, retList, supportedMimeFormats[MIME_ENTITY],	ConvertQMimeDataFromSceneTree)
	MIME_HANDLER(mimeData, retList, supportedMimeFormats[MIME_FILE_PATH],ConvertQMimeDataFromFilePath)
	
	return false;
}
