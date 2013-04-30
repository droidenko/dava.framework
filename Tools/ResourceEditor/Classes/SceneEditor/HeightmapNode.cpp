#include "HeightmapNode.h"
#include "bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "EditorScene.h"
#include "Render/Highlevel/Heightmap.h"

#include "Render/Highlevel/LandscapeCursor.h"

HeightmapNode::HeightmapNode(EditorScene * _scene, Landscape *_land)
    :   Entity()
    ,   position(0,0,0)
    ,   rotation(0)
{
    SetName("HeightmapNode");
    
    editorScene = _scene;
    SetVisible(false);

    land = _land;
    DVASSERT(land);
    
    //Get LandSize;
    Vector3 landSize;
    AABBox3 transformedBox;
    Matrix4 * worldTransformPtr = land->GetWorldTransformPtr();
    
    land->GetBoundingBox().GetTransformedBox(*worldTransformPtr, transformedBox);
    landSize = transformedBox.max - transformedBox.min;
    
    heightmap = land->GetHeightmap();
    sizeInMeters = landSize.x;
    areaScale = sizeInMeters / heightmap->Size();
    maxHeight = landSize.z;
    heightScale = maxHeight / 65535.f;
    float32 minHeight = 0.0f;
    
    
    uint16 *dt = heightmap->Data();
    size.x = (float32)heightmap->Size();
    size.y = (float32)heightmap->Size();
    hmap.resize(heightmap->Size() * heightmap->Size());

	for (int32 y = 0; y < heightmap->Size(); ++y)
	{
		for (int32 x = 0; x < heightmap->Size(); ++x)
		{
            int32 index = x + (y) * heightmap->Size();
            float32 mapValue = dt[index] * heightScale;
            SetValueToMap(x, y, mapValue, transformedBox);
		}
	}

	bool flipQuadEdges = true;

    colShape = new btHeightfieldTerrainShape(heightmap->Size(), heightmap->Size()
                                             , &hmap.front(), heightScale, minHeight, maxHeight
                                             , 2, PHY_FLOAT
                                             , flipQuadEdges);

    colShape->setLocalScaling(btVector3(areaScale, areaScale, 1.0f));
    
    // Create Dynamic Objects
    btTransform startTransform;
    startTransform.setIdentity();
    
    btScalar	mass(0.0f);

    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);
    
    btVector3 localInertia(0,0,0);
    if (isDynamic)
        colShape->calculateLocalInertia(mass,localInertia);
    
    startTransform.setOrigin(btVector3( btScalar(position.x),
                                       btScalar(position.y),
                                       btScalar(maxHeight/2.0f + position.z)));
    
    
    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    motionSate = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionSate,colShape,localInertia);
    rbInfo.m_friction = 0.9f;
    body = new btRigidBody(rbInfo);
    
    collisionObject = new btCollisionObject();
    collisionObject->setWorldTransform(startTransform);
    collisionObject->setCollisionShape(colShape);
    editorScene->landCollisionWorld->addCollisionObject(collisionObject);
	editorScene->landCollisionWorld->updateAabbs();
}

HeightmapNode::~HeightmapNode()
{
	if(editorScene->landCollisionWorld)
	{
        editorScene->landCollisionWorld->removeCollisionObject(collisionObject);
	}
    
    
    SafeDelete(body);
    
    SafeDelete(collisionObject);
    
    SafeDelete(motionSate);
    
    SafeDelete(colShape);
}


void HeightmapNode::SetValueToMap(int16 x, int16 y, float32 height, const AABBox3 &box)
{
    int32 index = x + y * heightmap->Size();

    hmap[index] = height;
}

void HeightmapNode::UpdateHeightmapRect(const Rect &rect)
{
    AABBox3 transformedBox;
    land->GetBoundingBox().GetTransformedBox(*land->GetWorldTransformPtr(), transformedBox);
    
    int32 x = (int32)rect.x;
    int32 y = (int32)rect.y;
    int32 endX = (int32)(rect.x + rect.dx);
    int32 endY = (int32)(rect.y + rect.dy);

    uint16 *dt = heightmap->Data();
    for (int32 yy = y; yy < endY; ++yy)
    {
		for (int32 xx = x; xx < endX; ++xx)
		{
            float32 mapValue = dt[(xx + (yy) * heightmap->Size())] * heightScale;
            SetValueToMap(xx, yy, mapValue, transformedBox);
		}
	}
}

float32 HeightmapNode::GetAreaScale()
{
    return areaScale;
}

const Vector3 &HeightmapNode::GetSize()
{
    return size;
}

float32 HeightmapNode::GetSizeInMeters()
{
    return sizeInMeters;
}

