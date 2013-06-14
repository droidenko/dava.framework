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

#include "SceneExporter.h"
#include "SceneEditor/SceneValidator.h"

#include "TextureCompression/PVRConverter.h"
#include "TextureCompression/DXTConverter.h"

#include "Render/TextureDescriptor.h"
#include "Qt/Scene/SceneDataManager.h"

#include "Render/GPUFamilyDescriptor.h"

#include "../StringConstants.h"
#include "BatchEntitiesHelper.h"

using namespace DAVA;

SceneExporter::SceneExporter()
{
    exportForGPU = GPU_UNKNOWN;
}

SceneExporter::~SceneExporter()
{
}


void SceneExporter::CleanFolder(const FilePath &folderPathname, Set<String> &errorLog)
{
    bool ret = FileSystem::Instance()->DeleteDirectory(folderPathname);
    if(!ret)
    {
        bool folderExists = FileSystem::Instance()->IsDirectory(folderPathname);
        if(folderExists)
        {
            errorLog.insert(String(Format("[CleanFolder] ret = %d, folder = %s", ret, folderPathname.GetAbsolutePathname().c_str())));
        }
    }
}

void SceneExporter::SetInFolder(const FilePath &folderPathname)
{
    sceneUtils.SetInFolder(folderPathname);
}

void SceneExporter::SetOutFolder(const FilePath &folderPathname)
{
    sceneUtils.SetOutFolder(folderPathname);
}

void SceneExporter::SetGPUForExporting(const String &newGPU)
{
    SetGPUForExporting(GPUFamilyDescriptor::GetGPUByName(newGPU));
}

void SceneExporter::SetGPUForExporting(const eGPUFamily newGPU)
{
    exportForGPU = newGPU;
}


void SceneExporter::ExportFile(const String &fileName, Set<String> &errorLog)
{
    Logger::Info("[SceneExporter::ExportFile] %s", fileName.c_str());
    
    FilePath filePath = sceneUtils.dataSourceFolder + fileName;
    
    //Load scene with *.sc2
    Scene *scene = new Scene();
    Entity *rootNode = scene->GetRootNode(filePath);
    if(rootNode)
    {
        int32 count = rootNode->GetChildrenCount();
		Vector<Entity*> tempV;
		tempV.reserve((count));
        for(int32 i = 0; i < count; ++i)
        {
			tempV.push_back(rootNode->GetChild(i));
        }
		for(int32 i = 0; i < count; ++i)
		{
			scene->AddNode(tempV[i]);
		}
		
		ExportScene(scene, filePath, errorLog);
    }
	else
	{
		errorLog.insert(Format("[SceneExporter::ExportFile] Can't open file %s", filePath.GetAbsolutePathname().c_str()));
	}

    SafeRelease(scene);
}

void SceneExporter::ExportScene(Scene *scene, const FilePath &fileName, Set<String> &errorLog)
{
    //Create destination folder
    String relativeFilename = fileName.GetRelativePathname(sceneUtils.dataSourceFolder);
    sceneUtils.workingFolder = fileName.GetDirectory().GetRelativePathname(sceneUtils.dataSourceFolder);
    
    FileSystem::Instance()->CreateDirectory(sceneUtils.dataFolder + sceneUtils.workingFolder, true); 
    
    scene->Update(0.1f);
    //Export scene data
    RemoveEditorNodes(scene);

    FilePath oldPath = SceneValidator::Instance()->SetPathForChecking(sceneUtils.dataSourceFolder);
    SceneValidator::Instance()->ValidateScene(scene, errorLog);
	//SceneValidator::Instance()->ValidateScales(scene, errorLog);

	BatchSceneNodes(scene, errorLog);
	
    ExportDescriptors(scene, errorLog);

    ExportLandscape(scene, errorLog);

    //save scene to new place
    FilePath tempSceneName = FilePath::CreateWithNewExtension(sceneUtils.dataSourceFolder + relativeFilename, ".exported.sc2");
    
    SceneFileV2 * outFile = new SceneFileV2();
    outFile->EnableSaveForGame(true);
    outFile->EnableDebugLog(false);
    
    outFile->SaveScene(tempSceneName, scene);
    SafeRelease(outFile);

    bool moved = FileSystem::Instance()->MoveFile(tempSceneName, sceneUtils.dataFolder + relativeFilename, true);
	if(!moved)
	{
		errorLog.insert(Format("Can't move file %s", fileName.GetAbsolutePathname().c_str()));
	}
    
    SceneValidator::Instance()->SetPathForChecking(oldPath);
}

