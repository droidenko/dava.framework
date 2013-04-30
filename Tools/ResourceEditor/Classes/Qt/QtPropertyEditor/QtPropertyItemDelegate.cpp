#include <QtGui>

#include "QtPropertyItemDelegate.h"
#include "QtPropertyModel.h"
#include "QtPropertyItem.h"
#include "Qt/QtPropertyEditor/QtPropertyWidgets/QtColorLineEdit.h"

QtPropertyItemDelegate::QtPropertyItemDelegate(QWidget *parent /* = 0 */)
	: QStyledItemDelegate(parent)
{ }

void QtPropertyItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	recalcOptionalWidgets(index, (QStyleOptionViewItem *) &option);
	QStyledItemDelegate::paint(painter, option, index);
}

QSize QtPropertyItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget* QtPropertyItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QWidget* editWidget = NULL;
	QtPropertyData* data = index.data(QtPropertyItem::PropertyDataRole).value<QtPropertyData*>();
	
	recalcOptionalWidgets(index, (QStyleOptionViewItem *) &option);

	if(NULL != data)
	{
		editWidget = data->CreateEditor(parent, option);
	}

	if(NULL == editWidget)
	{
		editWidget = QStyledItemDelegate::createEditor(parent, option, index);
	}

    return editWidget;
}

void QtPropertyItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    const QtPropertyModel *propertyModel = dynamic_cast<const QtPropertyModel *>(index.model());
	if(NULL != propertyModel)
	{
		QtPropertyItem* item = (QtPropertyItem*) propertyModel->itemFromIndex(index);
		QtPropertyData* data = item->GetPropertyData();
		if(NULL != data)
		{
            data->SetEditorData(editor);
		}
	}

    QStyledItemDelegate::setEditorData(editor, index);
}

void QtPropertyItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    const QtPropertyModel *propertyModel = dynamic_cast<const QtPropertyModel *>(index.model());
	if(NULL != propertyModel)
	{
		QtPropertyItem* item = (QtPropertyItem*) propertyModel->itemFromIndex(index);
		QtPropertyData* data = item->GetPropertyData();
		if(NULL != data)
		{
            data->EditorDone(editor);
		}
	}

    QStyledItemDelegate::setModelData(editor, model, index);
}

void QtPropertyItemDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	QStyledItemDelegate::updateEditorGeometry(editor, option, index);

	// tune widget border and geometry
	if(NULL != editor)
	{
		editor->setObjectName("customPropertyEditor");
		editor->setStyleSheet("#customPropertyEditor{ border: 2px solid gray; }");
		QRect r = option.rect;
		r.adjust(0, -2, 0, 2);
		editor->setGeometry(r);
	}
}

void QtPropertyItemDelegate::recalcOptionalWidgets(const QModelIndex &index, QStyleOptionViewItem *option) const
{
	QtPropertyData* data = index.data(QtPropertyItem::PropertyDataRole).value<QtPropertyData*>();

	if(NULL != data)
	{
		QWidget *owViewport = data->GetOWViewport();

		for (int i = data->GetOWCount() - 1; i >= 0; i--)
		{
			int prevOWSpace = 0;
			int owSpacing = 1;

			const QtPropertyOW *ow = data->GetOW(i);
			if(NULL != ow && NULL != owViewport && NULL != ow->widget)
			{
				QWidget *owWidget = ow->widget;
				int owWidth = ow->size.width();

				if(0 != owWidth)
				{
					QRect owRect = option->rect;
					owRect.setLeft(owRect.right() - owWidth - prevOWSpace);

					owWidget->setGeometry(owRect);
					owWidget->show();

					// if this widget isn't overlayed we should modify rect for tree view cell to be drawn in.
					if(!ow->overlay)
					{
						option->rect.setRight(owRect.left());
					}

					prevOWSpace += (owWidth + owSpacing);
				}
				else
				{
					owWidget->hide();
				}
			}
		}
	}
}

void QtPropertyItemDelegate::collapse(const QModelIndex & collapse_index)
{
	// hide all optional widgets from child
	// they will be shown after expanding them and on first paint call
	if(collapse_index.isValid())
	{
		const QAbstractItemModel *model = collapse_index.model();

		if(NULL != model)
		{
			// go thought columns and hide OW in each cell PropertyData
			for (int c = 0; c < model->columnCount(); c++)
			{
				QModelIndex cellIndex = model->index(collapse_index.row(), c, collapse_index.parent());
				QtPropertyData* cellData = cellIndex.data(QtPropertyItem::PropertyDataRole).value<QtPropertyData*>();
				if(NULL != cellData)
				{
					hideAllChildOptionalWidgets(cellData);
				}
			}
		}
	}
}

void QtPropertyItemDelegate::expand(const QModelIndex & index)
{ }

void QtPropertyItemDelegate::hideAllChildOptionalWidgets(QtPropertyData* data)
{
	for(int i = 0; i < data->ChildCount(); i++)
	{
		QPair<QString, QtPropertyData *> childPair = data->ChildGet(i);
		QtPropertyData *childData = childPair.second;

		for (int j = 0; j < childData->GetOWCount(); j++)
		{
			childData->GetOW(j)->widget->hide();
		}

		hideAllChildOptionalWidgets(childData);
	}
}