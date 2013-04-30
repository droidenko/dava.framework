#ifndef __SCENE_COLLISION_SYSTEM_H__
#define __SCENE_COLLISION_SYSTEM_H__

#include <QMap>
#include "Qt/Scene/EntityGroup.h"

// bullet
#include "bullet/btBulletCollisionCommon.h"

// framework
#include "UI/UIEvent.h"
#include "Entity/SceneSystem.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

class CollisionBaseObject;
class SceneCollisionDebugDrawer;

class SceneCollisionSystem : public DAVA::SceneSystem
{
	friend class SceneEditorProxy;
	friend class EntityModificationSystem;

public:
	enum DebugDrawFlags
	{
		DEBUG_DRAW_NOTHING = 0x0,

		DEBUG_DRAW_OBJECTS = 0x1,
		DEBUG_DRAW_SELECTED_OBJECTS = 0x2,
		DEBUG_DRAW_LAND = 0x4,
		DEBUG_DRAW_RAYTEST = 0x10,

		DEBUG_DRAW_ALL = 0xFFFFFFFF
	};

	SceneCollisionSystem(DAVA::Scene * scene);
	~SceneCollisionSystem();

	void SetDebugDrawFlags(int flags);
	int GetDebugDrawFlags();

	DAVA::AABBox3 GetBoundingBox(DAVA::Entity *entity);

	const EntityGroup* RayTest(DAVA::Vector3 from, DAVA::Vector3 to);
	const EntityGroup* RayTestFromCamera();

	void UpdateCollisionObject(DAVA::Entity *entity);

protected:
	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();

	virtual void AddEntity(DAVA::Entity * entity);
	virtual void RemoveEntity(DAVA::Entity * entity);

protected:
	int debugDrawFlags;

	DAVA::Vector3 lastRayFrom;
	DAVA::Vector3 lastRayTo;
	DAVA::Vector2 lastMousePos;

	EntityGroup rayIntersectedEntities;
	bool rayIntersectCached;

	btDefaultCollisionConfiguration* objectsCollConf;
	btCollisionDispatcher* objectsCollDisp;
	btAxisSweep3* objectsBroadphase;
	btCollisionWorld *objectsCollWorld;
	SceneCollisionDebugDrawer *objectsDebugDrawer;

	btDefaultCollisionConfiguration* landCollConf;
	btCollisionDispatcher* landCollDisp;
	btAxisSweep3* landBroadphase;
	btCollisionWorld *landCollWorld;
	SceneCollisionDebugDrawer *landDebugDrawer;

	QMap<DAVA::Entity*, CollisionBaseObject*> entityToCollision;
	QMap<btCollisionObject*, DAVA::Entity*> collisionToEntity;

	void BuildFromEntity(DAVA::Entity * entity);
	void DestroyFromEntity(DAVA::Entity * entity);
};

class SceneCollisionDebugDrawer : public btIDebugDraw
{
public:
	SceneCollisionDebugDrawer();

	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	virtual void drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color);
	virtual void reportErrorWarning(const char* warningString);
	virtual void draw3dText(const btVector3& location,const char* textString);
	virtual void setDebugMode(int debugMode);
	virtual int	getDebugMode() const;

protected:
	int dbgMode;
	DAVA::RenderManager *manager;
	DAVA::RenderHelper *helper;
};

#endif // __SCENE_COLLISION_SYSTEM_H__