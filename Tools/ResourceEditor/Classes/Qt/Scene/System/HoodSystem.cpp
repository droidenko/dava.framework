/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Scene/System/HoodSystem.h"
#include "Scene/System/ModifSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/SceneEditor2.h"

HoodSystem::HoodSystem(DAVA::Scene * scene, SceneCameraSystem *camSys)
	: DAVA::SceneSystem(scene)
	, cameraSystem(camSys)
	, curMode(ST_MODIF_OFF)
	, moseOverAxis(ST_AXIS_NONE)
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

	SetModifAxis(ST_AXIS_X);
	SetModifMode(ST_MODIF_MOVE);

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

void HoodSystem::SetModifMode(ST_ModifMode mode)
{
	if(curMode != mode)
	{
		if(NULL != curHood)
		{
			RemCollObjects(&curHood->collObjects);
		}

		curMode = mode;
		switch (mode)
		{
		case ST_MODIF_MOVE:
			curHood = &moveHood;
			break;
		case ST_MODIF_SCALE:
			curHood = &scaleHood;
			break;
		case ST_MODIF_ROTATE:
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

ST_ModifMode HoodSystem::GetModifMode() const
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
		moseOverAxis = ST_AXIS_NONE;
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
		ST_Axis showAsSelected = curAxis;

		if(curMode != ST_MODIF_OFF)
		{
			if(ST_AXIS_NONE != moseOverAxis)
			{
				showAsSelected = moseOverAxis;
			}
		}

		curHood->Draw(showAsSelected, moseOverAxis);

		// debug draw axis collision word
		//collWorld->debugDrawWorld();
	}
}

void HoodSystem::PropeccCommand(const Command2 *command, bool redo)
{

}

void HoodSystem::SetModifAxis(ST_Axis axis)
{
	if(ST_AXIS_NONE != axis)
	{
		curAxis = axis;
	}
}

ST_Axis HoodSystem::GetModifAxis() const
{
	return curAxis;
}

ST_Axis HoodSystem::GetPassingAxis() const
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
