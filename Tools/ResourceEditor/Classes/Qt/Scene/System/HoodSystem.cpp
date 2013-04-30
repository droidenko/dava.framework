#include "Qt/Scene/System/HoodSystem.h"
#include "Qt/Scene/System/ModifSystem.h"
#include "Qt/Scene/System/CollisionSystem.h"
#include "Qt/Scene/System/CameraSystem.h"
#include "Qt/Scene/System/SelectionSystem.h"
#include "Qt/Scene/SceneEditorProxy.h"

HoodSystem::HoodSystem(DAVA::Scene * scene, SceneCameraSystem *camSys)
	: DAVA::SceneSystem(scene)
	, cameraSystem(camSys)
	, curMode(EM_MODE_OFF)
	, moseOverAxis(EM_AXIS_NONE)
	, curHood(NULL)
	, moveHood()
	, locked(false)
	, visible(false)
{
	btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);

	collConfiguration = new btDefaultCollisionConfiguration();
	collDispatcher = new btCollisionDispatcher(collConfiguration);
	collBroadphase = new btAxisSweep3(worldMin,worldMax);
	collDebugDraw = new SceneCollisionDebugDrawer();
	collDebugDraw->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	collWorld = new btCollisionWorld(collDispatcher, collBroadphase, collConfiguration);
	collWorld->setDebugDrawer(collDebugDraw);

	SetSelectedAxis(EM_AXIS_X);
	SetType(EM_MODE_MOVE);

	moveHood.colorX = DAVA::Color(1, 0, 0, 1);
	moveHood.colorY = DAVA::Color(0, 1, 0, 1);
	moveHood.colorZ = DAVA::Color(0, 0, 1, 1);
	moveHood.colorS = DAVA::Color(1, 1, 0, 1);

	rotateHood.colorX = DAVA::Color(1, 0, 0, 1);
	rotateHood.colorY = DAVA::Color(0, 1, 0, 1);
	rotateHood.colorZ = DAVA::Color(0, 0, 1, 1);
	rotateHood.colorS = DAVA::Color(1, 1, 0, 1);

	scaleHood.colorX = DAVA::Color(1, 0, 0, 1);
	scaleHood.colorY = DAVA::Color(0, 1, 0, 1);
	scaleHood.colorZ = DAVA::Color(0, 0, 1, 1);
	scaleHood.colorS = DAVA::Color(1, 1, 0, 1);

	normalHood.colorX = DAVA::Color(0, 0, 0, 0.3f);
	normalHood.colorY = DAVA::Color(0, 0, 0, 0.3f);
	normalHood.colorZ = DAVA::Color(0, 0, 0, 0.3f);
	normalHood.colorS = DAVA::Color(1, 0, 0, 0.3f);
}

HoodSystem::~HoodSystem()
{
	delete collWorld;
	delete collDebugDraw;
	delete collBroadphase;
	delete collDispatcher;
	delete collConfiguration;
}

DAVA::Vector3 HoodSystem::GetPosition() const
{
	return (curPos + curOffset);
}

void HoodSystem::SetPosition(const DAVA::Vector3 &pos)
{
	if(!locked)
	{
		if(curPos != pos)
		{
			curPos = pos;
			curOffset = DAVA::Vector3(0, 0, 0);

			if(NULL != curHood)
			{
				curHood->UpdatePos(curPos);
			}
		}
	}
}

void HoodSystem::MovePosition(const DAVA::Vector3 &offset)
{
	if(curOffset != offset)
	{
		curOffset = offset;

		if(NULL != curHood)
		{
			curHood->UpdatePos(curPos + curOffset);
		}
	}
}

void HoodSystem::SetScale(DAVA::float32 scale)
{
	if(curScale != scale)
	{
		curScale = scale;

		if(NULL != curHood)
		{
			curHood->UpdateScale(curScale);
			collWorld->updateAabbs();
		}
	}
}

