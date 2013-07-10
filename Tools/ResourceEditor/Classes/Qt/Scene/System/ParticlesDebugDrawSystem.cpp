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
#include "Scene/System/ParticlesDebugDrawSystem.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/HoodSystem.h"

// framework
#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Components/RenderComponent.h"

#define DEFAULT_EFFECT_SIZE 0.5f

ParticlesDebugDrawSystem::ParticlesDebugDrawSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
{

}

ParticlesDebugDrawSystem::~ParticlesDebugDrawSystem()
{

}

void ParticlesDebugDrawSystem::Update(DAVA::float32 timeElapsed)
{

}

void ParticlesDebugDrawSystem::ProcessUIEvent(DAVA::UIEvent *event)
{

}

DAVA::float32 ParticlesDebugDrawSystem::GetDebugDrawRadius(DAVA::Entity* parentEntity)
{
	DAVA::float32 drawRadius = 0.1f;
	DAVA::float32 currentMax = 0.0f;
	SceneCollisionSystem *collisionSystem = ((SceneEditor2 *) GetScene())->collisionSystem;
	
	if (collisionSystem)
	{		
		for(int i = 0; i < parentEntity->GetChildrenCount(); ++i)
		{
			DAVA::Entity *entity = parentEntity->GetChild(i);
			DAVA::AABBox3 entityBox = collisionSystem->GetBoundingBox(entity);
			
			currentMax = Max(currentMax, Abs(entityBox.max.x));
			currentMax = Max(currentMax, Abs(entityBox.max.y));
			currentMax = Max(currentMax, Abs(entityBox.max.z));
			currentMax = Max(currentMax, Abs(entityBox.min.x));
			currentMax = Max(currentMax, Abs(entityBox.min.y));
			currentMax = Max(currentMax, Abs(entityBox.min.z));
		}
	}
	
	return Max(drawRadius, currentMax * 2);
}

void ParticlesDebugDrawSystem::Draw()
{
	// Draw debug information for non-selected entities
	for(int i = 0; i < entities.size(); ++i)
	{				
		DAVA::Entity *entity = entities[i];
				
		DAVA::AABBox3 box = entity->GetWTMaximumBoundingBoxSlow();		
		DAVA::Vector3 center = box.GetCenter();
		
		DAVA::float32 drawRadius = GetDebugDrawRadius(entity);
				
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.1f, 0.1f, 0.1f, 0.35f));
		DAVA::RenderHelper::Instance()->FillDodecahedron(center, drawRadius);		
	}
	
	// Draw debug information for selected entities
	SceneSelectionSystem *selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
	if(selectionSystem != NULL)
	{
		const EntityGroup *selectedEntities = selectionSystem->GetSelection();
		
		for (size_t i = 0; i < selectedEntities->Size(); i++)
		{
			DAVA::Entity *entity = selectedEntities->GetEntity(i);
			// Get center of entity object
			DAVA::AABBox3 box = entity->GetWTMaximumBoundingBoxSlow();
			DAVA::Vector3 center = box.GetCenter();
			
			DAVA::AABBox3 selectionBox = selectedEntities->GetBbox(i);
			
			DAVA::RenderComponent *renderComponent = static_cast<DAVA::RenderComponent*>(entity->GetComponent(DAVA::Component::RENDER_COMPONENT));
		
			if (renderComponent)
			{		
				DAVA::ParticleEmitter *emitter = dynamic_cast<DAVA::ParticleEmitter*>(renderComponent->GetRenderObject());
				// Draw additional effects according to emitter type
				if (emitter)
				{
					DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.7f, 0.0f, 0.0f, 0.25f));
					switch (emitter->emitterType)
					{
						case DAVA::ParticleEmitter::EMITTER_ONCIRCLE:
						case DAVA::ParticleEmitter::EMITTER_SHOCKWAVE:
						{
							float emitterRadius = DEFAULT_EFFECT_SIZE;
							if (emitter->radius)
							{
								emitterRadius = emitter->radius->GetValue(emitter->GetLifeTime());
							}
							DAVA::RenderHelper::Instance()->FillDodecahedron(center, emitterRadius);
						}
						break;
					
						case DAVA::ParticleEmitter::EMITTER_RECT:
						{
							DrawSizeBox(emitter, center);
							DrawVectorArrow(emitter, center);
						}
						break;
					
						case DAVA::ParticleEmitter::EMITTER_POINT:
						{
							DrawVectorArrow(emitter, center);
							DAVA::RenderHelper::Instance()->FillDodecahedron(center, 0.05f);
						}
						break;
					} // switch
				} // if
			} // if
		} // for
	} // if
	
	DAVA::RenderManager::Instance()->ResetColor();
}

