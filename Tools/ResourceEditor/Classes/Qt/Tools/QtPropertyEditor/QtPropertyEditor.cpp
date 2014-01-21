/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include <QMouseEvent>
#include <QHeaderView>
#include <QPainter>
#include <QCoreApplication>

#include "QtPropertyEditor.h"
#include "QtPropertyItemDelegate.h"

QtPropertyEditor::QtPropertyEditor(QWidget *parent /* = 0 */)
: QTreeView(parent)
, updateTimeout(0)
, doUpdateOnPaintEvent(false)
, lastHoverData(NULL)
{
	curModel = new QtPropertyModel(viewport());
	setModel(curModel);

	curItemDelegate = new QtPropertyItemDelegate(curModel);
	setItemDelegate(curItemDelegate);

	QObject::connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(OnItemClicked(const QModelIndex &)));
	QObject::connect(curModel, SIGNAL(PropertyEdited(const QModelIndex &)), this, SLOT(OnItemEdited(const QModelIndex &)));
	QObject::connect(curModel, SIGNAL(PropertyExpanded(const QModelIndex &, bool)), this, SLOT(OnItemExpanded(const QModelIndex &, bool)));
	QObject::connect(&updateTimer, SIGNAL(timeout()), this, SLOT(OnUpdateTimeout()));

	setMouseTracking(true);
}

QtPropertyEditor::~QtPropertyEditor()
{ }

QModelIndex QtPropertyEditor::AppendProperty(const QString &name, QtPropertyData* data, const QModelIndex &parent)
{
	lastHoverData = NULL;
	return curModel->AppendProperty(name, data, parent);
}

QModelIndex QtPropertyEditor::InsertProperty(const QString &name, QtPropertyData* data, int row, const QModelIndex &parent)
{
	lastHoverData = NULL;
	return curModel->InsertProperty(name, data, row, parent);
}

QModelIndex QtPropertyEditor::AppendHeader(const QString &text)
{
	QModelIndex propHeader = AppendProperty(text, new QtPropertyData(""));

	ApplyStyle(GetProperty(propHeader), HEADER_STYLE);
	return propHeader;
}

QModelIndex QtPropertyEditor::InsertHeader(const QString &text, int row)
{
	QModelIndex propHeader = InsertProperty(text, new QtPropertyData(""), row);

	ApplyStyle(GetProperty(propHeader), HEADER_STYLE);
	return propHeader;
}

QtPropertyData* QtPropertyEditor::GetProperty(const QModelIndex &index) const
{
	return curModel->itemFromIndex(index);
}

QtPropertyData * QtPropertyEditor::GetRootProperty() const
{
	return curModel->rootItem();
}

void QtPropertyEditor::RemoveProperty(const QModelIndex &index)
{
	lastHoverData = NULL;
	curModel->RemoveProperty(index);
}

void QtPropertyEditor::RemoveProperty(QtPropertyData *data)
{
	lastHoverData = NULL;
	curModel->RemoveProperty(curModel->indexFromItem(data));
}

void QtPropertyEditor::RemovePropertyAll()
{
	lastHoverData = NULL;
	curModel->RemovePropertyAll();
}

void QtPropertyEditor::SetFilter(const QString &regex)
{
	//curFilteringModel->setFilterRegExp(regex);
}

void QtPropertyEditor::SetEditTracking(bool enabled)
{
	curModel->SetEditTracking(enabled);
}

bool QtPropertyEditor::GetEditTracking() const
{
	return curModel->GetEditTracking();
}

void QtPropertyEditor::SetUpdateTimeout(int ms)
{
	updateTimeout = ms;

	if(0 != updateTimeout)
	{
		updateTimer.start(updateTimeout);
	}
	else
	{
		updateTimer.stop();
	}
}

int QtPropertyEditor::GetUpdateTimeout()
{
	return updateTimeout;
}

void QtPropertyEditor::Update()
{
	OnUpdateTimeout();
}

void QtPropertyEditor::OnUpdateTimeout()
{
	if(state() != QAbstractItemView::EditingState)
	{
		doUpdateOnPaintEvent = false;
		curModel->UpdateStructure();
	}

	SetUpdateTimeout(updateTimeout);
}

void QtPropertyEditor::expand(const QModelIndex & index)
{
	QTreeView::expand(index);

	QtPropertyData *data = GetProperty(index);
	if(NULL != data)
	{
		data->SetExpanded(true);
	}
}

void QtPropertyEditor::expandAll()
{
	QTreeView::expandAll();
	UpdateExpandState(rootIndex());
}

void QtPropertyEditor::expandToDepth(int depth)
{
	QTreeView::expandToDepth(depth);
	UpdateExpandState(rootIndex());
}

void QtPropertyEditor::collapse(const QModelIndex & index)
{
	QTreeView::expand(index);

	QtPropertyData *data = GetProperty(index);
	if(NULL != data)
	{
		data->SetExpanded(true);
	}
}

void QtPropertyEditor::collapseAll()
{
	QTreeView::collapseAll();
	UpdateExpandState(rootIndex());
}

