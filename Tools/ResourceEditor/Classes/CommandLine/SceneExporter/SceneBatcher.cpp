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


#include "SceneBatcher.h"
#include "BatchEntitiesHelper.h"

namespace DAVA {

// Scene Batcher Settings.
// Max Texture Size for batched texture.
static const int32 SCENE_BATCHER_MAX_BATCHED_TEXTURE_SIZE = 2048;

// Whether to use only Square Textures for batched one?
static const bool SCENE_BATCHER_USE_ONLY_SQUARE_TEXTURES = true;
	
// Whether to output all error messages to the log?
static const bool SCENE_BATCHER_OUTPUT_DEBUG_INFO_TO_LOG = true;
	
SceneBatcher::TexturesAndMaterialData::TexturesAndMaterialData()
{
	isSucceeded = false;
	isTexturesBatchingPerformed = false;
	batchedMaterial = NULL;
}

void SceneBatcher::BatchSceneNodes(Scene *scene, Set<String> &errorLog)
{
	// First pass - lookup for the batch IDs to be batched.
	Set<int32> batchIDs = BuildBatchIndicesList(scene);
	if (batchIDs.size() == 0)
	{
		// Nothing to batch.
		return;
	}
	
	for (Set<int32>::iterator iter = batchIDs.begin(); iter != batchIDs.end(); iter ++)
	{
		int32 batchID = *iter;
		Set<Entity*> entitiesToBatch = GetEntitiesForBatchIndex(scene, batchID);
		for (Set<Entity*>::iterator iter = entitiesToBatch.begin(); iter != entitiesToBatch.end(); iter ++)
		{
			Logger::Debug("Entity to batch: %s", (*iter)->GetName().c_str());
		}
		
		if (entitiesToBatch.size() == 0)
		{
			continue;
		}
		
		Map<Entity*, Entity*> geometryEntitiesToBatch = SelectGeometryEntities(entitiesToBatch);
		
		// A new Material is created for the batched entity - it might contain new batched texture.
		TexturesAndMaterialData batchedTMData =
			BatchTexturesAndPrepareMaterial(geometryEntitiesToBatch, batchID, errorLog);
		if (!batchedTMData.isSucceeded)
		{
			AddToErrorLog(errorLog, Format("Unable to batch Textures and prepare Material for batch ID %i - skipping", batchID));
			continue;
		}

		// For batching only the nodes contains geometry is added. For LOD nodes -
		// need to take only the first LOD level.
		// TODO! handle LODs separately!!!
		// TODO! handle LODs separately!!!
		// TODO! handle LODs separately!!!
		String batchName = GetNextBatchedEntityName(scene);
		Entity* resultEntity = BatchEntities(scene, errorLog, geometryEntitiesToBatch, batchName, batchedTMData);
		
		if (!resultEntity)
		{
			continue;
		}
		
		// Add the new batched entity to the parent of the current root entity, if any.
		Entity* rootEntity = geometryEntitiesToBatch.begin()->first;
		Entity* parentEntity = rootEntity->GetParent();
		if (!parentEntity)
		{
			AddToErrorLog(errorLog, Format("Batching: no Parent Entity for Root Entity %s", rootEntity->GetName().c_str()));
			continue;
		}
		
		parentEntity->AddNode(resultEntity);
		DeleteEntities(geometryEntitiesToBatch);
	}
}

Set<int32> SceneBatcher::BuildBatchIndicesList(Scene* scene)
{
	Set<int32> resultSet;
	if (scene)
	{
		BuildBatchIndicesListRecursive(scene, resultSet);
	}
	
	return resultSet;
}

void SceneBatcher::BuildBatchIndicesListRecursive(Entity* rootEntity, Set<int32>& resultSet)
{
	if (!rootEntity)
	{
		return;
	}
	
	int32 batchIndex = BatchEntitiesHelper::GetBatchIndex(rootEntity);
	if (batchIndex != BATCH_INDEX_DEFAULT_VALUE)
	{
		resultSet.insert(batchIndex);
	}
	
	int32 childrenCount = rootEntity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		BuildBatchIndicesListRecursive(rootEntity->GetChild(i), resultSet);
	}
}

