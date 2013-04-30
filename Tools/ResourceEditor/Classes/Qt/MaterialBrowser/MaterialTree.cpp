#include "Qt/MaterialBrowser/MaterialTree.h"


MaterialTree::MaterialTree(QWidget *parent /*= 0*/)
{

}

void MaterialTree::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
	QTreeView::selectionChanged(selected, deselected);
	emit Selected(selected, deselected);
}
