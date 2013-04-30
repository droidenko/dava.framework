#include "Qt/MaterialBrowser/MaterialTreeItem.h"

MaterialTreeItem::MaterialTreeItem(DAVA::Material *material, MaterialTreeItem *parent /* = NULL */)
	: parentItem(parent)
	, material(material)
{
	type = TYPE_MATERIAL;
	if(NULL != material)
	{
		name = material->GetName().c_str();
	}

	if(NULL != parent)
	{
		parent->AddChild(this);
	}
}

MaterialTreeItem::MaterialTreeItem(QString folderName, MaterialTreeItem *parent /* = NULL */)
	: parentItem(parent)
	, material(NULL)
{
	type = TYPE_FOLDER;
	name = folderName;

	if(NULL != parent)
	{
		parent->AddChild(this);
	}
}

MaterialTreeItem::~MaterialTreeItem()
{
	qDeleteAll(childItems);
}

void MaterialTreeItem::AddChild(MaterialTreeItem *child)
{
	childItems.append(child);

	if(NULL != child)
	{
		child->parentItem = this;
	}
}

MaterialTreeItem* MaterialTreeItem::Child(int row) const
{
	MaterialTreeItem* item = NULL;

	if(row < childItems.count())
	{
		item = childItems.value(row);
	}

	return item;
}

MaterialTreeItem* MaterialTreeItem::Parent() const
{
	return parentItem;
}

int MaterialTreeItem::Row() const
{
	int row = 0;

	if (parentItem)
	{
		row = parentItem->childItems.indexOf(const_cast<MaterialTreeItem*>(this));
	}

	return 0;
}

int MaterialTreeItem::ChildCount() const
{
	return childItems.count();
}

int MaterialTreeItem::ColumnCount() const
{
	// TODO: 
	return 1;
}

QVariant MaterialTreeItem::Data(int column) const
{
	return QVariant(name);
}

const DAVA::Material* MaterialTreeItem::Material() const
{
	return material;
}
