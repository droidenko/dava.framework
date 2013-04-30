#ifndef __TEXTURE_VIEW_MODEL_H__
#define __TEXTURE_VIEW_MODEL_H__

#include <QAbstractListModel>
#include <QVector>

#include "DAVAEngine.h"
#include "Qt/MaterialBrowser/MaterialTreeItem.h"

class MaterialViewModel : public QAbstractListModel
{
	Q_OBJECT

public:
	MaterialViewModel(QObject *parent = 0);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	void SetTreeItem(MaterialTreeItem * item);

private:
	MaterialTreeItem *treeItem;
};

#endif // __TEXTURE_VIEW_MODEL_H__
