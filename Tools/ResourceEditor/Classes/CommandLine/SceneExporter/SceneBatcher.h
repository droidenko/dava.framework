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

#ifndef __SCENE_BATCHER_H__
#define __SCENE_BATCHER_H__

#include "DAVAEngine.h"
#include "TexturePacker/TexturePacker.h"

namespace DAVA {

class SceneBatcher
{
public:
	// Batch the scene nodes.
	void BatchSceneNodes(Scene *scene, Set<String> &errorLog);
	
protected:

	// Textures & Material Batching Data.
	struct TexturesAndMaterialData
	{
		TexturesAndMaterialData();

		bool isSucceeded;
		bool isTexturesBatchingPerformed;
		Material* batchedMaterial;
		TexturePacker::BatchTexturesResult batchTexturesResult;
	};

	// Polygon Groups Batching Data
	struct PolygonGroupBatchingData
	{
		PolygonGroup* curPolygonGroup;
		PolygonGroup* batchedPolygonGroup;
		uint32 meshFormat;
		uint32 verticesBatched;
		Matrix4 curEntityMatrix;
		Vector3 batchedEntityCenter;
		InstanceMaterialState* curMaterialInstance;
		Vector2 textureMultiply;
		Vector2 textureOffset;
	};

	// Build the list of unique Batch Indices in the scene.
	Set<int32> BuildBatchIndicesList(Scene* scene);
	void BuildBatchIndicesListRecursive(Entity* rootEntity, Set<int32>& resultSet);
	
    // Get all the entities with the same Batch Index.
	Set<Entity*> GetEntitiesForBatchIndex(Scene* scene, int32 batchIndex);
	void GetEntitiesForBatchIndexRecursive(Entity* rootEntity, int32 batchIndex, Set<Entity*>& resultSet);
	
	// Select only the entities contain geometry from the list of entities to batch.
	Map<Entity*, Entity*> SelectGeometryEntities(Set<Entity*>& entitiesToBatch);
	void SelectGeometryEntitiesRecursive(Entity* rootLevelEntity, Entity* curLevelEntity, Map<Entity*, Entity*>& resultMap);

	// Perform the actual batch (vertices, indexes, textures).
	Entity* BatchEntities(Scene* scene, Set<String>& errorLog,
						  const Map<Entity*, Entity*>& entitiesToBatch,
						  const String& batchedEntityName,
						  const TexturesAndMaterialData& batchedTMData);
	
	// Calculate the paremeters for the batched entity.
	void CalculateBatchedEntityParameters(const Map<Entity*, Entity*>& entitiesToBatch,
										  uint32& vertexCount, uint32& indexCount,
										  AABBox3& batchedBoundingBox,
										  Set<String>& errorLog);
	
	// Merge the Polygon Groups.
	uint32 MergePolygonGroups(const PolygonGroupBatchingData& data);

	// Merge the Indices.
	uint32 MergeIndices(PolygonGroup* batchedPolygonGroup, PolygonGroup* curPolygonGroup,
						uint32 verticesBatched, uint32 indicesBatched);
	
	// Delete the entities.
	void DeleteEntities(const Map<Entity*, Entity*>& entitiesToDelete);
	
	// Get the index for the next entity.
	String GetNextBatchedEntityName(Scene* scene);
	void BuildEntityNamesList(Entity* rootEntity, Set<String>& usedNames);
	
	// Textures batching.
	TexturesAndMaterialData BatchTexturesAndPrepareMaterial(const Map<Entity*, Entity*> entitiesToBatch,
															int32 batchID, Set<String> &errorLog);

	// Calculate transformation (offset/multiplication) for the batched texture.
	void CalculateTextureTransform(Vector2 &textureMultiply, Vector2& textureOffset, const TexturesAndMaterialData& batchedTMData, const Texture* curTexture);

	// Get the first render batch for the entity.
	RenderBatch* GetFirstRenderBatch(Entity* entity);
	
	// Add the error message to log and to console if needed.
	void AddToErrorLog(Set<String>& errorLog, const String& message);
};

}

#endif /* defined(__SCENE_BATCHER_H__) */