void SceneExporter::RemoveEditorNodes(DAVA::Entity *rootNode)
{
    //Remove scene nodes
    Vector<Entity *> scenenodes;
    rootNode->GetChildNodes(scenenodes);
        
    //remove nodes from hierarhy
    Vector<Entity *>::reverse_iterator endItDeletion = scenenodes.rend();
    for (Vector<Entity *>::reverse_iterator it = scenenodes.rbegin(); it != endItDeletion; ++it)
    {
        Entity * node = *it;
		String::size_type pos = node->GetName().find(ResourceEditor::EDITOR_BASE);
        if(String::npos != pos)
        {
            node->GetParent()->RemoveNode(node);
        }
		else
		{
			DAVA::RenderComponent *renderComponent = static_cast<DAVA::RenderComponent *>(node->GetComponent(DAVA::Component::RENDER_COMPONENT));
			if(renderComponent)
			{
				DAVA::RenderObject *ro = renderComponent->GetRenderObject();
				if(ro && ro->GetType() != RenderObject::TYPE_LANDSCAPE)
				{
					DAVA::uint32 count = ro->GetRenderBatchCount();
					for(DAVA::uint32 ri = 0; ri < count; ++ri)
					{
						DAVA::Material *material = ro->GetRenderBatch(ri)->GetMaterial();
						if(material)
							material->SetStaticLightingParams(0);
					}
				}

			}
		}
    }
}

void SceneExporter::ExportDescriptors(DAVA::Scene *scene, Set<String> &errorLog)
{
    Set<FilePath> descriptorsForExport;
    SceneDataManager::EnumerateDescriptors(scene, descriptorsForExport);

    Set<FilePath>::const_iterator endIt = descriptorsForExport.end();
    Set<FilePath>::iterator it = descriptorsForExport.begin();
    for(; it != endIt; ++it)
    {
        ExportTextureDescriptor(*it, errorLog);
    }
    
    descriptorsForExport.clear();
}


bool SceneExporter::ExportTextureDescriptor(const FilePath &pathname, Set<String> &errorLog)
{
    TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(pathname);
    if(!descriptor)
    {
        errorLog.insert(Format("Can't create descriptor for pathname %s", pathname.GetAbsolutePathname().c_str()));
        return false;
    }
    
    descriptor->exportedAsGpuFamily = exportForGPU;
    descriptor->exportedAsPixelFormat = descriptor->GetPixelFormatForCompression(exportForGPU);

    if((descriptor->exportedAsGpuFamily != GPU_UNKNOWN) && (descriptor->exportedAsPixelFormat == FORMAT_INVALID))
    {
        errorLog.insert(Format("Not selected export format for pathname %s", pathname.GetAbsolutePathname().c_str()));
        
        SafeRelease(descriptor);
        return false;
    }
    
    
    String workingPathname = sceneUtils.RemoveFolderFromPath(descriptor->pathname, sceneUtils.dataSourceFolder);
    sceneUtils.PrepareFolderForCopyFile(workingPathname, errorLog);

    bool isExported = ExportTexture(descriptor, errorLog);
    if(isExported)
    {
        descriptor->Export(sceneUtils.dataFolder + workingPathname);
    }
    
    SafeRelease(descriptor);
    
    return isExported;
}

bool SceneExporter::ExportTexture(const TextureDescriptor * descriptor, Set<String> &errorLog)
{
    CompressTextureIfNeed(descriptor, errorLog);
    
    if(descriptor->exportedAsGpuFamily == GPU_UNKNOWN)
    {
        FilePath sourceTexturePathname =  FilePath::CreateWithNewExtension(descriptor->pathname, ".png");
        return sceneUtils.CopyFile(sourceTexturePathname, errorLog);
    }

    FilePath compressedTexureName = GPUFamilyDescriptor::CreatePathnameForGPU(descriptor, (eGPUFamily)descriptor->exportedAsGpuFamily);
    return sceneUtils.CopyFile(compressedTexureName, errorLog);
}



