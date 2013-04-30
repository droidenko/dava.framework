#include "Qt/Scene/System/CollisionSystem/CollisionRenderObject.h"


CollisionRenderObject::CollisionRenderObject(DAVA::Entity *entity, btCollisionWorld *word, DAVA::RenderObject *renderObject)
	: CollisionBaseObject(entity, word)
{
	if(NULL != renderObject && NULL != word)
	{
		bool anyPolygonAdded = false;
		DAVA::Matrix4 curEntityTransform = entity->GetWorldTransform();

		for(DAVA::uint32 i = 0; i < renderObject->GetRenderBatchCount(); ++i)
		{
			DAVA::RenderBatch* batch = renderObject->GetRenderBatch(i);
			DAVA::PolygonGroup* pg = batch->GetPolygonGroup();

			if(NULL != pg)
			{
				// is this the first polygon in cycle
				if(!anyPolygonAdded)
				{
					anyPolygonAdded = true;
					btTriangles = new btTriangleMesh();
				}

				for(int i = 0; i < pg->indexCount; i += 3 )
				{
					DAVA::uint16 index0 = pg->indexArray[i];
					DAVA::uint16 index1 = pg->indexArray[i+1];
					DAVA::uint16 index2 = pg->indexArray[i+2];

					DAVA::Vector3 v;
					pg->GetCoord(index0, v);
					v = v * curEntityTransform;
					btVector3 vertex0(v.x, v.y, v.z);

					pg->GetCoord(index1, v);
					v = v * curEntityTransform;
					btVector3 vertex1(v.x, v.y, v.z);

					pg->GetCoord(index2, v);
					v = v * curEntityTransform;
					btVector3 vertex2(v.x, v.y, v.z);

					btTriangles->addTriangle(vertex0, vertex1, vertex2, false);
				}

				// save original bbox
				boundingBox = pg->GetBoundingBox();
				
				// increase bbox a a little bit
				boundingBox.AddPoint(boundingBox.min - DAVA::Vector3(0.5f, 0.5f, 0.5f));
				boundingBox.AddPoint(boundingBox.max + DAVA::Vector3(0.5f, 0.5f, 0.5f));
			}
		}

		if(anyPolygonAdded)
		{
			DAVA::Vector3 pos = curEntityTransform.GetTranslationVector();

			btObject = new btCollisionObject();
			btShape = new btBvhTriangleMeshShape(btTriangles, true, true);

			btObject->setCollisionShape(btShape);
			btWord->addCollisionObject(btObject);
		}
	}
}

CollisionRenderObject::~CollisionRenderObject()
{
	if(NULL != btObject)
	{
		btWord->removeCollisionObject(btObject);
		DAVA::SafeDelete(btObject);
		DAVA::SafeDelete(btShape);
		DAVA::SafeDelete(btTriangles);
	}
}
