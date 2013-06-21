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

#ifndef __SCENE_EXPORTER_H__
#define __SCENE_EXPORTER_H__

#include "DAVAEngine.h"
#include "CommandLine/SceneUtils/SceneUtils.h"
#include "TexturePacker/TexturePacker.h"

using namespace DAVA;

class SceneExporter
{
public:

	SceneExporter();
	virtual ~SceneExporter();
    
    void SetGPUForExporting(const String &newGPU);
    void SetGPUForExporting(const eGPUFamily newGPU);
    
    void CleanFolder(const FilePath &folderPathname, Set<String> &errorLog);
    
    void SetInFolder(const FilePath &folderPathname);
    void SetOutFolder(const FilePath &folderPathname);
    
    void ExportFile(const String &fileName, Set<String> &errorLog);
    void ExportFolder(const String &folderName, Set<String> &errorLog);
    
    void ExportScene(Scene *scene, const FilePath &fileName, Set<String> &errorLog);
    
protected:
    
    void RemoveEditorNodes(Entity *rootNode);
    
    void ExportDescriptors(Scene *scene, Set<String> &errorLog);
    bool ExportTextureDescriptor(const FilePath &pathname, Set<String> &errorLog);
    bool ExportTexture(const TextureDescriptor * descriptor, Set<String> &errorLog);
    void CompressTextureIfNeed(const TextureDescriptor * descriptor, Set<String> &errorLog);

    void ExportLandscape(Scene *scene, Set<String> &errorLog);
    void ExportLandscapeFullTiledTexture(Landscape *landscape, Set<String> &errorLog);
	void BatchSceneNodes(Scene *scene, Set<String> &errorLog);
	
	// Batch Entities functionality.
	// Build the list of unique Batch Indices in the scene.
	Set<int32> BuildBatchIndicesList(Scene* scene);
	void BuildBatchIndicesListRecursive(Entity* rootEntity, Set<int32>& resultSet);
	
    // Get all the entities with the same Batch Index.
	Set<Entity*> GetEntitiesForBatchIndex(Scene* scene, int32 batchIndex);
	void GetEntitiesForBatchIndexRecursive(Entity* rootEntity, int32 batchIndex, Set<Entity*>& resultSet);

	// Select only the entities contain geometry from the list of entities to batch.
	Set<Entity*> SelectGeometryEntities(Set<Entity*>& entitiesToBatch);
	void SelectGeometryEntitiesRecursive(Entity* entity, Set<Entity*>& resultSet);

	// Perform the actual batch (vertices, indexes, textures).
	Entity* BatchEntities(Scene* scene, Set<String>& errorLog,
						  const Set<Entity*>& entitiesToBatch,
						  const String& batchedEntityName,
						  Material* batchedMaterial,
						  const BatchTexturesResult& batchTexturesResult);

	// Calculate the paremeters for the batched entity.
	void CalculateBatchedEntityParameters(const Set<Entity*>& entitiesToBatch,
										  uint32& vertexCount, uint32& indexCount,
										  AABBox3& batchedBoundingBox);

	// Merge the Polygon Groups.
	uint32 MergePolygonGroups(PolygonGroup* batchedPolygonGroup, PolygonGroup* curPolygonGroup,
							uint32 meshFormat, uint32 verticesBatched,
							  const Matrix4& curEntityMatrix,
							  const Vector3& batchedEntityCenter,
							  InstanceMaterialState* curMaterialInstance,
							  const Vector2& textureMultiply, const Vector2& textureOffset);

	// Merge the Indices.
	uint32 MergeIndices(PolygonGroup* batchedPolygonGroup, PolygonGroup* curPolygonGroup,
					  uint32 verticesBatched, uint32 indicesBatched);

	// Delete the entities.
	void DeleteEntities(const Set<Entity*>& entitiesToDelete);

	// Get the "solid" entity for current one, if present.
	Entity* GetSolidParentEntityIfPresent(Entity* entity);

	// Get the index for the next entity.
	String GetNextBatchedEntityName(Scene* scene);
	void BuildEntityNamesList(Entity* rootEntity, Set<String>& usedNames);

	// Textures batching.
	Material* BatchTextures(const Set<Entity*> entitiesToBatch, int32 batchID,
							BatchTexturesResult& batchTextureResult,
							Set<String> &errorLog);

	// Get the first render batch for the entity.
	RenderBatch* GetFirstRenderBatch(Entity* entity);

protected:
    
    SceneUtils sceneUtils;

    eGPUFamily exportForGPU;
};



#endif // __SCENE_EXPORTER_H__