void SceneExporter::ExportFolder(const String &folderName, Set<String> &errorLog)
{
    FilePath folderPathname = sceneUtils.dataSourceFolder + folderName;
    folderPathname.MakeDirectoryPathname();
    
	FileList * fileList = new FileList(folderPathname);
    for (int32 i = 0; i < fileList->GetCount(); ++i)
	{
        FilePath pathname = fileList->GetPathname(i);
		if(fileList->IsDirectory(i))
		{
            if(!fileList->IsNavigationDirectory(i))
            {
                String workingPathname = sceneUtils.RemoveFolderFromPath(pathname, sceneUtils.dataSourceFolder);
                ExportFolder(workingPathname, errorLog);
            }
        }
        else 
        {
            if(pathname.IsEqualToExtension(".sc2"))
            {
                String::size_type exportedPos = pathname.GetAbsolutePathname().find(".exported.sc2");
                if(exportedPos != String::npos)
                {
                    //Skip temporary files, created during export
                    continue;
                }
                
                String workingPathname = sceneUtils.RemoveFolderFromPath(pathname, sceneUtils.dataSourceFolder);
                ExportFile(workingPathname, errorLog);
            }
        }
    }
    
    SafeRelease(fileList);
}



void SceneExporter::ExportLandscape(Scene *scene, Set<String> &errorLog)
{
    DVASSERT(scene);

    Landscape *landscape = EditorScene::GetLandscape(scene);
    if (landscape)
    {
        sceneUtils.CopyFile(landscape->GetHeightmapPathname(), errorLog);
        
        Landscape::eTiledShaderMode mode = landscape->GetTiledShaderMode();
        if(mode == Landscape::TILED_MODE_MIXED || mode == Landscape::TILED_MODE_TEXTURE)
        {
            ExportLandscapeFullTiledTexture(landscape, errorLog);
        }
    }
}

void SceneExporter::ExportLandscapeFullTiledTexture(Landscape *landscape, Set<String> &errorLog)
{
    if(landscape->GetTiledShaderMode() == Landscape::TILED_MODE_TILEMASK)
    {
        return;
    }
    
    FilePath textureName = landscape->GetTextureName(Landscape::TEXTURE_TILE_FULL);
    if(textureName.IsEmpty())
    {
        FilePath fullTiledPathname = landscape->GetTextureName(Landscape::TEXTURE_COLOR);
        fullTiledPathname.ReplaceExtension(".thumbnail_exported.png");
        
        String workingPathname = sceneUtils.RemoveFolderFromPath(fullTiledPathname, sceneUtils.dataSourceFolder);
        sceneUtils.PrepareFolderForCopyFile(workingPathname, errorLog);

        Texture *fullTiledTexture = Texture::GetPinkPlaceholder();
        Image *image = fullTiledTexture->CreateImageFromMemory();
        if(image)
        {
            ImageLoader::Save(image, sceneUtils.dataFolder + workingPathname);
            ImageLoader::Save(image, sceneUtils.dataSourceFolder + workingPathname);
            SafeRelease(image);
            
            TextureDescriptor *descriptor = new TextureDescriptor();
            
            FilePath descriptorPathnameInData = TextureDescriptor::GetDescriptorPathname(sceneUtils.dataFolder + workingPathname);
            descriptor->Save(descriptorPathnameInData);
            
            FilePath descriptorPathnameInDataSource = TextureDescriptor::GetDescriptorPathname(sceneUtils.dataSourceFolder + workingPathname);
            bool needToDeleteDescriptorFile = !FileSystem::Instance()->IsFile(descriptorPathnameInDataSource);

            descriptor->Save(descriptorPathnameInDataSource);
            SafeRelease(descriptor);
            
            landscape->SetTexture(Landscape::TEXTURE_TILE_FULL, sceneUtils.dataSourceFolder + workingPathname);
            
            if(needToDeleteDescriptorFile)
            {
                FileSystem::Instance()->DeleteFile(descriptorPathnameInDataSource);
            }
            FileSystem::Instance()->DeleteFile(sceneUtils.dataSourceFolder + workingPathname);

            errorLog.insert(String(Format("Full tiled texture is autogenerated png-image.", workingPathname.c_str())));
        }
        else
        {
            errorLog.insert(String(Format("Can't create image for fullTiled Texture for file %s", workingPathname.c_str())));
            landscape->SetTextureName(Landscape::TEXTURE_TILE_FULL, String(""));
        }
        
        landscape->SetTextureName(Landscape::TEXTURE_TILE_FULL, sceneUtils.dataSourceFolder + workingPathname);
    }
}





