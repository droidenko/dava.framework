#ifndef __GRAPH_ITEM_H__
#define __GRAPH_ITEM_H__

#include "DAVAEngine.h"
#include <QVariant>

using namespace DAVA;

class GraphItem: public BaseObject
{
public:
    GraphItem(GraphItem *parent = 0);
    virtual ~GraphItem();
    
	GraphItem *GetParent();
    void SetParent(GraphItem * parent);

	virtual void SetUserData(void *data) = 0;
    void * GetUserData();

    void AppendChild(GraphItem *child);
    void InsertChild(GraphItem *child, int32 pos);
    void RemoveChild(int32 row);
    void RemoveChild(GraphItem *child);
	GraphItem *Child(int32 row);
	int32 ChildrenCount() const;

	int32 Row() const;
    int32 ColumnCount() const;

	virtual QVariant Data(int32 column) = 0;

protected:

	virtual void ReleaseUserData()= 0;

protected:
	void *userData;

private:
	Vector<GraphItem *>children;
    GraphItem *parentItem;
};

#include "Qt/DockSceneGraph/PointerHolder.h"
DECLARE_POINTER_TYPE(GraphItem *);


#endif // __GRAPH_ITEM_H__
