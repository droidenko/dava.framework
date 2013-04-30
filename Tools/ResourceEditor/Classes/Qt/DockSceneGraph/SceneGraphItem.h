#ifndef __SCENE_GRAPH_ITEM_H__
#define __SCENE_GRAPH_ITEM_H__

#include "GraphItem.h"
#include "Qt/DockSceneGraph/ExtraUserData.h"

class SceneGraphItem: public GraphItem
{
public:
    SceneGraphItem(GraphItem *parent = 0);
    virtual ~SceneGraphItem();
    
	virtual QVariant Data(int32 column);
	virtual void SetUserData(void *data);

    // Extra User Data for non-SceneNodes. Note - a Scene Graph item
    // keeps pointer to these data only and isn't responsible for obtaining/
    // cleanup memory for them!
    virtual void SetExtraUserData(ExtraUserData* extraData);
    virtual ExtraUserData* GetExtraUserData() const;

protected:
	virtual void ReleaseUserData();
    
private:
    ExtraUserData* extraUserData;
};

#endif // __SCENE_GRAPH_ITEM_H__
