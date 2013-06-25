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

#include "DockSceneTree/SceneTree.h"
#include <QBoxLayout>
#include <QDropEvent>
#include <QMenu>

SceneTree::SceneTree(QWidget *parent /*= 0*/)
	: QTreeView(parent)
	, skipTreeSelectionProcessing(false)
{
	treeModel = new SceneTreeModel();
	setModel(treeModel);

	setDragDropMode(QAbstractItemView::InternalMove);
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);
	setContextMenuPolicy(Qt::CustomContextMenu);

	// scene signals
	QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(SceneActivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(SceneDeactivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Selected(SceneEditor2 *, DAVA::Entity *)), this, SLOT(EntitySelected(SceneEditor2 *, DAVA::Entity *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Deselected(SceneEditor2 *, DAVA::Entity *)), this, SLOT(EntityDeselected(SceneEditor2 *, DAVA::Entity *)));

	// this widget signals
	QObject::connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(TreeSelectionChanged(const QItemSelection &, const QItemSelection &)));
	QObject::connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(TreeItemClicked(const QModelIndex &)));
	QObject::connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(TreeItemDoubleClicked(const QModelIndex &)));
	QObject::connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));
}

SceneTree::~SceneTree()
{

}

void SceneTree::dropEvent(QDropEvent * event)
{
	QTreeView::dropEvent(event);

	// this is a workaround for Qt bug
	// see https://bugreports.qt-project.org/browse/QTBUG-26229 
	// for more information
	if(!treeModel->DropIsAccepted())
	{
		event->setDropAction(Qt::IgnoreAction);
	}
}

void SceneTree::SceneActivated(SceneEditor2 *scene)
{
	treeModel->SetScene(scene);
}

void SceneTree::SceneDeactivated(SceneEditor2 *scene)
{
	treeModel->SetScene(NULL);
}

void SceneTree::EntitySelected(SceneEditor2 *scene, DAVA::Entity *entity)
{
	if(!skipTreeSelectionProcessing)
	{
		skipTreeSelectionProcessing = true;

		if(scene == treeModel->GetScene())
		{
			QModelIndex index = treeModel->GetEntityIndex(entity);

			if(index.isValid())
			{
				selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
				scrollTo(index);
			}
		}
		else
		{
			selectionModel()->clear();
		}

		skipTreeSelectionProcessing = false;
	}
}

void SceneTree::EntityDeselected(SceneEditor2 *scene, DAVA::Entity *entity)
{
	if(!skipTreeSelectionProcessing)
	{
		skipTreeSelectionProcessing = true;

		if(scene == treeModel->GetScene())
		{
			QModelIndex index = treeModel->GetEntityIndex(entity);

			if(index.isValid())
			{
				selectionModel()->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
			}
		}
		else
		{
			selectionModel()->clear();
		}

		skipTreeSelectionProcessing = false;
	}
}

void SceneTree::TreeSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
	if(!skipTreeSelectionProcessing)
	{
		skipTreeSelectionProcessing = true;

		SceneEditor2* curScene = treeModel->GetScene();
		if(NULL != curScene)
		{
			// deselect items in scene
			QModelIndexList indexList = deselected.indexes();
			for (int i = 0; i < indexList.size(); ++i)
			{
				DAVA::Entity *entity = treeModel->GetEntity(indexList[i]);
				curScene->selectionSystem->RemSelection(entity);
			}

			// select items in scene
			indexList = selected.indexes();
			for (int i = 0; i < indexList.size(); ++i)
			{
				DAVA::Entity *entity = treeModel->GetEntity(indexList[i]);
				curScene->selectionSystem->AddSelection(entity);
			}
		}

		skipTreeSelectionProcessing = false;
	}
}

void SceneTree::TreeItemClicked(const QModelIndex & index)
{
	SceneEditor2* sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		// TODO:
		// ...
	}
}

void SceneTree::TreeItemDoubleClicked(const QModelIndex & index)
{
	SceneEditor2* sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		DAVA::Entity *entity = treeModel->GetEntity(index);
		if(NULL != entity)
		{
			DAVA::AABBox3 worldBox;
			DAVA::AABBox3 box = sceneEditor->selectionSystem->CalcAABox(entity);

			box.GetTransformedBox(entity->GetWorldTransform(), worldBox);
			sceneEditor->cameraSystem->LookAt(worldBox);
		}
	}
}

void SceneTree::ShowContextMenu(const QPoint &pos)
{
	QMenu contextMenu;

	contextMenu.addAction(QIcon(":/QtIcons/zoom.png"), "Look at", this, SLOT(LookAtSelection()));
	contextMenu.addSeparator();
	contextMenu.addAction(QIcon(":/QtIcons/remove.png"), "Remove", this, SLOT(RemoveSelection()));

	contextMenu.exec(mapToGlobal(pos));
}

void SceneTree::LookAtSelection()
{
	SceneEditor2* sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		const EntityGroup* selection = sceneEditor->selectionSystem->GetSelection();
		if(NULL != selection)
		{
			sceneEditor->cameraSystem->LookAt(selection->GetCommonBbox());
		}
	}
}

void SceneTree::RemoveSelection()
{
	SceneEditor2* sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		const EntityGroup* selection = sceneEditor->selectionSystem->GetSelection();
		if(NULL != selection)
		{
			if(selection->Size() > 1)
			{
				sceneEditor->BeginBatch("Remove entities");
			}

			for(int i = 0; i < selection->Size(); ++i)
			{
				sceneEditor->structureSystem->Remove(selection->GetEntity(i));
			}

			if(selection->Size() > 1)
			{
				sceneEditor->EndBatch();
			}
		}
	}
}
