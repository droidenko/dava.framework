//
//  SceneGraphModelStateHelper.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 2/5/13.
//
//

#ifndef __SCENE_GRAPH_MODEL_STATE_HELPER__H__
#define __SCENE_GRAPH_MODEL_STATE_HELPER__H__

#include <QTreeView>

#include "SceneGraphModel.h"
#include "Qt/Main/QTreeViewStateHelper.h"

using namespace DAVA;

// Save/Restore helper for QSceneGraphTreeView.
class SceneGraphModelStateHelper : public DAVA::QTreeViewStateHelper<void*>
{
public:
	SceneGraphModelStateHelper(QTreeView* treeView, SceneGraphModel* model);
	
protected:
	virtual void* GetPersistentDataForModelIndex(const QModelIndex &modelIndex);
	
private:
	SceneGraphModel* model;
};

#endif /* defined(__SCENE_GRAPH_MODEL_STATE_HELPER__H__) */