void ParticlesDebugDrawSystem::DrawSizeBox(DAVA::ParticleEmitter *emitter, DAVA::Vector3 center)
{
	// Default value of emitter size
	DAVA::Vector3 emitterSize(DEFAULT_EFFECT_SIZE, DEFAULT_EFFECT_SIZE, DEFAULT_EFFECT_SIZE);

	DAVA::Vector3 min;
	DAVA::Vector3 max;
	
	if (emitter->size)
	{
		emitterSize = emitter->size->GetValue(emitter->GetLifeTime());
	}
	
	float halfSizeX = emitterSize.x / 2;
	float halfSizeY = emitterSize.y / 2;
	float halfSizeZ = emitterSize.z / 2;
	
	// Calculate box min and max values
	min.x = center.x - halfSizeX;
	min.y = center.y - halfSizeY;
	min.z = center.z - halfSizeZ;
	
	max.x = center.x + halfSizeX;
	max.y = center.y + halfSizeY;
	max.z = center.z + halfSizeZ;
	
	DAVA::RenderHelper::Instance()->FillBox(DAVA::AABBox3(min, max));
}

void ParticlesDebugDrawSystem::DrawVectorArrow(DAVA::ParticleEmitter *emitter, DAVA::Vector3 center)
{
	DAVA::Vector3 emitterVector;
	DAVA::float32 arrowBaseSize = 5.0f;
				
	if (emitter->emissionVector)
	{
		emitterVector = emitter->emissionVector->GetValue(emitter->GetLifeTime());
	}
	
	DAVA::float32 scale = 1.0f;
	// Get current scale from HoodSystem
	HoodSystem *hoodSystem = ((SceneEditor2 *) GetScene())->hoodSystem;
	if(hoodSystem != NULL)
	{
		scale = hoodSystem->GetScale();
	}
	
	//DAVA::float32 coefx = Abs(arrowBaseSize / (emitterVector.x != 0 ? emitterVector.x : 1.0f));
	//DAVA::float32 coefy = Abs(arrowBaseSize / (emitterVector.y != 0 ? emitterVector.y : 1.0f));
	//DAVA::float32 coefz = Abs(arrowBaseSize / (emitterVector.z != 0 ? emitterVector.z : 1.0f));
	
	//DAVA::float32 coef = Max(coefx, coefy);
	//coef = Max(coef, coefz);
	//coef /= arrowBaseSize;
	
	//emitterVector = arrowBaseSize / emitterVector;
	
	emitterVector = (emitterVector * scale * arrowBaseSize) + center;
	
	//emitterVector = (emitterVector * arrowBaseSize * scale) + center;
	
	DAVA::float32 arrowSize = (arrowBaseSize * scale) / 6;
	
	//DAVA::RenderHelper::Instance()->DrawLine(center, emitterVector);
	
	DAVA::RenderHelper::Instance()->FillArrow(center, emitterVector, arrowSize, 1);
}

void ParticlesDebugDrawSystem::AddEntity(DAVA::Entity * entity)
{
	entities.push_back(entity);
}

void ParticlesDebugDrawSystem::RemoveEntity(DAVA::Entity * entity)
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

void ParticlesDebugDrawSystem::ProcessCommand(const Command2 *command, bool redo)
{

}



