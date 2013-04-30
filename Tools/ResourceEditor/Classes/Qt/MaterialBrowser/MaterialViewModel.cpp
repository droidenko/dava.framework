#include <QImage>
#include <QPainter>
#include "Qt/MaterialBrowser/MaterialViewModel.h"

MaterialViewModel::MaterialViewModel(QObject *parent /* = 0 */)
	: treeItem(NULL)
{

}

int MaterialViewModel::rowCount(const QModelIndex &parent /* = QModelIndex */) const
{
	if(NULL != treeItem)
	{
		return treeItem->ChildCount();
	}

	return 0;
}

QVariant MaterialViewModel::data(const QModelIndex &index, int role /* = Qt::DisplayRole */) const
{
	QVariant v;

	if(index.isValid())
	{
		MaterialTreeItem *item = treeItem->Child(index.row());
		if(NULL != item)
		{
			switch(role)
			{
			case Qt::DisplayRole:
				v = item->Data(0);
				break;
			case Qt::DecorationRole:
				{
					QImage img = QImage(80, 80, QImage::Format_ARGB32);
					QPainter p(&img);
					p.setBrush(QColor(0,255,0));
					p.drawEllipse(QPoint(40, 40), 25, 25);
					v = img;
				}
				break;
			}
		}
	}

	return v;
}

void MaterialViewModel::SetTreeItem(MaterialTreeItem* item)
{
	beginResetModel();

	DAVA::SafeRelease(treeItem);
	treeItem = item;
	DAVA::SafeRetain(treeItem);

	endResetModel();

	//emit dataChanged();
}