Set<Entity*> SceneBatcher::GetEntitiesForBatchIndex(Scene* scene, int32 batchIndex)
{
	Set<Entity*> resultSet;
	
	// Lookup through the whole scene.
	if (scene)
	{
		GetEntitiesForBatchIndexRecursive(scene, batchIndex, resultSet);
	}
	
	return resultSet;
}

void SceneBatcher::GetEntitiesForBatchIndexRecursive(Entity* rootEntity, int32 batchIndex,
													  Set<Entity*>& resultSet)
{
	if (!rootEntity)
	{
		return;
	}
	
	if (BatchEntitiesHelper::GetBatchIndex(rootEntity) == batchIndex)
	{
		resultSet.insert(rootEntity);
	}
	
	int32 childrenCount = rootEntity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		GetEntitiesForBatchIndexRecursive(rootEntity->GetChild(i), batchIndex, resultSet);
	}
}

Entity* SceneBatcher::BatchEntities(DAVA::Scene* scene, Set<String>& errorLog,
									const Map<Entity*, Entity*>& entitiesToBatch,
									const String& batchedEntityName,
									const TexturesAndMaterialData& batchedTMData)
{
	int childrenCount = entitiesToBatch.size();
	if (childrenCount == 0)
	{
		// Nothing to batch.
		return NULL;
	}
	
	Entity* firstEntity = entitiesToBatch.begin()->second;
	if (!firstEntity)
	{
		AddToErrorLog(errorLog, "Batching: First Entity in Batch is NULL");
		return NULL;
	}
	
	RenderComponent * firstEntityComponent = static_cast<RenderComponent*>(firstEntity->GetComponent(Component::RENDER_COMPONENT));
	if (!firstEntityComponent || !firstEntityComponent->GetRenderObject())
	{
		AddToErrorLog(errorLog, Format("Batching: First Entity %s does'nt have Render Object", firstEntity->GetName().c_str()));
		return NULL;
	}
	
	if (firstEntityComponent->GetRenderObject()->GetRenderBatchCount() != 1)
	{
		AddToErrorLog(errorLog,
					  Format("Batching: First Entity Component for entity %s has %i Render Batches, expected 1",
							 firstEntity->GetName().c_str(),
							 firstEntityComponent->GetRenderObject()->GetRenderBatchCount()));
		return NULL;
	}
	
	RenderBatch* firstEntityRenderBatch = firstEntityComponent->GetRenderObject()->GetRenderBatch(0);
	
	Entity* parentEntity = firstEntity->GetParent();
	if (!parentEntity)
	{
		AddToErrorLog(errorLog, Format("Batching: Parent for First Entity %s is NULL", firstEntity->GetName().c_str()));
		return NULL;
	}
	
	Entity* batchedEntity = new Entity();
	batchedEntity->SetName(batchedEntityName);
	
	// Precalculate the sizes.
	uint32 vertexCount = 0;
	uint32 indexCount = 0;
	AABBox3 batchedBoundingBox;
	CalculateBatchedEntityParameters(entitiesToBatch, vertexCount, indexCount, batchedBoundingBox, errorLog);
	
	if (vertexCount == 0 || indexCount == 0)
	{
		AddToErrorLog(errorLog, Format("Batching: Vertex/Index count for the Batched Entity %s is 0",
							   firstEntity->GetName().c_str()));
		return NULL;
	}
	
	Vector3 batchedEntityCenter = batchedBoundingBox.GetCenter();
	
	// Create the mesh for the new entity.
	Mesh* batchedMesh = new Mesh();
	PolygonGroup* batchedPolygonGroup = new PolygonGroup();
	uint32 meshFormat = firstEntityRenderBatch->GetPolygonGroup()->GetFormat();
	batchedPolygonGroup->AllocateData(meshFormat, vertexCount, indexCount);
	
	// Merge the streams themselves.
	uint32 verticesBatched = 0;
	uint32 indicesBatched = 0;
	
	for (Map<Entity*, Entity*>::const_iterator iter = entitiesToBatch.begin();
		 iter != entitiesToBatch.end(); iter ++)
	{
		// Batch the Vertices and Indices.
		Entity* curRootEntity = iter->first;
		Entity* curEntity = iter->second;
		
		// The Transformation Matrix has to be taken from the upper's level "solid" entity,
		// if it exists.
		Matrix4 curEntityMatrix = curRootEntity->GetLocalTransform();
		RenderComponent * component = static_cast<RenderComponent*>(curEntity->GetComponent(Component::RENDER_COMPONENT));
		
		if(!component || !component->GetRenderObject())
		{
			AddToErrorLog(errorLog, Format("Batching: Entity %s does'nt have Render Object", curEntity->GetName().c_str()));
			continue;
		}

		if (component->GetRenderObject()->GetType() != RenderObject::TYPE_MESH)
		{
			AddToErrorLog(errorLog, Format("Batching: Entity %s is not a Mesh", curEntity->GetName().c_str()));
			continue;
		}

		// Yuri Coder, 2013/06/11. Currently can merge only render objects with one render batch.
		Mesh* curMesh = static_cast<Mesh*>(component->GetRenderObject());
		uint32 curRenderBatchesCount = curMesh->GetRenderBatchCount();
		DVASSERT(curRenderBatchesCount == 1);
		
		Material* curMaterial = curMesh->GetRenderBatch(0)->GetMaterial();
		if (!curMaterial)
		{
			AddToErrorLog(errorLog, Format("Batching: Entity %s, RenderBatch 0 doesn't have an Material attached", curEntity->GetName().c_str()));
			continue;
		}
		
		Texture* curTexture = curMaterial->GetTexture(Material::TEXTURE_DIFFUSE);
		if (!curTexture)
		{
			AddToErrorLog(errorLog, Format("Batching: Entity %s, RenderBatch 0 doesn't have an TEXTURE_DIFFUSE", curEntity->GetName().c_str()));
			continue;
		}
		
		// Batch the polygon groups.
		PolygonGroup* curPolygonGroup = curMesh->GetPolygonGroup(0);
		InstanceMaterialState* curMaterialInstance = curMesh->GetRenderBatch(0)->GetMaterialInstance();

		Vector2 textureMultiply;
		Vector2 textureOffset;
		CalculateTextureTransform(textureMultiply, textureOffset, batchedTMData, curTexture);

		PolygonGroupBatchingData batchingData;
		batchingData.curPolygonGroup = curPolygonGroup;
		batchingData.batchedPolygonGroup = batchedPolygonGroup;
		batchingData.meshFormat = meshFormat;
		batchingData.verticesBatched = verticesBatched;
		batchingData.curEntityMatrix = curEntityMatrix;
		batchingData.batchedEntityCenter = batchedEntityCenter;
		batchingData.curMaterialInstance = curMaterialInstance;
		batchingData.textureMultiply = textureMultiply;
		batchingData.textureOffset = textureOffset;
	
		uint32 verticesBatchedOnThisPass = MergePolygonGroups(batchingData);
		uint32 indicesBatchedOnThisPass = MergeIndices(batchedPolygonGroup, curPolygonGroup, verticesBatched,
													   indicesBatched);
		
		verticesBatched += verticesBatchedOnThisPass;
		indicesBatched += indicesBatchedOnThisPass;
	}
	
	// Done batching.
	DVASSERT(verticesBatched == vertexCount);
	DVASSERT(indicesBatched == indexCount);
	batchedPolygonGroup->BuildBuffers();
	
	batchedMesh->AddPolygonGroup(batchedPolygonGroup, batchedTMData.batchedMaterial);
	
	// Update the material instance for the batched mesh.
	RenderBatch* batchedRenderBatch = batchedMesh->GetRenderBatch(0);
	InstanceMaterialState* batchedMaterialInstance = batchedRenderBatch->GetMaterialInstance();
	
	// Lightmap Texture is common through the whole scene.
	const FilePath& lightMapTextureName = firstEntityRenderBatch->GetMaterialInstance()->GetLightmapName();
	batchedMaterialInstance->SetLightmap(DAVA::Texture::CreateFromFile(lightMapTextureName), lightMapTextureName);
	batchedMaterialInstance->SetUVOffsetScale(Vector2(1.0f, 1.0f), Vector2(1.0f, 1.0f));
	
	RenderComponent* batchedRenderComponent = new RenderComponent();
	batchedRenderComponent->SetRenderObject(batchedMesh);
	batchedEntity->AddComponent(batchedRenderComponent);
	
	// Move the entity back to its correct center.
	Matrix4 localBatchedMatrix = batchedEntity->GetLocalTransform();
	Matrix4 moveModification;
	
	moveModification.CreateTranslation(batchedEntityCenter);
	
	Matrix4 positionedBatchedMatrix = batchedEntity->GetLocalTransform() * moveModification;
	batchedEntity->SetLocalTransform(positionedBatchedMatrix);
	
	return batchedEntity;
}

