#ifndef __MATERIAL_TREE_MODEL_H__
#define __MATERIAL_TREE_MODEL_H__

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "DAVAEngine.h"
#include "Qt/MaterialBrowser/MaterialTreeItem.h"


class MaterialTreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	MaterialTreeModel(QObject *parent = 0);
	~MaterialTreeModel();

	void SetScene(DAVA::Scene *scene);
	MaterialTreeItem* Item(const QModelIndex &index) const;

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	QVariant data(const QModelIndex &index, int role) const;

private:
	DAVA::Scene *scene;
	MaterialTreeItem *rootFiltredMaterial;

	DAVA::Vector<DAVA::Material *> allMaterials;

	void SearchMaterialsInScene();
	void SearchMaterialsInFolder();
	void ApplyFilterAndSort();
};

#endif // __MATERIAL_TREE_MODEL_H__
