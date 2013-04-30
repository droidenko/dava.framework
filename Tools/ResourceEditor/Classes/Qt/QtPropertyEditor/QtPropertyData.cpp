#include <QPushButton>
#include "Qt/QtPropertyEditor/QtPropertyData.h"

QtPropertyData::QtPropertyData()
	: curFlags(0)
	, parent(NULL)
	, optionalWidgetViewport(NULL)
{ }

QtPropertyData::QtPropertyData(const QVariant &value)
	: curValue(value)
	, curFlags(0)
	, parent(NULL)
	, optionalWidgetViewport(NULL)
{ }

QtPropertyData::~QtPropertyData() 
{
	QHashIterator<QString, QtPropertyData*> i = QHashIterator<QString, QtPropertyData*>(children);
	while(i.hasNext())
	{
		i.next();

		QtPropertyData *data = i.value();
		if(NULL != data)
		{
			delete data;
		}
	}

	for (int i = 0; i < optionalWidgets.size(); i++)
	{
		if(NULL != optionalWidgets.at(i).widget)
		{
			delete optionalWidgets.at(i).widget;
		}
	}
}

QVariant QtPropertyData::GetValue()
{
	QVariant value = GetValueInternal();

	if(value != curValue)
	{
		curValue = value;
		ChildNeedUpdate();
	}

	return curValue;
}

void QtPropertyData::SetValue(const QVariant &value)
{
	SetValueInternal(value);
	curValue = value;

	ChildNeedUpdate();
	ParentUpdate();
}

void QtPropertyData::SetIcon(const QIcon &icon)
{
	curIcon = icon;
}

QIcon QtPropertyData::GetIcon()
{
	return curIcon;
}

int QtPropertyData::GetFlags()
{
	return curFlags;
}

void QtPropertyData::SetFlags(int flags)
{
	curFlags = flags;
}

QWidget* QtPropertyData::CreateEditor(QWidget *parent, const QStyleOptionViewItem& option) 
{ 
	return CreateEditorInternal(parent, option);
}

void QtPropertyData::EditorDone(QWidget *editor)
{
    return EditorDoneInternal(editor);
}

void QtPropertyData::SetEditorData(QWidget *editor)
{
    return SetEditorDataInternal(editor);
}

void QtPropertyData::ParentUpdate()
{
	if(NULL != parent)
	{
		parent->ChildChanged(parent->children.key(this), this);
		parent->ParentUpdate();
	}
}

void QtPropertyData::ChildAdd(const QString &key, QtPropertyData *data)
{
	if(NULL != data && !key.isEmpty())
	{
		int sz = children.size();
        
        children.insert(key, data);
        childrenOrder.insert(key, sz);

		data->parent = this;

		emit ChildAdded(key, data);
	}
}

void QtPropertyData::ChildAdd(const QString &key, const QVariant &value)
{
	ChildAdd(key, new QtPropertyData(value));
}

int QtPropertyData::ChildCount()
{
	return children.size();
}

QPair<QString, QtPropertyData*> QtPropertyData::ChildGet(int i)
{
	QPair<QString, QtPropertyData*> p("", NULL);

	if(i >= 0 && i < children.size())
	{
		p.first = childrenOrder.key(i);
		p.second = children[p.first];
	}

	return p;
}

QtPropertyData * QtPropertyData::ChildGet(const QString &key)
{
	QtPropertyData *data = NULL;

	if(children.contains(key))
	{
		data = children[key];
	}

	return data;
}

void QtPropertyData::ChildRemove(const QString &key)
{
	QtPropertyData *data = ChildGet(key);

	if(NULL != data)
	{
		emit ChildRemoving(key, data);

		children.remove(key);
		childrenOrder.remove(key);

		delete data;
	}
}

void QtPropertyData::ChildRemove(int i)
{
	QPair<QString, QtPropertyData*> pair = ChildGet(i);

	if(!pair.first.isEmpty() && NULL != pair.second)
	{
		emit ChildRemoving(pair.first, pair.second);

		children.remove(pair.first);
		childrenOrder.remove(pair.first);

		delete pair.second;
	}
}

void QtPropertyData::ChildRemove(QtPropertyData *data)
{
	QString key = children.key(data, "");
	if(!key.isEmpty())
	{
		ChildRemove(key);
	}
}

int QtPropertyData::GetOWCount()
{
	return optionalWidgets.size();
}

const QtPropertyOW* QtPropertyData::GetOW(int index)
{
	const QtPropertyOW *ret = NULL;

	if(index >= 0 && index < optionalWidgets.size())
	{
		ret = &optionalWidgets.at(index);
	}

	return ret;
}

void QtPropertyData::AddOW(const QtPropertyOW &ow)
{
	optionalWidgets.append(ow);

	if(NULL != ow.widget)
	{
		ow.widget->setParent(optionalWidgetViewport);
	}
}

void QtPropertyData::RemOW(int index)
{
	if(index >= 0 && index < optionalWidgets.size())
	{
		if(NULL != optionalWidgets.at(index).widget)
		{
			delete optionalWidgets.at(index).widget;
		}

		optionalWidgets.remove(index);
	}
}

QWidget* QtPropertyData::GetOWViewport()
{
	return optionalWidgetViewport;
}

void QtPropertyData::SetOWViewport(QWidget *viewport)
{
	optionalWidgetViewport = viewport;

	for(int i = 0; i < optionalWidgets.size(); ++i)
	{
		if(NULL != optionalWidgets.at(i).widget)
		{
			optionalWidgets.at(i).widget->setParent(viewport);
		}
	}

	QHashIterator<QString, QtPropertyData*> i(children);
	while (i.hasNext()) 
	{
		i.next();
		i.value()->SetOWViewport(viewport);
	}
}

QVariant QtPropertyData::GetValueInternal()
{
	// should be re-implemented by sub-class

	return curValue;
}

void QtPropertyData::SetValueInternal(const QVariant &value)
{
	// should be re-implemented by sub-class

	curValue = value;
}

QWidget* QtPropertyData::CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option)
{
	// should be re-implemented by sub-class

	return NULL;
}

void QtPropertyData::EditorDoneInternal(QWidget *editor)
{
	// should be re-implemented by sub-class
}

void QtPropertyData::SetEditorDataInternal(QWidget *editor)
{
	// should be re-implemented by sub-class
}

void QtPropertyData::ChildChanged(const QString &key, QtPropertyData *data)
{
	// should be re-implemented by sub-class
}

void QtPropertyData::ChildNeedUpdate()
{
	// should be re-implemented by sub-class
}