void SceneBatcher::CalculateBatchedEntityParameters(const Map<Entity*, Entity*>& entitiesToBatch,
													uint32& vertexCount, uint32& indexCount,
													AABBox3& batchedBoundingBox,
													Set<String>& errorLog)
{
	for (Map<Entity*, Entity*>::const_iterator iter = entitiesToBatch.begin();
		 iter != entitiesToBatch.end(); iter ++)
	{
		Entity* curEntity = iter->second;
		RenderComponent * component = static_cast<RenderComponent*>(curEntity->GetComponent(Component::RENDER_COMPONENT));
		
		if(!component || !component->GetRenderObject())
		{
			AddToErrorLog(errorLog, Format("Entity %s does not have Render Component or Render Object", curEntity->GetName().c_str()));
			continue;
		}
		
		if (component->GetRenderObject()->GetType() != RenderObject::TYPE_MESH)
		{
			AddToErrorLog(errorLog, Format("Entity %s type is not a Mesh", curEntity->GetName().c_str()));
			continue;
		}

		Mesh* curMesh = static_cast<Mesh*>(component->GetRenderObject());
		uint32 curRenderBatchesCount = curMesh->GetRenderBatchCount();
		DVASSERT(curRenderBatchesCount == 1);
		
		RenderBatch* curRenderBatch = curMesh->GetRenderBatch(0);
		vertexCount += curRenderBatch->GetPolygonGroup()->GetVertexCount();
		indexCount += curRenderBatch->GetPolygonGroup()->GetIndexCount();
		
		// Bounding Box is to be calculated in world coordinates.
		AABBox3 worldBoundingBox;
		curRenderBatch->GetBoundingBox().GetTransformedBox(curEntity->GetWorldTransform(), worldBoundingBox);
		batchedBoundingBox.AddAABBox(worldBoundingBox);
	}
}

