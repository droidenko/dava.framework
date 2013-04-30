#include "Qt/QtPropertyEditor/QtPropertyModel.h"
#include "Qt/QtPropertyEditor/QtPropertyItem.h"

QtPropertyModel::QtPropertyModel(QObject* parent /* = 0 */)
	: QStandardItemModel(parent)
{
	QStringList headerLabels;

	headerLabels.append("Name");
	headerLabels.append("Value");

	setColumnCount(2);
	setHorizontalHeaderLabels(headerLabels);

	QObject::connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(ItemChanged(QStandardItem *)));
}

QtPropertyModel::~QtPropertyModel()
{ }

QPair<QtPropertyItem*, QtPropertyItem*> QtPropertyModel::AppendProperty(const QString &name, QtPropertyData* data, QtPropertyItem* parent /*= NULL*/)
{
	QList<QStandardItem *> items;
	QStandardItem* root = (QStandardItem *) parent;

	QtPropertyItem *newPropertyName = new QtPropertyItem(name);
	QtPropertyItem *newPropertyValue = new QtPropertyItem(data, newPropertyName);

	newPropertyName->setEditable(false);

	items.append(newPropertyName);
	items.append(newPropertyValue);

	if(NULL == root)
	{
		root = invisibleRootItem();
	}

	root->appendRow(items);

	return QPair<QtPropertyItem*, QtPropertyItem*>(newPropertyName, newPropertyValue);
}

QtPropertyData * QtPropertyModel::GetProperty(const QString &name, QtPropertyItem* parent/* = NULL*/)
{
    QStandardItem* root = (QStandardItem *) parent;
	if(NULL == root)
	{
		root = invisibleRootItem();
	}

    for(DAVA::int32 r = 0; r < root->rowCount(); ++r)
    {
        QtPropertyItem *keyItem = static_cast<QtPropertyItem *>(root->child(r, 0));
        if(keyItem->GetPropertyData()->GetValue().toString() == name)
        {
            QtPropertyItem *dataItem = static_cast<QtPropertyItem *>(root->child(r, 1));
            return dataItem->GetPropertyData();
        }
    }
    
    return NULL;
}


void QtPropertyModel::RemoveProperty(QtPropertyItem* item)
{
	removeRow(indexFromItem(item).row());
}

void QtPropertyModel::RemovePropertyAll()
{
	removeRows(0, rowCount());
}

void QtPropertyModel::ItemChanged(QStandardItem* item)
{
	QtPropertyItem *propItem = (QtPropertyItem *) item;

	if(NULL != propItem)
	{
		if(propItem->isCheckable())
		{
			propItem->GetPropertyData()->SetValue(QVariant(propItem->checkState() == Qt::Checked));
		}
	}
}