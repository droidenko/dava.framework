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

#include "Render/Material.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Render/Highlevel/RenderFastNames.h"
#include "Scene3D/Components/SkyboxComponent.h"
#include "Scene3D/Systems/SkyboxSystem.h"
#include "Scene3D/Entity.h"

//do not create lower cube face
const int SKYBOX_VERTEX_COUNT = (5 * 6);

namespace DAVA
{
	SkyboxSystem::SkyboxSystem(Scene * scene)
	:	BaseProcessSystem(Component::SKYBOX_COMPONENT, scene)
	{
		skyboxEntity = NULL;
		state = SYSTEM_NONE;
		skyboxMaterial = NULL;
	}
	
	SkyboxSystem::~SkyboxSystem()
	{
		SafeRelease(skyboxEntity);
	}
	
	void SkyboxSystem::Process()
	{
		if(SYSTEM_DIRTY == state &&
		   skyboxEntity)
		{
			UpdateSkybox();
			
			state = SYSTEM_READY;
		}
		else if(SYSTEM_INITIAL_STATE == state &&
				skyboxEntity)
		{
			CreateSkybox();
			UpdateSkybox();
			
			state = SYSTEM_READY;
		}
	}
	
	void SkyboxSystem::AddEntity(Entity * entity)
	{
		DVASSERT(NULL != entity);
		DVASSERT(NULL == skyboxEntity);
		
		if(NULL == skyboxEntity &&
		   NULL != entity)
		{
			skyboxEntity = SafeRetain(entity);
			
			state = SYSTEM_INITIAL_STATE;
		}
	}
	
	void SkyboxSystem::RemoveEntity(Entity * entity)
	{
		DVASSERT(entity == skyboxEntity);
		
		if(entity == skyboxEntity)
		{
			SafeRelease(skyboxEntity);
		}
	}

	void SkyboxSystem::CreateSkybox()
	{
		RenderDataObject* renderDataObj = new RenderDataObject();
		
		skyboxMaterial = new Material();
		skyboxMaterial->SetType(Material::MATERIAL_SKYBOX);
		skyboxMaterial->SetAlphablend(false);
		skyboxMaterial->SetName("SkyBox_material");
		skyboxMaterial->GetRenderState()->SetDepthFunc(CMP_LEQUAL);
		skyboxMaterial->GetRenderState()->state |= RenderState::STATE_DEPTH_TEST;
		skyboxMaterial->GetRenderState()->state &= ~RenderState::STATE_DEPTH_WRITE;
		
		skyboxRenderBatch = new SkyBoxRenderBatch();
		skyboxRenderBatch->SetRenderDataObject(renderDataObj);
		skyboxRenderBatch->SetMaterial(skyboxMaterial);
		SafeRelease(renderDataObj);
		
		RenderObject* renderObj = new RenderObject();
		renderObj->AddRenderBatch(skyboxRenderBatch);
		
		RenderComponent* renderComponent =
			static_cast<RenderComponent*>(skyboxEntity->GetComponent(Component::RENDER_COMPONENT));
		
		if(NULL != renderComponent)
		{
			skyboxEntity->RemoveComponent(Component::RENDER_COMPONENT);
		}

		renderComponent = new RenderComponent();
		renderComponent->SetRenderObject(renderObj);
		skyboxEntity->AddComponent(renderComponent);

		SafeRelease(renderObj);
	}
	
	void SkyboxSystem::UpdateSkybox()
	{
		SkyboxComponent* skyboxComponent =
			static_cast<SkyboxComponent*>(skyboxEntity->GetComponent(Component::SKYBOX_COMPONENT));
		
		skyboxMaterial->SetTexture(Material::TEXTURE_DIFFUSE,
								   DAVA::Texture::CreateFromFile(skyboxComponent->GetTexture()));
		
		Vector3 boxSize = skyboxComponent->GetBoxSize();
		AABBox3 box(Vector3(-0.5 * boxSize.x, -0.5 * boxSize.y, -0.5 * boxSize.z),
					Vector3(0.5 * boxSize.x, 0.5 * boxSize.y, 0.5 * boxSize.z));
		
		skyboxRenderBatch->SetBox(box);
		skyboxRenderBatch->SetVerticalOffset(skyboxComponent->GetVerticalOffset());
		skyboxRenderBatch->SetRotation(skyboxComponent->GetRotationAngle());
	}
	
