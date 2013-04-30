//
//  PropertyEditorStateHelper.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 2/7/13.
//
//

#ifndef __PROPERTY_EDITOR_STATE_HELPER__H__
#define __PROPERTY_EDITOR_STATE_HELPER__H__

#include "Qt/Main/QTreeViewStateHelper.h"
#include "Qt/QtPropertyEditor/QtPropertyModel.h"

class PropertyEditorStateHelper : public DAVA::QTreeViewStateHelper<QString>
{
public:
	virtual void SaveTreeViewState(bool needCleanupStorage);
	PropertyEditorStateHelper(QTreeView* treeView, QtPropertyModel* model);
	
protected:
	virtual QString GetPersistentDataForModelIndex(const QModelIndex &modelIndex);
	
private:
	QtPropertyModel* model;

	// Auxiliary map to store Full Paths to the items were already calculated.
	DAVA::Map<QStandardItem*, QString> fullPathsCache;
};

#endif /* defined(__PROPERTY_EDITOR_STATE_HELPER__H__) */