void SceneExporter::CompressTextureIfNeed(const TextureDescriptor * descriptor, Set<String> &errorLog)
{
    if(descriptor->exportedAsGpuFamily == GPU_UNKNOWN)
        return;
    
    
    FilePath compressedTexureName = GPUFamilyDescriptor::CreatePathnameForGPU(descriptor, (eGPUFamily)descriptor->exportedAsGpuFamily);
    FilePath sourceTexturePathname =  FilePath::CreateWithNewExtension(descriptor->pathname, ".png");

    bool fileExcists = FileSystem::Instance()->IsFile(compressedTexureName);
    bool needToConvert = SceneValidator::IsTextureChanged(descriptor, (eGPUFamily)descriptor->exportedAsGpuFamily);
    
    if(needToConvert || !fileExcists)
    {
        //TODO: convert to pvr/dxt
        //TODO: do we need to convert to pvr if needToConvert is false, but *.pvr file isn't at filesystem
        
        const String & extension = GPUFamilyDescriptor::GetCompressedFileExtension((eGPUFamily)descriptor->exportedAsGpuFamily, (PixelFormat)descriptor->exportedAsPixelFormat);
        
        if(extension == ".pvr")
        {
            PVRConverter::Instance()->ConvertPngToPvr(*descriptor, (eGPUFamily)descriptor->exportedAsGpuFamily);
        }
        else if(extension == ".dds")
        {
            DXTConverter::ConvertPngToDxt(*descriptor, (eGPUFamily)descriptor->exportedAsGpuFamily);
        }
        else
        {
            DVASSERT(false);
        }
        
        bool wasUpdated = descriptor->UpdateCrcForFormat((eGPUFamily)descriptor->exportedAsGpuFamily);
        if(wasUpdated)
        {
            descriptor->Save();
        }
    }
}

void SceneExporter::BatchSceneNodes(Scene *scene, Set<String> &errorLog)
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
		if (entitiesToBatch.size() == 0)
		{
			continue;
		}
		
		String batchName = GetNextBatchedEntityName(scene);
		Entity* resultEntity = BatchEntities(scene, errorLog, entitiesToBatch, batchName);
	
		if (!resultEntity)
		{
			continue;
		}

		// Add the new batched entity to the parent of the current solid entity, if any.
		Entity* parentSolidEntity = GetSolidParentEntityIfPresent(*entitiesToBatch.begin());
		Entity* parentEntity = (parentSolidEntity)->GetParent();
		if (!parentEntity)
		{
			errorLog.insert(Format("Batching: no Parent Entity for %s", parentSolidEntity->GetName().c_str()));
			continue;
		}

		parentEntity->AddNode(resultEntity);
		DeleteEntities(entitiesToBatch);
	}
}

Set<int32> SceneExporter::BuildBatchIndicesList(Scene* scene)
{
	Set<int32> resultSet;
	if (scene)
	{
		BuildBatchIndicesListRecursive(scene, resultSet);
	}

	return resultSet;
}

void SceneExporter::BuildBatchIndicesListRecursive(Entity* rootEntity, Set<int32>& resultSet)
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

Set<Entity*> SceneExporter::GetEntitiesForBatchIndex(Scene* scene, int32 batchIndex)
{
	Set<Entity*> resultSet;

	// Lookup through the whole scene.
	if (scene)
	{
		GetEntitiesForBatchIndexRecursive(scene, batchIndex, resultSet);
	}
	
	return resultSet;
}

