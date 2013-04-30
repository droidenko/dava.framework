#include "Qt/MaterialBrowser/MaterialView.h"

MaterialView::MaterialView(QWidget *parent /* = 0 */)
{
	setViewMode(QListView::IconMode);
	setUniformItemSizes(true);
}

void MaterialView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
	QListView::selectionChanged(selected, deselected);
}

void MaterialView::resizeEvent(QResizeEvent *e)
{
	QListView::resizeEvent(e);
	setGridSize(QSize());
}