	void SkyboxSystem::SetSystemStateDirty()
	{
		state = SYSTEM_DIRTY;
	}
	
	bool SkyboxSystem::IsSkyboxPresent()
	{
		return (skyboxEntity != NULL);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////
	
	SkyboxSystem::SkyBoxRenderBatch::SkyBoxRenderBatch()
	:
	positionStream(NULL),
	texCoordStream(NULL),
	nonClippingDistance(0.0f),
	zOffset(0.0f),
	rotation(0.0f)
	{
		SetOwnerLayerName(LAYER_AFTER_OPAQUE);
	}
	
	SkyboxSystem::SkyBoxRenderBatch::~SkyBoxRenderBatch()
	{
		//don't delete streams - they are part of renderdataobject and will be free'd up with it
		positionStream = NULL;
		texCoordStream = NULL;
	}
	
	void SkyboxSystem::SkyBoxRenderBatch::SetVerticalOffset(float32 verticalOffset)
	{
		zOffset = verticalOffset;
	}
	
	void SkyboxSystem::SkyBoxRenderBatch::SetRotation(float32 angle)
	{
		rotation = DAVA::DegToRad(angle);
	}
	
	void SkyboxSystem::SkyBoxRenderBatch::SetBox(const AABBox3& box)
	{
		RenderDataObject* renderDataObj = GetRenderDataObject();
		
		if(NULL == positionStream)
		{
			positionStream = renderDataObj->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, NULL);
		}
		
		if(NULL == texCoordStream)
		{
			texCoordStream = renderDataObj->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 3, 0, NULL);
		}
		
		static Vector3 cubeVertices[SKYBOX_VERTEX_COUNT];
		static Vector3 cubeTexCoords[SKYBOX_VERTEX_COUNT] = {
			Vector3(-1, -1, -1), Vector3(-1, -1, 1), Vector3(1, -1, -1), Vector3(1, -1, -1), Vector3(-1, -1, 1), Vector3(1, -1, 1),
			Vector3(1, -1, -1), Vector3(1, -1, 1), Vector3(1, 1, -1), Vector3(1, 1, -1), Vector3(1, -1, 1), Vector3(1, 1, 1),
			Vector3(1, 1, -1), Vector3(1, 1, 1), Vector3(-1, 1, -1), Vector3(-1, 1, -1), Vector3(1, 1, 1), Vector3(-1, 1, 1),
			Vector3(-1, 1, -1), Vector3(-1, 1, 1), Vector3(-1, -1, -1), Vector3(-1, -1, -1), Vector3(-1, 1, 1), Vector3(-1, -1, 1),
			Vector3(-1, -1, 1), Vector3(-1, 1, 1), Vector3(1, -1, 1), Vector3(1, -1, 1), Vector3(-1, 1, 1), Vector3(1, 1, 1)
		};
		
		//face 0
		
		cubeVertices[0].x = box.min.x;
		cubeVertices[0].y = box.min.y;
		cubeVertices[0].z = box.min.z;
		
		cubeVertices[1].x = box.min.x;
		cubeVertices[1].y = box.min.y;
		cubeVertices[1].z = box.max.z;
		
		cubeVertices[2].x = box.max.x;
		cubeVertices[2].y = box.min.y;
		cubeVertices[2].z = box.min.z;
		
		cubeVertices[3].x = box.max.x;
		cubeVertices[3].y = box.min.y;
		cubeVertices[3].z = box.min.z;
		
		cubeVertices[4].x = box.min.x;
		cubeVertices[4].y = box.min.y;
		cubeVertices[4].z = box.max.z;
		
		cubeVertices[5].x = box.max.x;
		cubeVertices[5].y = box.min.y;
		cubeVertices[5].z = box.max.z;
		
		//face 1
		
		cubeVertices[6].x = box.max.x;
		cubeVertices[6].y = box.min.y;
		cubeVertices[6].z = box.min.z;
		
		cubeVertices[7].x = box.max.x;
		cubeVertices[7].y = box.min.y;
		cubeVertices[7].z = box.max.z;
		
		cubeVertices[8].x = box.max.x;
		cubeVertices[8].y = box.max.y;
		cubeVertices[8].z = box.min.z;
		
		cubeVertices[9].x = box.max.x;
		cubeVertices[9].y = box.max.y;
		cubeVertices[9].z = box.min.z;
		
