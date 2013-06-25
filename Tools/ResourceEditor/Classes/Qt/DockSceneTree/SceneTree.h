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

#ifndef __QT_SCENE_TREE_H__
#define __QT_SCENE_TREE_H__

#include <QWidget>
#include <QTreeView>
#include <QTableView>

#include "Scene/SceneSignals.h"
#include "DockSceneTree/SceneTreeModel.h"
#include "DockSceneTree/SceneTreeDelegate.h"

class SceneTree : public QTreeView
{
	Q_OBJECT

public:
	SceneTree(QWidget *parent = 0);
	~SceneTree();

public slots:
	void ShowContextMenu(const QPoint &pos);
	void LookAtSelection();
	void RemoveSelection();
	void LockEntities();
	void UnlockEntities();

protected:
	SceneTreeModel * treeModel;
	SceneTreeDelegate *treeDelegate;

	bool skipTreeSelectionProcessing;

	void dropEvent(QDropEvent * event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);

protected slots:
	void SceneActivated(SceneEditor2 *scene);
	void SceneDeactivated(SceneEditor2 *scene);
	void EntitySelected(SceneEditor2 *scene, DAVA::Entity *entity);
	void EntityDeselected(SceneEditor2 *scene, DAVA::Entity *entity);

	void TreeSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
	void TreeItemClicked(const QModelIndex & index);
	void TreeItemDoubleClicked(const QModelIndex & index);
	void TreeItemCollapsed(const QModelIndex &index);
	void TreeItemExpanded(const QModelIndex &index);

	void SyncSelectionToTree();
	void SyncSelectionFromTree();
};

#endif // __QT_SCENE_TREE_H__