uint32 SceneBatcher::MergePolygonGroups(const PolygonGroupBatchingData& data)
{
	if (!data.batchedPolygonGroup || !data.curPolygonGroup)
	{
		DVASSERT(false);
		return 0;
	}
	
	// The translation matrix is needed to move the bounding box to the (0,0,0) coord.
	// The resulting entity will then be moved back to the original position.
	Matrix4 translateMatrix;
	translateMatrix.CreateTranslation(-data.batchedEntityCenter);
	
	uint32 verticesToBatch = data.curPolygonGroup->GetVertexCount();
	Vector3 vector3Param;
	Vector2 vector2Param;
	for (uint32 i = 0; i < verticesToBatch; i ++)
	{
		uint32 positionInBatch = i + data.verticesBatched;
		if (data.meshFormat & EVF_VERTEX && data.curPolygonGroup->vertexArray)
		{
			data.curPolygonGroup->GetCoord(i, vector3Param);
			Vector3 newCoordParam = vector3Param * data.curEntityMatrix * translateMatrix;
			data.batchedPolygonGroup->SetCoord(positionInBatch, newCoordParam);
		}
		
		if (data.meshFormat & EVF_NORMAL && data.curPolygonGroup->normalArray)
		{
			data.curPolygonGroup->GetNormal(i, vector3Param);
			data.batchedPolygonGroup->SetNormal(positionInBatch, vector3Param);
		}
		
		if (data.meshFormat & EVF_TANGENT && data.curPolygonGroup->tangentArray)
		{
			data.curPolygonGroup->GetTangent(i, vector3Param);
			data.batchedPolygonGroup->SetTangent(positionInBatch, vector3Param);
		}
		
		if (data.meshFormat & EVF_TEXCOORD0 && data.curPolygonGroup->textureCoordCount > 0)
		{
			data.curPolygonGroup->GetTexcoord(0, i, vector2Param);
			vector2Param.x = vector2Param.x * data.textureMultiply.x + data.textureOffset.x;
			vector2Param.y = vector2Param.y * data.textureMultiply.y + data.textureOffset.y;
			data.batchedPolygonGroup->SetTexcoord(0, positionInBatch, vector2Param);
		}
		
		// In case we have Material Instance from the current entity - recalculate
		// the TextCoord1 (LightMap) positions.
		if (data.meshFormat & EVF_TEXCOORD1 && data.curPolygonGroup->textureCoordCount > 1)
		{
			data.curPolygonGroup->GetTexcoord(1, i, vector2Param);
			if (data.curMaterialInstance)
			{
				vector2Param = Vector2(vector2Param.x * data.curMaterialInstance->GetUVScale().x,
									   vector2Param.y * data.curMaterialInstance->GetUVScale().y);
				vector2Param += data.curMaterialInstance->GetUVOffset();
			}
			
			data.batchedPolygonGroup->SetTexcoord(1, positionInBatch, vector2Param);
		}
	}
	
	return verticesToBatch;
}