void HoodSystem::SetType(int type)
{
	if(curMode != type )
	{
		if(NULL != curHood)
		{
			RemCollObjects(&curHood->collObjects);
		}

		curMode = type;
		switch (type)
		{
		case EM_MODE_MOVE:
			curHood = &moveHood;
			break;
		case EM_MODE_SCALE:
			curHood = &scaleHood;
			break;
		case EM_MODE_ROTATE:
			curHood = &rotateHood;
			break;
		default:
			curHood = &normalHood;
			break;
		}

		if(NULL != curHood)
		{
			AddCollObjects(&curHood->collObjects);

			curHood->UpdatePos(curPos + curOffset);
			curHood->UpdateScale(curScale);
		}

		collWorld->updateAabbs();
	}
}

int HoodSystem::GetType() const
{
	return curMode;
}

void HoodSystem::AddCollObjects(const DAVA::Vector<HoodCollObject*>* objects)
{
	if(NULL != objects)
	{
		for (size_t i = 0; i < objects->size(); ++i)
		{
			collWorld->addCollisionObject(objects->operator[](i)->btObject);
		}
	}
}

void HoodSystem::RemCollObjects(const DAVA::Vector<HoodCollObject*>* objects)
{
	if(NULL != objects)
	{
		for (size_t i = 0; i < objects->size(); ++i)
		{
			collWorld->removeCollisionObject(objects->operator[](i)->btObject);
		}
	}
}


void HoodSystem::Update(float timeElapsed)
{
	if(visible && !locked)
	{
		// scale hood depending on current camera position
		DAVA::Vector3 camPosition = cameraSystem->GetCameraPosition();
		SetScale((GetPosition() - camPosition).Length() / 20.f);
	}
}

void HoodSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	// before checking result mark that there is no hood axis under mouse
	if(!locked)
	{
		moseOverAxis = EM_AXIS_NONE;
	}
	
	// if is visible and not locked check mouse over status
	if(visible && !locked && NULL != curHood)
	{
		// get intersected items in the line from camera to current mouse position
		DAVA::Vector3 camPosition = cameraSystem->GetCameraPosition();
		DAVA::Vector3 camToPointDirection = cameraSystem->GetPointDirection(event->point);
		DAVA::Vector3 traceTo = camPosition + camToPointDirection * 1000.0f;

		btVector3 btFrom(camPosition.x, camPosition.y, camPosition.z);
		btVector3 btTo(traceTo.x, traceTo.y, traceTo.z);

		btCollisionWorld::AllHitsRayResultCallback btCallback(btFrom, btTo);
		collWorld->rayTest(btFrom, btTo, btCallback);

		if(btCallback.hasHit())
		{
			const DAVA::Vector<HoodCollObject*>* curHoodObjects = &curHood->collObjects;
			for(size_t i = 0; i < curHoodObjects->size(); ++i)
			{
				HoodCollObject *hObj = curHoodObjects->operator[](i);

				if(hObj->btObject == btCallback.m_collisionObjects[0])
				{
					// mark that mouse is over one of hood axis
					moseOverAxis = hObj->axis;
					break;
				}
			}
		}
	}
}

void HoodSystem::Draw()
{
	if(visible && NULL != curHood)
	{
		int showAsSelected = curAxis;

		if(curMode != EM_MODE_OFF)
		{
			if(EM_AXIS_NONE != moseOverAxis)
			{
				showAsSelected = moseOverAxis;
			}
		}

		curHood->Draw(showAsSelected, moseOverAxis);

		// debug draw collision word
		//collWorld->debugDrawWorld();
	}
}

void HoodSystem::SetSelectedAxis(int axis)
{
	if(EM_AXIS_NONE != axis)
	{
		curAxis = axis;
	}
}

int HoodSystem::GetSelectedAxis() const
{
	return curAxis;
}

int HoodSystem::GetPassingAxis() const
{
	return moseOverAxis;
}

void HoodSystem::Lock()
{
	locked = true;
}

void HoodSystem::Unlock()
{
	locked = false;
}

void HoodSystem::Show()
{
	visible = true;
}

void HoodSystem::Hide()
{
	visible = false;
}