void QtPropertyEditor::drawRow(QPainter * painter, const QStyleOptionViewItem &option, const QModelIndex & index) const
{
	QColor gridColor = option.palette.color(QPalette::Normal, QPalette::Window);

	// draw horizontal bottom line
	painter->setPen(gridColor);
	painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());

	// adjust rect, so that grid line wont be overdrawn
	QStyleOptionViewItemV4 opt = option;
	opt.rect.adjust(0, 0, 0, -1);

	// draw row
	QTreeView::drawRow(painter, opt, index);

	// draw vertical line
	if(!(option.state & QStyle::State_Selected))
	{
		QHeaderView *hdr = header();
		if(NULL != hdr && hdr->count() > 1)
		{
			int sz = hdr->sectionSize(0);

			QPoint p1 = option.rect.topLeft();
			QPoint p2 = option.rect.bottomLeft();

			p1.setX(p1.x() + sz - 1);
			p2.setX(p2.x() + sz - 1);

			painter->setPen(gridColor);
			painter->drawLine(p1, p2);
		}
	}
}

void QtPropertyEditor::mouseMoveEvent(QMouseEvent * event)
{
	OnHover(indexAt(event->pos()));
	QTreeView::mouseMoveEvent(event);
}

void QtPropertyEditor::mousePressEvent(QMouseEvent * event)
{
	bool isExpandCase = false;
	bool allowExpand = false;

	QModelIndex index = indexAt(event->pos());
	QtPropertyData *data = GetProperty(index);

	OnHover(index);

	if(style()->styleHint(QStyle::SH_Q3ListViewExpand_SelectMouseType, 0, this) == QEvent::MouseButtonPress)
	{
		isExpandCase = true;

		if(NULL != data)
		{
			allowExpand = data->IsExpandable();
		}
	}

	if(!isExpandCase || (isExpandCase && allowExpand))
	{
		QTreeView::mousePressEvent(event);
	}
	
	if(isExpandCase && allowExpand)
	{
		// is this case expand/collapse user action was handled by tree view
		// we should update expanded state to item
		data->SetExpanded(isExpanded(index));
	}
}

void QtPropertyEditor::mouseReleaseEvent(QMouseEvent * event)
{
	OnHover(indexAt(event->pos()));
	QTreeView::mouseReleaseEvent(event);
}

void QtPropertyEditor::leaveEvent(QEvent * event)
{
	OnHover(QModelIndex());
	QTreeView::leaveEvent(event);
}

void QtPropertyEditor::OnHover(const QModelIndex &index)
{
	QtPropertyData *data = GetProperty(index);

	if(data != lastHoverData)
	{
		QtPropertyData *prevData = lastHoverData;
		lastHoverData = data;

		QModelIndex dataIndex = index.sibling(index.row(), 1);

		if(NULL != prevData)
		{
			for(int i = 0; i < prevData->GetButtonsCount(); ++i)
			{
				QtPropertyToolButton *btn = prevData->GetButton(i);
				btn->activeIndex = QModelIndex();
				btn->hide();
			}
		}

		if(NULL != data)
		{
			for(int i = 0; i < data->GetButtonsCount(); ++i)
			{
				QtPropertyToolButton *btn = data->GetButton(i);
				btn->activeIndex = dataIndex;
				btn->show();
			}

			update(dataIndex);
		}
	}
}

void QtPropertyEditor::ApplyStyle(QtPropertyData *data, int style)
{
	if(NULL != data)
	{
		switch(style)
		{
		case DEFAULT_STYLE:
			data->ResetStyle();
			break;

		case HEADER_STYLE:
			{
				QFont boldFont = data->GetFont();
				boldFont.setBold(true);
				data->SetFont(boldFont);

				data->SetBackground(QBrush(QColor(Qt::lightGray)));
				data->SetBackground(QBrush(QColor(Qt::lightGray)));
			}
			break;

		default:
			break;
		}
	}
}

QToolButton* QtPropertyEditor::GetButton(QMouseEvent * event)
{
	QToolButton *ret = NULL;
	QtPropertyData *data = GetProperty(indexAt(event->pos()));

	if(NULL != data)
	{
		for(int i = 0; i < data->GetButtonsCount(); ++i)
		{
			QToolButton *btn = data->GetButton(i);
			QPoint p = event->pos() - btn->pos();

			if(p.x() >= 0 && p.y() >= 0 && p.x() < btn->width() && p.y() < btn->height())
			{
				ret = btn;
			}
		}
	}

	return ret;
}

void QtPropertyEditor::paintEvent(QPaintEvent * event)
{
	QTreeView::paintEvent(event);
}

void QtPropertyEditor::OnItemClicked(const QModelIndex &index)
{
	edit(index, QAbstractItemView::DoubleClicked, NULL);
}

void QtPropertyEditor::OnItemEdited(const QModelIndex &index)
{
	lastHoverData = NULL;
	emit PropertyEdited(index);
}

void QtPropertyEditor::OnItemExpanded(const QModelIndex &index, bool state)
{
	if(isExpanded(index) != state)
	{
		setExpanded(index, state);
	}
}

void QtPropertyEditor::UpdateExpandState(const QModelIndex &parent)
{
	int rowCount = model()->rowCount(parent);
	for(int i = 0; i < rowCount; ++i)
	{
		QModelIndex child = model()->index(i, 0, parent);
		QtPropertyData *data = GetProperty(child);
		if(NULL != data)
		{
			data->SetExpanded(isExpanded(child));
		}

		UpdateExpandState(child);
	}
}
