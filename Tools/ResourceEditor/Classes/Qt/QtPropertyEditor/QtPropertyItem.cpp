#include "Qt/QtPropertyEditor/QtPropertyItem.h"
#include "Qt/QtPropertyEditor/QtPropertyData.h"

QtPropertyItem::QtPropertyItem()
	: QStandardItem()
	, itemData(NULL)
	, parentName(NULL)
	, itemDataDeleteByParent(false)
{
}

QtPropertyItem::QtPropertyItem(QtPropertyData* data, QtPropertyItem *name)
	: QStandardItem()
	, itemData(data)
	, parentName(name)
	, itemDataDeleteByParent(false)
{
	if(NULL != data && NULL != parentName)
	{
		for (int i = 0; i < data->ChildCount(); ++i)
		{
			QPair<QString, QtPropertyData*> childData = data->ChildGet(i);
			ChildAdd(childData.first, childData.second);
		}

		ApplyDataFlags();
		ApplyNameStyle();

		QObject::connect(data, SIGNAL(ChildRemoving(const QString &, QtPropertyData *)), this, SLOT(DataChildRemoving(const QString &, QtPropertyData *)));
		QObject::connect(data, SIGNAL(ChildAdded(const QString &, QtPropertyData *)), this, SLOT(DataChildAdded(const QString &, QtPropertyData *)));
	}
}

QtPropertyItem::QtPropertyItem(const QVariant &value)
	: QStandardItem()
	, itemData(NULL)
	, parentName(NULL)
	, itemDataDeleteByParent(false)
{
	itemData = new QtPropertyData(value);
}

QtPropertyItem::~QtPropertyItem()
{
	if(NULL != itemData && !itemDataDeleteByParent)
	{
		delete itemData;
		itemData = NULL;
	}
}

QtPropertyData* QtPropertyItem::GetPropertyData() const
{
	return itemData;
}

int QtPropertyItem::type() const
{
	return QStandardItem::UserType + 1;
}

QVariant QtPropertyItem::data(int role) const
{
	QVariant v;

	if(NULL != itemData)
	{
		switch(role)
		{
		case Qt::DecorationRole:
			v = itemData->GetIcon();
			break;
		case Qt::DisplayRole:
		case Qt::EditRole:
			v = itemData->GetValue();
			break;
		case PropertyDataRole:
			v = qVariantFromValue(GetPropertyData());
			break;
		default:
			break;
		}
	}
	
	if(v.isNull())
	{
		v = QStandardItem::data(role);
	}

	return v;
}

void QtPropertyItem::setData(const QVariant & value, int role)
{
	switch(role)
	{
	case Qt::EditRole:
		if(NULL != itemData)
		{
			itemData->SetValue(value);
		}
		break;
	default:
		QStandardItem::setData(value, role);
		break;
	}
}

void QtPropertyItem::ChildAdd(const QString &key, QtPropertyData* data)
{
	if(NULL != parentName)
	{
		QList<QStandardItem *> subItems;
		QtPropertyItem *subName = new QtPropertyItem(key);
		QtPropertyItem *subValue = new QtPropertyItem(data, subName);

		subValue->itemDataDeleteByParent = true;
		// subValue->setFont(QFont("Courier"));
		subName->setEditable(false);

		subItems.append(subName);
		subItems.append(subValue);

		parentName->appendRow(subItems);
	}
}

void QtPropertyItem::ChildRemove(QtPropertyData* data)
{
	if(NULL != parentName)
	{
		for(int i = 0; i < parentName->rowCount(); ++i)
		{
			QtPropertyItem* childItem = (QtPropertyItem*) parentName->child(i, 1);
			if(NULL != childItem && childItem->itemData == data)
			{
				childItem->itemData = NULL;
				parentName->removeRow(i);
			}
		}
	}
}

void QtPropertyItem::ApplyNameStyle()
{
	QFont curFont = parentName->font();

	// if there are childs, set bold font
	if(NULL != parentName && parentName->rowCount() > 0)
	{
		curFont.setBold(true);
	}
	else
	{
		curFont.setBold(false);
	}

	parentName->setFont(curFont);
}

void QtPropertyItem::ApplyDataFlags()
{
	if(NULL != itemData)
	{
		int dataFlags = itemData->GetFlags();

		if(dataFlags & QtPropertyData::FLAG_IS_CHECKABLE)
		{
			setCheckable(true);
			if(itemData->GetValue().toBool())
			{
				setCheckState(Qt::Checked);
			}
		}

		if(dataFlags & QtPropertyData::FLAG_IS_DISABLED)
		{
			setEnabled(false);
		}

		if(dataFlags & QtPropertyData::FLAG_IS_NOT_EDITABLE)
		{
			setEditable(false);
		}
	}
}

void QtPropertyItem::DataChildAdded(const QString &key, QtPropertyData *data)
{
	ChildAdd(key, data);
	ApplyNameStyle();
}

void QtPropertyItem::DataChildRemoving(const QString &key, QtPropertyData *data)
{
	ChildRemove(data);
	ApplyNameStyle();
}
