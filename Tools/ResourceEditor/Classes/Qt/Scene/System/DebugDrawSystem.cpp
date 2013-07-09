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

#include "Scene/SceneEditor2.h"
#include "Scene/System/DebugDrawSystem.h"
#include "Scene/System/SelectionSystem.h"

// framework
#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Particles/ParticleEmitter.h"
//#include "Particles/ParticleEffect.h"
#include "Scene3D/Components/RenderComponent.h"

DebugDrawSystem::DebugDrawSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
{

}

DebugDrawSystem::~DebugDrawSystem()
{

}

void DebugDrawSystem::Update(DAVA::float32 timeElapsed)
{

}

void DebugDrawSystem::ProcessUIEvent(DAVA::UIEvent *event)
{

}

void DebugDrawSystem::Draw()
{
    int size = entities.size();
	for(int i = 0; i < size; ++i)
	{
		DAVA::Entity * entity = entities[i];
		
		DAVA::RenderComponent * renderComponent = static_cast<DAVA::RenderComponent*>(entity->GetComponent(DAVA::Component::RENDER_COMPONENT));
		
		if (renderComponent)
		{		
			DAVA::ParticleEmitter *emitter = dynamic_cast<DAVA::ParticleEmitter*>(renderComponent->GetRenderObject());
			if (emitter)
			{
				if (emitter->radius)
				{
					float rad = emitter->radius->GetValue(0.0f);
				}
				
				//DAVA::RenderManager* rm = DAVA::RenderManager::Instance();
				//DAVA::RenderHelper* rh = DAVA::RenderHelper::Instance();
				//DAVA::uint32 oldState = rm->GetState();
				
				//rm->SetState(DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_WRITE | DAVA::RenderState::STATE_DEPTH_TEST);
				//rm->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
				
				/*SceneSelectionSystem *selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
				//((SceneEditor2 *) GetScene())->cam
				if(NULL != selectionSystem)
				{
					const EntityGroup* selection = selectionSystem->GetSelection();
					if(NULL != selection)
					{
					//	rh->DrawCornerBox(DAVA::AABBox3(selection->GetCommonBbox().GetCenter(), 0.5f));
					//	rh->FillArrow(selection->GetCommonBbox().GetCenter(), DAVA::Vector3(10, 0, 10), 1, 1);
					}
				}		*/
						
				//DAVA::Vector3 pos = emitter->GetPosition();
				DAVA::AABBox3 box = entity->GetWTMaximumBoundingBoxSlow();
				DAVA::Vector3 center = box.GetCenter();
				//rh->DrawCornerBox(DAVA::AABBox3(center, 0.5f));
				
				//rm->SetColor(DAVA::Color(1.0f, 1.0f, 1.0f, 0.15f));
				//rh->FillBox(DAVA::AABBox3(center, 0.5f));
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f));
				DAVA::RenderHelper::Instance()->DrawCornerBox(DAVA::AABBox3(center, 0.5f));
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 1.0f, 0.15f));
				DAVA::RenderHelper::Instance()->FillBox(DAVA::AABBox3(center, 0.5f));
				
				//rm->ResetColor();
				//rm->SetState(oldState);
				//rm->SetState(DAVA::RenderState::DEFAULT_3D_STATE);
	
			//	rh->DrawCircle(DAVA::Vector3(20,0,0), 25.0f);
				/*DAVA::Polygon3 poly;
				poly.AddPoint(DAVA::Vector3(10,0,0));
				poly.AddPoint(DAVA::Vector3(10,0,10));
				poly.AddPoint(DAVA::Vector3(25,0,10));
				poly.AddPoint(DAVA::Vector3(25,0,0));*/

				//rh->DrawLine(DAVA::Vector3(0,0,0), DAVA::Vector3(-50, -50, 0));
				//rh->FillPolygon(poly);
				//rh->DrawSphere(DAVA::Vector3(0,0,0), 10.0f);
				//rh->FillSphere(DAVA::Vector3(0,0,0), 1.0f);
				//rh->DrawDodecahedron(DAVA::Vector3(0,0,0), 25.0f);
				//rh->
				//rh->DrawPoint(DAVA::Vector3(-50,0,0), 50.0f);


			}
		}
	}

}

void DebugDrawSystem::AddEntity(DAVA::Entity * entity)
{
	entities.push_back(entity);
}

void DebugDrawSystem::RemoveEntity(DAVA::Entity * entity)
{
	
    int size = entities.size();
	for(int i = 0; i < size; ++i)
	{
		if(entities[i] == entity)
		{
			entities[i] = entities[size-1];
			entities.pop_back();
			return;
		}
	}
}

void DebugDrawSystem::ProcessCommand(const Command2 *command, bool redo)
{

}