uint32 SceneBatcher::MergeIndices(PolygonGroup* batchedPolygonGroup, PolygonGroup* curPolygonGroup,
								   uint32 verticesBatched, uint32 indicesBatched)
{
	if (!batchedPolygonGroup || !curPolygonGroup)
	{
		DVASSERT(false);
		return 0;
	}
	
	int32 indicesToBatch = curPolygonGroup->GetIndexCount();
	for (int32 i = 0; i < indicesToBatch; i ++)
	{
		int32 index = 0;
		curPolygonGroup->GetIndex(i, index);
		batchedPolygonGroup->SetIndex(i + indicesBatched, index + verticesBatched);
	}
	
	return indicesToBatch;
}

void SceneBatcher::DeleteEntities(const Map<Entity*, Entity*>& entitiesToDelete)
{
	for (Map<Entity*, Entity*>::const_iterator iter = entitiesToDelete.begin(); iter != entitiesToDelete.end();
		 iter ++)
	{
		// Delete the root-level entities only.
		Entity* rootEntityToDelete = iter->first;
		if (!rootEntityToDelete)
		{
			continue;
		}
		Entity* parentNode = rootEntityToDelete->GetParent();
		DVASSERT(parentNode);
		
		parentNode->Retain();
		parentNode->RemoveNode(rootEntityToDelete);
		parentNode->Release();
	}
}

String SceneBatcher::GetNextBatchedEntityName(Scene* scene)
{
	Set<String> usedNames;
	BuildEntityNamesList(scene, usedNames);
	
	// Lookup through the all scene to find the "free" batched entity names.
	int32 nextBatchedEntityIndex = 0;
	while (nextBatchedEntityIndex < 9999)
	{
		String curEntityName = Format("BatchedEntity%i", nextBatchedEntityIndex);
		if (usedNames.find(curEntityName) == usedNames.end())
		{
			return curEntityName;
		}
		
		nextBatchedEntityIndex ++;
	}
	
	return "BatchedEntity";
}

void SceneBatcher::BuildEntityNamesList(Entity* rootEntity, Set<String>& usedNames)
{
	if (!rootEntity)
	{
		return;
	}
	
	usedNames.insert(rootEntity->GetName());
	
	int32 childrenCount = rootEntity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		BuildEntityNamesList(rootEntity->GetChild(i), usedNames);
	}
}

Map<Entity*, Entity*> SceneBatcher::SelectGeometryEntities(Set<Entity*>& entitiesToBatch)
{
	Map<Entity*, Entity*> resultMap;
	for (Set<Entity*>::iterator iter = entitiesToBatch.begin(); iter != entitiesToBatch.end();
		 iter ++)
	{
		Entity* rootLevelEntity = (*iter);
		SelectGeometryEntitiesRecursive(rootLevelEntity, rootLevelEntity, resultMap);
	}

	return resultMap;
}

