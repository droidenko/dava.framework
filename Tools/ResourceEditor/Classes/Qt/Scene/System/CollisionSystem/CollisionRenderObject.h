#ifndef __SCENE_COLLISION_RENDER_OBJECT_H__
#define __SCENE_COLLISION_RENDER_OBJECT_H__

#include "Qt/Scene/System/CollisionSystem/CollisionBaseObject.h"
#include "Render/Highlevel/RenderObject.h"

class CollisionRenderObject : public CollisionBaseObject
{
public:
	CollisionRenderObject(DAVA::Entity *entity, btCollisionWorld *word, DAVA::RenderObject *renderObject);
	virtual ~CollisionRenderObject();

protected:
	btTriangleMesh* btTriangles;
	btCollisionShape* btShape;
};

#endif // __SCENE_COLLISION_BASE_OBJECT_H__
