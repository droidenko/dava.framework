#include <QMouseEvent>
#include "Qt/QtPropertyEditor/QtPropertyEditor.h"
#include "Qt/QtPropertyEditor/QtPropertyModel.h"
#include "Qt/QtPropertyEditor/QtPropertyItemDelegate.h"

QtPropertyEditor::QtPropertyEditor(QWidget *parent /* = 0 */)
	: QTreeView(parent)
{
	curModel = new QtPropertyModel();
	setModel(curModel);

	curItemDelegate = new QtPropertyItemDelegate();
	setItemDelegate(curItemDelegate);

	QObject::connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(ItemClicked(const QModelIndex &)));
	QObject::connect(this, SIGNAL(expanded(const QModelIndex &)), curItemDelegate, SLOT(expand(const QModelIndex &)));
	QObject::connect(this, SIGNAL(collapsed(const QModelIndex &)), curItemDelegate, SLOT(collapse(const QModelIndex &)));
}

QtPropertyEditor::~QtPropertyEditor()
{ }

QPair<QtPropertyItem*, QtPropertyItem*> QtPropertyEditor::AppendProperty(const QString &name, QtPropertyData* data, QtPropertyItem* parent /*= NULL*/)
{
	if(NULL != data)
	{
		data->SetOWViewport(viewport());
	}

	return curModel->AppendProperty(name, data, parent);
}

void QtPropertyEditor::RemoveProperty(QtPropertyItem* item)
{
	curModel->RemoveProperty(item);
}

void QtPropertyEditor::RemovePropertyAll()
{
	curModel->RemovePropertyAll();
}

void QtPropertyEditor::Expand(QtPropertyItem *item)
{
	expand(curModel->indexFromItem(item));
}

void QtPropertyEditor::ItemClicked(const QModelIndex &index)
{
	QStandardItem *item = curModel->itemFromIndex(index);
	if(NULL != item && item->isEditable() && item->isEnabled())
	{
		edit(index);
	}
}