void SceneExporter::GetEntitiesForBatchIndexRecursive(Entity* rootEntity, int32 batchIndex,
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

Entity* SceneExporter::GetSolidParentEntityIfPresent(Entity* entity)
{
	Entity* solidEntity = entity;
	while (NULL != solidEntity)
	{
		KeyedArchive *customProperties = solidEntity->GetCustomProperties();
		if(customProperties && customProperties->IsKeyExists(String(Entity::SCENE_NODE_IS_SOLID_PROPERTY_NAME)))
		{
			break;
		}
		solidEntity = solidEntity->GetParent();
	};

	// If no solid parent entity is present on this level - return the entity passed.
	return solidEntity ? solidEntity : entity;
}

Entity* SceneExporter::BatchEntities(DAVA::Scene* scene, Set<String>& errorLog,
									 const Set<Entity*>& entitiesToBatch,
									 const String& batchedEntityName)
{
	int childrenCount = entitiesToBatch.size();
	if (childrenCount == 0)
	{
		// Nothing to batch.
		return NULL;
	}

	Entity* firstEntity = (*entitiesToBatch.begin());
	if (!firstEntity)
	{
		errorLog.insert("Batching: First Entity in Batch is NULL");
		return NULL;
	}
	
	RenderComponent * firstEntityComponent = static_cast<RenderComponent*>(firstEntity->GetComponent(Component::RENDER_COMPONENT));
	if (!firstEntityComponent || !firstEntityComponent->GetRenderObject())
	{
		errorLog.insert(Format("Batching: First Entity %s does'nt have Render Object", firstEntity->GetName().c_str()));
		return NULL;
	}
	
	if (firstEntityComponent->GetRenderObject()->GetRenderBatchCount() != 1)
	{
		errorLog.insert(Format("Batching: First Entity Component for entity %s has %i Render Batches, expected 1",
							   firstEntity->GetName().c_str(),
							   firstEntityComponent->GetRenderObject()->GetRenderBatchCount()));
		return NULL;
	}
	
	RenderBatch* firstEntityRenderBatch = firstEntityComponent->GetRenderObject()->GetRenderBatch(0);
	
	Entity* parentEntity = firstEntity->GetParent();
	if (!parentEntity)
	{
		errorLog.insert(Format("Batching: Parent for First Entity %s is NULL", firstEntity->GetName().c_str()));
		return NULL;
	}

	Entity* batchedEntity = new Entity();
	batchedEntity->SetName(batchedEntityName);

	// Precalculate the sizes.
	uint32 vertexCount = 0;
	uint32 indexCount = 0;
	AABBox3 batchedBoundingBox;
	CalculateBatchedEntityParameters(entitiesToBatch, vertexCount, indexCount, batchedBoundingBox);

	if (vertexCount == 0 || indexCount == 0)
	{
		errorLog.insert(Format("Batching: Vertex/Index count for the Batched Entity %s is 0",
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
	for (Set<Entity*>::iterator iter = entitiesToBatch.begin(); iter != entitiesToBatch.end(); iter ++)
	{
		// Batch the Vertices and Indices.
		Entity* curEntity = (*iter);

		// The Transformation Matrix has to be taken from the upper's level "solid" entity,
		// if it exists.
		Entity* curSolidEntity = GetSolidParentEntityIfPresent(curEntity);
		Matrix4 curEntityMatrix = curSolidEntity->GetLocalTransform();
		RenderComponent * component = static_cast<RenderComponent*>(curEntity->GetComponent(Component::RENDER_COMPONENT));

		if(!component || !component->GetRenderObject())
		{
			errorLog.insert(Format("Batching: Entity %s does'nt have Render Object", curEntity->GetName().c_str()));
			continue;
		}

		if (component->GetRenderObject()->GetType() != RenderObject::TYPE_MESH)
		{
			errorLog.insert(Format("Batching: Entity %s is not a Mesh", curEntity->GetName().c_str()));
			continue;
		}

		// Yuri Coder, 2013/06/11. Currently can merge only render objects with one render batch.
		Mesh* curMesh = static_cast<Mesh*>(component->GetRenderObject());
		uint32 curRenderBatchesCount = curMesh->GetRenderBatchCount();
		DVASSERT(curRenderBatchesCount == 1);

		// Batch the polygon groups.
		PolygonGroup* curPolygonGroup = curMesh->GetPolygonGroup(0);
		uint32 verticesBatchedOnThisPass = MergePolygonGroups(batchedPolygonGroup, curPolygonGroup, meshFormat,
															  verticesBatched, curEntityMatrix, batchedEntityCenter);
		uint32 indicesBatchedOnThisPass = MergeIndices(batchedPolygonGroup, curPolygonGroup, verticesBatched,
													   indicesBatched);

		verticesBatched += verticesBatchedOnThisPass;
		indicesBatched += indicesBatchedOnThisPass;
	}
	
	// Done batching.
	DVASSERT(verticesBatched == vertexCount);
	DVASSERT(indicesBatched == indexCount);
	batchedPolygonGroup->BuildBuffers();
	
	batchedMesh->AddPolygonGroup(batchedPolygonGroup, firstEntityRenderBatch->GetMaterial());
	
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

void SceneExporter::CalculateBatchedEntityParameters(const Set<Entity*>& entitiesToBatch,
													 uint32& vertexCount, uint32& indexCount,
													 AABBox3& batchedBoundingBox)
{
	for (Set<Entity*>::iterator iter = entitiesToBatch.begin();
		 iter != entitiesToBatch.end(); iter ++)
	{
		Entity* curEntity = (*iter);
		RenderComponent * component = static_cast<RenderComponent*>(curEntity->GetComponent(Component::RENDER_COMPONENT));
		
		if(!component || !component->GetRenderObject())
		{
			// TODO! error log!
			continue;
		}
		
		if (component->GetRenderObject()->GetType() != RenderObject::TYPE_MESH)
		{
			// TODO! error log!
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

uint32 SceneExporter::MergePolygonGroups(PolygonGroup* batchedPolygonGroup, PolygonGroup* curPolygonGroup,
										 uint32 meshFormat, uint32 verticesBatched,
										 const Matrix4& curEntityMatrix, const Vector3& batchedEntityCenter)
{
	if (!batchedPolygonGroup || !curPolygonGroup)
	{
		DVASSERT(false);
		return;
	}

	// The translation matrix is needed to move the bounding box to the (0,0,0) coord.
	// The resulting entity will then be moved back to the original position.
	Matrix4 translateMatrix;
	translateMatrix.CreateTranslation(-batchedEntityCenter);

	uint32 verticesToBatch = curPolygonGroup->GetVertexCount();
	Vector3 vector3Param;
	Vector2 vector2Param;
	for (uint32 i = 0; i < verticesToBatch; i ++)
	{
		uint32 positionInBatch = i + verticesBatched;
		if (meshFormat & EVF_VERTEX && curPolygonGroup->vertexArray)
		{
			curPolygonGroup->GetCoord(i, vector3Param);
			Vector3 newCoordParam = vector3Param * curEntityMatrix * translateMatrix;
			batchedPolygonGroup->SetCoord(positionInBatch, newCoordParam);
		}

		if (meshFormat & EVF_NORMAL && curPolygonGroup->normalArray)
		{
			curPolygonGroup->GetNormal(i, vector3Param);
			batchedPolygonGroup->SetNormal(positionInBatch, vector3Param);
		}

		if (meshFormat & EVF_TANGENT && curPolygonGroup->tangentArray)
		{
			curPolygonGroup->GetTangent(i, vector3Param);
			batchedPolygonGroup->SetTangent(positionInBatch, vector3Param);
		}

		if (meshFormat & EVF_TEXCOORD0 && curPolygonGroup->textureCoordCount > 0)
		{
			curPolygonGroup->GetTexcoord(0, i, vector2Param);
			batchedPolygonGroup->SetTexcoord(0, positionInBatch, vector2Param);
		}
	
		if (meshFormat & EVF_TEXCOORD1 && curPolygonGroup->textureCoordCount > 1)
		{
			curPolygonGroup->GetTexcoord(1, i, vector2Param);
			batchedPolygonGroup->SetTexcoord(1, positionInBatch, vector2Param);
		}
	}
	
	return verticesToBatch;
}

uint32 SceneExporter::MergeIndices(PolygonGroup* batchedPolygonGroup, PolygonGroup* curPolygonGroup,
								 uint32 verticesBatched, uint32 indicesBatched)
{
	if (!batchedPolygonGroup || !curPolygonGroup)
	{
		DVASSERT(false);
		return;
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

void SceneExporter::DeleteEntities(const Set<Entity*>& entitiesToDelete)
{
	for (Set<Entity*>::iterator iter = entitiesToDelete.begin(); iter != entitiesToDelete.end();
		 iter ++)
	{
		Entity* entityToDelete = *iter;
		if (!entityToDelete)
		{
			continue;
		}
		
		Entity* solidEntityToDelete = GetSolidParentEntityIfPresent(entityToDelete);
		Entity* parentNode = solidEntityToDelete->GetParent();
		DVASSERT(parentNode);
		
		parentNode->Retain();
		parentNode->RemoveNode(solidEntityToDelete);
		parentNode->Release();
	}
}

String SceneExporter::GetNextBatchedEntityName(Scene* scene)
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

void SceneExporter::BuildEntityNamesList(Entity* rootEntity, Set<String>& usedNames)
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