void SceneBatcher::SelectGeometryEntitiesRecursive(Entity* rootLevelEntity, Entity* curLevelEntity,
												   Map<Entity*, Entity*>& resultMap)
{
	LodComponent* lodComponent = GetLodComponent(curLevelEntity);
	RenderObject* renderObject = GetRenerObject(curLevelEntity);
	if (renderObject)
	{
		// This entity is a geometry one - add as is.
		Logger::Debug("Adding render component %s, root %s",
					  curLevelEntity->GetName().c_str(),
					  rootLevelEntity->GetName().c_str());
		resultMap[rootLevelEntity] = curLevelEntity;
	}
	else if (lodComponent)
	{
		// For LOD entities we have to add all the LOD 0 entities.
		Vector<LodComponent::LodData*> lodLayers;
		lodComponent->GetLodData(lodLayers);
		if (lodLayers.size() > 0)
		{
			for (int32 i = 0; i < lodLayers[0]->nodes.size(); i ++)
			{
				Entity* lodEntity = lodLayers[0]->nodes[i];
				if (!lodEntity)
				{
					continue;
				}
				
				// TODO: Yuri Coder, 2013/06/18. Return to LOD batching later!
				Logger::Debug("Adding render component %s from LOD, root %s",
							  lodEntity->GetName().c_str(),
							  curLevelEntity->GetName().c_str());
				resultMap[rootLevelEntity] = lodEntity;
			}
		}

		// LOD level is the deepest one - no need to go further.
		return;
	}

	// Verify for the children.
	int32 childrenCount = curLevelEntity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		Entity* childEntity = curLevelEntity->GetChild(i);
		SelectGeometryEntitiesRecursive(rootLevelEntity, childEntity, resultMap);
	}
}

SceneBatcher::TexturesAndMaterialData SceneBatcher::BatchTexturesAndPrepareMaterial(const Map<Entity*, Entity*> entitiesToBatch,
																					int32 batchID, Set<String> &errorLog)
{
	TexturesAndMaterialData resultData;
	if (entitiesToBatch.size() < 2)
	{
		// Nothing to batch.
		AddToErrorLog(errorLog, Format("Amount of entities to batch for Batch ID %i is %i - minimum two entities required",
									   batchID, entitiesToBatch.size()));
		return resultData;
	}

	Entity* firstEntity = entitiesToBatch.begin()->second;
	RenderBatch* firstRenderBatch = GetFirstRenderBatch(firstEntity);
	if (!firstEntity || !firstRenderBatch)
	{
		AddToErrorLog(errorLog, Format("First Entity or First Render Batch does not exist for Batch ID %i",
							   batchID));
		return resultData;
	}
	
	Material* firstMaterial = firstRenderBatch->GetMaterial();
	if (!firstMaterial)
	{
		AddToErrorLog(errorLog, Format("First Entity %s for Batch ID %i does not have Material assigned",
							   firstEntity->GetName().c_str(), batchID));
		return resultData;
	}
	
	// First pass - determine whether the material can be batched, and determine the list
	// of textures to batch, if any.
	List<Texture*> texturesToBatch;
	texturesToBatch.push_back(firstMaterial->GetTexture(Material::TEXTURE_DIFFUSE));

	Texture* firstLightMapTexture = firstRenderBatch->GetMaterialInstance()->GetLightmap();

	bool textureBatchingNeeded = false;
	bool textureBatchingCanBeDone = true;
	for (Map<Entity*, Entity*>::const_iterator iter = entitiesToBatch.begin(); iter != entitiesToBatch.end();
		 iter ++)
	{
		Entity* curEntity = iter->second;
		RenderBatch* curRenderBatch = GetFirstRenderBatch(curEntity);
		
		if (!curEntity || !curRenderBatch)
		{
			continue;
		}
		
		Material* curMaterial = curRenderBatch->GetMaterial();

		// Verify the lightmap texture.
		Texture* curLightmapTexture = curRenderBatch->GetMaterialInstance()->GetLightmap();
		if ((curLightmapTexture && !firstLightMapTexture) ||
			(!curLightmapTexture && firstLightMapTexture) ||
			(curLightmapTexture != firstLightMapTexture))
		{
			AddToErrorLog(errorLog, Format("Can't batch Batch ID %i - Lightmaps assigned to batch components are different",
										   batchID));
			textureBatchingCanBeDone = false;
			break;
		}

		Material::eMaterialComparisonResult compareResult = firstMaterial->Compare(curMaterial, true);
		if (compareResult == Material::MATERIALS_DIFFERENT)
		{
			// Can't batch.
			textureBatchingCanBeDone = false;
			break;
		}
		else if (compareResult == Material::MATERIALS_DIFFER_IN_TEXTURES_ONLY)
		{
			// Remember the texture we have to batch.
			textureBatchingNeeded = true;
			texturesToBatch.push_back(curMaterial->GetTexture(Material::TEXTURE_DIFFUSE));
		}
	}

	if (!textureBatchingCanBeDone)
	{
		// Cannot batch these textures at all - the materials are different.
		AddToErrorLog(errorLog, Format("Can't batch Batch ID %i - Materials assigned to the batch components are different",
							   batchID));
		return resultData;
	}

	if (!textureBatchingNeeded)
	{
		// No textures batching is needed at all - the materials are identical. Not an error.
		resultData.isSucceeded = true;
		resultData.isTexturesBatchingPerformed = false;
		resultData.batchedMaterial = firstMaterial->Clone();
		return resultData;
	}
	
	// Batch the textures themselves.
	TexturePacker texturePacker;
	texturePacker.SetMaxTextureSize(SCENE_BATCHER_MAX_BATCHED_TEXTURE_SIZE);
	if (SCENE_BATCHER_USE_ONLY_SQUARE_TEXTURES)
	{
		texturePacker.UseOnlySquareTextures();
	}

	
	resultData.batchTexturesResult = texturePacker.Batch(texturesToBatch, batchID);
	if (!resultData.batchTexturesResult.errorCode != TexturePacker::SUCCESS)
	{
		// Cannot batch these textures at all - the materials are different.
		AddToErrorLog(errorLog, Format("Can't batch Batch ID %i - Texture Packer returned %i",
									   batchID, resultData.batchTexturesResult.errorCode));
		return resultData;
	}

	// Build the new Material based on the Batch Result.
	resultData.isSucceeded = true;
	resultData.isTexturesBatchingPerformed = true;
	resultData.batchedMaterial = firstMaterial->Clone();
	resultData.batchedMaterial->SetTexture(Material::TEXTURE_DIFFUSE, resultData.batchTexturesResult.batchedTexturePath);
	
	return resultData;
}