		cubeVertices[10].x = box.max.x;
		cubeVertices[10].y = box.min.y;
		cubeVertices[10].z = box.max.z;
		
		cubeVertices[11].x = box.max.x;
		cubeVertices[11].y = box.max.y;
		cubeVertices[11].z = box.max.z;
		
		
		//face 2
		
		cubeVertices[12].x = box.max.x;
		cubeVertices[12].y = box.max.y;
		cubeVertices[12].z = box.min.z;
		
		cubeVertices[13].x = box.max.x;
		cubeVertices[13].y = box.max.y;
		cubeVertices[13].z = box.max.z;
		
		cubeVertices[14].x = box.min.x;
		cubeVertices[14].y = box.max.y;
		cubeVertices[14].z = box.min.z;
		
		cubeVertices[15].x = box.min.x;
		cubeVertices[15].y = box.max.y;
		cubeVertices[15].z = box.min.z;
		
		cubeVertices[16].x = box.max.x;
		cubeVertices[16].y = box.max.y;
		cubeVertices[16].z = box.max.z;
		
		cubeVertices[17].x = box.min.x;
		cubeVertices[17].y = box.max.y;
		cubeVertices[17].z = box.max.z;
		
		//face 3
		
		cubeVertices[18].x = box.min.x;
		cubeVertices[18].y = box.max.y;
		cubeVertices[18].z = box.min.z;
		
		cubeVertices[19].x = box.min.x;
		cubeVertices[19].y = box.max.y;
		cubeVertices[19].z = box.max.z;
		
		cubeVertices[20].x = box.min.x;
		cubeVertices[20].y = box.min.y;
		cubeVertices[20].z = box.min.z;
		
		cubeVertices[21].x = box.min.x;
		cubeVertices[21].y = box.min.y;
		cubeVertices[21].z = box.min.z;
		
		cubeVertices[22].x = box.min.x;
		cubeVertices[22].y = box.max.y;
		cubeVertices[22].z = box.max.z;
		
		cubeVertices[23].x = box.min.x;
		cubeVertices[23].y = box.min.y;
		cubeVertices[23].z = box.max.z;
		
		//face 4 (top)
		
		cubeVertices[24].x = box.min.x;
		cubeVertices[24].y = box.min.y;
		cubeVertices[24].z = box.max.z;
		
		cubeVertices[25].x = box.min.x;
		cubeVertices[25].y = box.max.y;
		cubeVertices[25].z = box.max.z;
		
		cubeVertices[26].x = box.max.x;
		cubeVertices[26].y = box.min.y;
		cubeVertices[26].z = box.max.z;
		
		cubeVertices[27].x = box.max.x;
		cubeVertices[27].y = box.min.y;
		cubeVertices[27].z = box.max.z;
		
		cubeVertices[28].x = box.min.x;
		cubeVertices[28].y = box.max.y;
		cubeVertices[28].z = box.max.z;
		
		cubeVertices[29].x = box.max.x;
		cubeVertices[29].y = box.max.y;
		cubeVertices[29].z = box.max.z;
		
		//could be any pair of opposite diagonal vertices
		Vector3 maxDistanceBetweenVertices = cubeVertices[29] - cubeVertices[24];
		nonClippingDistance = 0.5f * maxDistanceBetweenVertices.Length();
		
		positionStream->Set(TYPE_FLOAT, 3, 0, cubeVertices);
		texCoordStream->Set(TYPE_FLOAT, 3, 0, cubeTexCoords);
	}
	
	void SkyboxSystem::SkyBoxRenderBatch::Draw(DAVA::Camera * camera)
	{
		if(!renderObject || !renderDataObject) return;
		
		//scale cube so it's not get clipped by zNear plane
		float32 zNear = camera->GetZNear();
		float32 scale = (nonClippingDistance + zNear) / nonClippingDistance;
		
		Vector3 camPos = camera->GetPosition();
		
		camPos.z += zOffset;
		
		Matrix4 finalMatrix = Matrix4::MakeRotation(Vector3(0.0f, 0.0f, 1.0f), rotation) *
								Matrix4::MakeScale(Vector3(scale, scale, scale)) *
								Matrix4::MakeTranslation(camPos) *
								camera->GetMatrix();
		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);
		
		RenderManager::Instance()->SetRenderData(renderDataObject);
		material->PrepareRenderState();
		
		RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLELIST, 0, SKYBOX_VERTEX_COUNT);
	}
};