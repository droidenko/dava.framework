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

#include <QMimeData>

#include "DockSceneTree/SceneTreeModel.h"
#include "Scene/SceneSignals.h"

SceneTreeModel::SceneTreeModel(QObject* parent /*= 0*/ )
	: QStandardItemModel(parent)
	, curScene(NULL)
	, dropAccepted(false)
{
	setColumnCount(1);
	setSupportedDragActions(Qt::MoveAction);

	QStringList headerLabels;
	headerLabels.append("Scene hierarchy");
	setHorizontalHeaderLabels(headerLabels);

	QObject::connect(SceneSignals::Instance(), SIGNAL(Removed(SceneEditor2 *, DAVA::Entity *)), this, SLOT(EntityRemoved(SceneEditor2 *, DAVA::Entity *)));
}

SceneTreeModel::~SceneTreeModel()
{
	if(NULL != curScene)
	{
		curScene->Release();
		curScene = NULL;
	}
}

void SceneTreeModel::SetScene(SceneEditor2 *scene)
{
	// remove add rows
	removeRows(0, rowCount());

	if(NULL != curScene)
	{
		curScene->Release();
	}

	curScene = scene;

	if(NULL != curScene)
	{
		curScene->Retain();

		// add new rows
		for(int i = 0; i < scene->GetChildrenCount(); ++i)
		{
			SceneTreeItem *item = new SceneTreeItem(scene->GetChild(i));

			QList<QStandardItem *> row;
			row.push_back(item);

			appendRow(row);
		}
	}
}

SceneEditor2* SceneTreeModel::GetScene() const
{
	return curScene;
}

QModelIndex SceneTreeModel::GetEntityIndex(DAVA::Entity *entity) const
{
	QModelIndex index;
	SceneTreeItem *foundItem = NULL;

	for(int i = 0; i < rowCount(); ++i)
	{
		foundItem = ((SceneTreeItem *) item(i))->SearchEntity(entity);
		if(NULL != foundItem)
		{
			break;
		}
	}

	if(NULL != foundItem)
	{
		index = indexFromItem(foundItem);
	}

	return index;
}

DAVA::Entity* SceneTreeModel::GetEntity(const QModelIndex &index) const
{
	DAVA::Entity *ret = NULL;

	SceneTreeItem *item = (SceneTreeItem *) itemFromIndex(index);
	if(NULL != item)
	{
		ret = item->GetEntity();
	}

	return ret;
}

bool SceneTreeModel::DropIsAccepted()
{
	return dropAccepted;
}

Qt::DropActions SceneTreeModel::supportedDropActions() const
{
	return Qt::MoveAction;
}

QMimeData * SceneTreeModel::mimeData(const QModelIndexList & indexes) const
{
	QMimeData *mimeData = new QMimeData();
	QByteArray encodeData;

	QDataStream stream(&encodeData, QIODevice::WriteOnly);
	foreach(QModelIndex index, indexes)
	{
		if(index.isValid())
		{
			DAVA::Entity* entity = GetEntity(index);
			stream.writeBytes((char *) &entity, sizeof(DAVA::Entity*));
		}
	}

	mimeData->setData("application/dava.entity", encodeData);
	return mimeData;
}

QStringList SceneTreeModel::mimeTypes() const
{
	QStringList types;

	types << "application/dava.entity";

	return types;
}

bool SceneTreeModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
	bool ret = false;

	DAVA::Entity *parentEntity = GetEntity(parent);
	if(NULL != parentEntity)
	{

	}

	//ret = QStandardItemModel::dropMimeData(data, action, row, column, parent);

	dropAccepted = ret;
	return ret;
}

void SceneTreeModel::EntityRemoved(SceneEditor2 *scene, DAVA::Entity *entity)
{
	if(curScene == scene)
	{
		QModelIndex index = GetEntityIndex(entity);
		if(index.isValid())
		{
			removeRow(index.row(), index.parent());
		}
	}
}