void SceneBatcher::CalculateTextureTransform(Vector2 &textureMultiply, Vector2& textureOffset, const TexturesAndMaterialData& batchedTMData, const Texture* curTexture)
{
	textureMultiply.Set(1.0f, 1.0f);
	textureOffset.Set(0.0f, 0.0f);

	if (!batchedTMData.isTexturesBatchingPerformed)
	{
		// Textures were not batched, so no transform needed.
		return;
	}

	// Multiplication is calculated according to curTexture.Size / batchedTexture.Size.
	textureMultiply.x = (float32)curTexture->GetWidth() / (float32)batchedTMData.batchTexturesResult.batchedTextureWidth;
	textureMultiply.y = (float32)curTexture->GetHeight() / (float32)batchedTMData.batchTexturesResult.batchedTextureHeight;

	Map<FilePath, TexturePacker::BatchTexturesOutputData>::const_iterator iter =
		batchedTMData.batchTexturesResult.outputData.find(curTexture->GetPathname());
	if (iter != batchedTMData.batchTexturesResult.outputData.end())
	{
		const TexturePacker::BatchTexturesOutputData& curData = iter->second;
		textureOffset.x = (float32)curData.offsetX / (float32)batchedTMData.batchTexturesResult.batchedTextureWidth;
		textureOffset.y = (float32)curData.offsetY / (float32)batchedTMData.batchTexturesResult.batchedTextureHeight;
	}
}

RenderBatch* SceneBatcher::GetFirstRenderBatch(Entity* entity)
{
	if (!entity)
	{
		return NULL;
	}
	
	RenderComponent* renderComponent = static_cast<RenderComponent*>(entity->GetComponent(Component::RENDER_COMPONENT));
	if (!renderComponent || !renderComponent->GetRenderObject())
	{
		return NULL;
	}
	
	if (renderComponent->GetRenderObject()->GetRenderBatchCount() != 1)
	{
		return NULL;
	}
	
	return renderComponent->GetRenderObject()->GetRenderBatch(0);
}

void SceneBatcher::AddToErrorLog(Set<String>& errorLog, const String& message)
{
	errorLog.insert(message);
	if (SCENE_BATCHER_OUTPUT_DEBUG_INFO_TO_LOG)
	{
		Logger::Error(message.c_str());
	}
}

};