#ifndef __MATERIAL_LIST_H__
#define __MATERIAL_LIST_H__

#include <QListView>
#include "Qt/MaterialBrowser/MaterialTreeItem.h"
#include "Qt/MaterialBrowser/MaterialViewModel.h"

class MaterialView : public QListView
{
	Q_OBJECT

public:
	MaterialView(QWidget *parent = 0);

signals:
	void selected(const QModelIndex &index);

protected:
	virtual void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
	virtual void resizeEvent(QResizeEvent * event);

};

#endif // __TEXTURE_LIST_H__
