 /*==================================================================================
 Copyright (c) 2008, binaryzebra
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the binaryzebra nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/


#include "Scene3D/Systems/MaterialSystem.h"
#include "Render/Material/NMaterial.h"
#include "Render/RenderManager.h"
#include "Render/RenderState.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/RenderBase.h"
#include "FileSystem/YamlParser.h"
#include "Render/Shader.h"
#include "Render/Material/MaterialGraph.h"
#include "Render/Material/MaterialCompiler.h"
#include "Render/ShaderCache.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Utils/StringFormat.h"
#include "Render/Material/NMaterialTemplate.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/TextureDescriptor.h"

namespace DAVA
{
    
	static const FastName DEFINE_VERTEX_LIT("VERTEX_LIT");
	static const FastName DEFINE_PIXEL_LIT("PIXEL_LIT");
	
	const FastName NMaterial::TEXTURE_ALBEDO("albedo");
	const FastName NMaterial::TEXTURE_NORMAL("normalmap");
	const FastName NMaterial::TEXTURE_DETAIL("detail");
	const FastName NMaterial::TEXTURE_LIGHTMAP("lightmap");
	const FastName NMaterial::TEXTURE_DECAL("decal");
	const FastName NMaterial::TEXTURE_CUBEMAP("cubemap");
	
	const FastName NMaterial::PARAM_LIGHT_POSITION0("lightPosition0");
	const FastName NMaterial::PARAM_PROP_AMBIENT_COLOR("ambientColor");
	const FastName NMaterial::PARAM_PROP_DIFFUSE_COLOR("diffuseColor");
	const FastName NMaterial::PARAM_PROP_SPECULAR_COLOR("specularColor");
	const FastName NMaterial::PARAM_LIGHT_AMBIENT_COLOR("materialLightAmbientColor");
	const FastName NMaterial::PARAM_LIGHT_DIFFUSE_COLOR("materialLightDiffuseColor");
	const FastName NMaterial::PARAM_LIGHT_SPECULAR_COLOR("materialLightSpecularColor");
	const FastName NMaterial::PARAM_LIGHT_INTENSITY0("lightIntensity0");
	const FastName NMaterial::PARAM_MATERIAL_SPECULAR_SHININESS("materialSpecularShininess");
	const FastName NMaterial::PARAM_FOG_COLOR("fogColor");
	const FastName NMaterial::PARAM_FOG_DENSITY("fogDensity");
	const FastName NMaterial::PARAM_FLAT_COLOR("flatColor");
	const FastName NMaterial::PARAM_TEXTURE0_SHIFT("texture0Shift");
	const FastName NMaterial::PARAM_UV_OFFSET("uvOffset");
	const FastName NMaterial::PARAM_UV_SCALE("uvScale");
    const FastName NMaterial::PARAM_SPEED_TREE_LEAF_COLOR_MUL("treeLeafColorMul");
    const FastName NMaterial::PARAM_SPEED_TREE_LEAF_OCC_MUL("treeLeafOcclusionMul");
    const FastName NMaterial::PARAM_SPEED_TREE_LEAF_OCC_OFFSET("treeLeafOcclusionOffset");
	
	const FastName NMaterial::FLAG_VERTEXFOG = FastName("VERTEX_FOG");
	const FastName NMaterial::FLAG_TEXTURESHIFT = FastName("TEXTURE0_SHIFT_ENABLED");
    const FastName NMaterial::FLAG_TEXTURE0_ANIMATION_SHIFT = FastName("TEXTURE0_ANIMATION_SHIFT");
	const FastName NMaterial::FLAG_FLATCOLOR = FastName("FLATCOLOR");
	const FastName NMaterial::FLAG_LIGHTMAPONLY = FastName("MATERIAL_VIEW_LIGHTMAP_ONLY");
	const FastName NMaterial::FLAG_TEXTUREONLY = FastName("MATERIAL_VIEW_TEXTURE_ONLY");
	const FastName NMaterial::FLAG_SETUPLIGHTMAP = FastName("SETUP_LIGHTMAP");

	static FastName TEXTURE_NAME_PROPS[] = {
		NMaterial::TEXTURE_ALBEDO,
		NMaterial::TEXTURE_NORMAL,
		NMaterial::TEXTURE_DETAIL,
		NMaterial::TEXTURE_LIGHTMAP,
		NMaterial::TEXTURE_DECAL
	};
	
	static FastName RUNTIME_ONLY_FLAGS[] =
	{
		NMaterial::FLAG_LIGHTMAPONLY,
		NMaterial::FLAG_TEXTUREONLY,
		NMaterial::FLAG_SETUPLIGHTMAP
	};
	
	const FastName NMaterial::DEFAULT_QUALITY_NAME = FastName("Normal");
	
	Texture* NMaterial::stubCubemapTexture = NULL;
	Texture* NMaterial::stub2dTexture = NULL;
		
////////////////////////////////////////////////////////////////////////////////
	
	NMaterial::NMaterial() :
	materialType(NMaterial::MATERIALTYPE_NONE),
	materialKey(0),
	parent(NULL),
	requiredVertexFormat(EVF_FORCE_DWORD),
	lightCount(0),
	illuminationParams(NULL),
	materialSetFlags(8),
	baseTechnique(NULL),
	activePassInstance(NULL),
	activeRenderPass(NULL),
	instancePasses(4),
	textures(8),
	materialDynamicLit(false),
	materialTemplate(NULL),
	materialProperties(16),
	instancePassRenderStates(4),
	materialSortKey(0)
	{
		memset(lights, 0, sizeof(lights));
	}
	
	NMaterial::~NMaterial()
	{
		SetParent(NULL);

		ReleaseInstancePasses();
		
		for(HashMap<FastName, UniqueHandle>::iterator it = instancePassRenderStates.begin();
			it != instancePassRenderStates.end();
			++it)
		{
			RenderManager::Instance()->ReleaseRenderStateData(it->second);
		}
		
		for(HashMap<FastName, NMaterialProperty*>::iterator it = materialProperties.begin();
			it != materialProperties.end();
			++it)
		{
			SafeDelete(it->second);
		}
		materialProperties.clear();
		
		for(HashMap<FastName, TextureBucket*>::iterator it = textures.begin();
			it != textures.end();
			++it)
		{
			SafeRelease(it->second->texture);
			SafeDelete(it->second);
		}
		textures.clear();
				
		SafeDelete(illuminationParams);
		
		SafeRelease(baseTechnique);
	}
				
	void NMaterial::SetParent(NMaterial* newParent, bool inheritTemplate)
	{
		DVASSERT(this != newParent);
		
		if(newParent != parent &&
		   newParent != this)
		{
			if(parent)
			{
				Vector<NMaterial*>::iterator curMaterial = std::find(parent->children.begin(),
																	 parent->children.end(),
																	 this);
				
				DVASSERT(curMaterial != parent->children.end());
				if(curMaterial != parent->children.end())
				{
					parent->children.erase(curMaterial);
				}
				
				SafeRelease(parent);
			}
			
			if(newParent)
			{
				DVASSERT(std::find(newParent->children.begin(), newParent->children.end(), this) == newParent->children.end());
				
				newParent->children.push_back(this);
			}
			
			parent = SafeRetain(newParent);;
			
			OnParentChanged(newParent, inheritTemplate);
		}
	}

	void NMaterial::SetFlag(const FastName& flag, eFlagValue flagValue)
	{
		materialSetFlags.insert(flag, flagValue);

		UpdateShaderWithFlags(true);
	}
    
    void NMaterial::ResetFlag(const FastName& flag)
    {
        if(materialSetFlags.count(flag) > 0)
        {
            materialSetFlags.erase(flag);
            
            UpdateShaderWithFlags(true);
        }
    }
			
	int32 NMaterial::GetFlagValue(const FastName& flag) const
	{
		int32 flagValue = NMaterial::FlagOff;
        
        if(materialSetFlags.count(flag) > 0)
        {
            flagValue = materialSetFlags.at(flag);
        }
        else
        {
            if(parent)
            {
                flagValue = parent->GetFlagValue(flag) | NMaterial::FlagInherited;
            }
        }
			
		return flagValue;
	}
    
    bool NMaterial::IsFlagEffective(const FastName& flag) const
    {
        int32 flagValue = GetFlagValue(flag);
        
        return ((flagValue & 1) == NMaterial::FlagOn);
    }
			
	void NMaterial::Save(KeyedArchive * archive,
						 SerializationContext* serializationContext)
	{
		DataNode::Save(archive, serializationContext);
		
		archive->SetString("materialName", (materialName.IsValid()) ? materialName.c_str() : "");
		archive->SetInt32("materialType", (int32)materialType);
		archive->SetUInt64("materialKey", materialKey);
		
		if(NMaterial::MATERIALTYPE_INSTANCE == materialType &&
		   parent)
		{
			archive->SetUInt64("parentMaterialKey", parent->materialKey);
			//Logger::FrameworkDebug("[NMaterial::Save] Parent: %s, Child %s, parent key% %ld",
			//					   parent->GetName().c_str(),
			//					   this->GetName().c_str(),
			//					   parent->materialKey);
		}
		
		if(serializationContext->GetDefaultMaterialQuality() != currentQuality)
		{
			archive->SetString("materialQuality", currentQuality.c_str());
		}
		
		DVASSERT(materialTemplate);
		archive->SetString("materialTemplate", (materialTemplate) ? materialTemplate->name.c_str() : "");
		
		if(instancePassRenderStates.size() > 0)
		{
			KeyedArchive* materialCustomStates = new KeyedArchive();
			for(HashMap<FastName, UniqueHandle>::iterator it = instancePassRenderStates.begin();
				it != instancePassRenderStates.end();
				++it)
			{
				UniqueHandle currentHandle = it->second;
				
				const RenderStateData* stateData = RenderManager::Instance()->GetRenderStateData(currentHandle);
				materialCustomStates->SetByteArray(it->first.c_str(), (uint8*)stateData, sizeof(RenderStateData));
			}
			archive->SetArchive("materialCustomStates", materialCustomStates);
			SafeRelease(materialCustomStates);
		}
		
		KeyedArchive* materialTextures = new KeyedArchive();
		for(HashMap<FastName, TextureBucket*>::iterator it = textures.begin();
			it != textures.end();
			++it)
		{
			if(it->second->texture)
			{
				String texturePath = it->second->path.GetRelativePathname(serializationContext->GetScenePath());
				
				if(texturePath.size() > 0)
				{
					materialTextures->SetString(it->first.c_str(),
												texturePath);
				}
				else
				{
					Logger::FrameworkDebug("[NMaterial::Save] Material '%s' has empty texture '%s'! Skipping...", materialName.c_str(), it->first.c_str());
				}
			}
		}
		archive->SetArchive("textures", materialTextures);
		SafeRelease(materialTextures);
		
		KeyedArchive* materialProps = new KeyedArchive();
		for(HashMap<FastName, NMaterialProperty*>::iterator it = materialProperties.begin();
			it != materialProperties.end();
			++it)
		{
			NMaterialProperty* property = it->second;
			
			uint32 dataSize = Shader::GetUniformTypeSize(property->type) * property->size;
			uint8* propertyStorage = new uint8[dataSize + sizeof(property->type) + sizeof(property->size)];
			
			memcpy(propertyStorage, &property->type, sizeof(property->type));
			memcpy(propertyStorage + sizeof(property->type), &property->size, sizeof(property->size));
			memcpy(propertyStorage + sizeof(property->type) + sizeof(property->size), property->data, dataSize);
			
			materialProps->SetByteArray(it->first.c_str(), propertyStorage, dataSize + sizeof(property->type) + sizeof(property->size));
			
			SafeDeleteArray(propertyStorage);
		}
		archive->SetArchive("properties", materialProps);
		SafeRelease(materialProps);
		
		KeyedArchive* materialSetFlagsArchive = new KeyedArchive();
		for(HashMap<FastName, int32>::iterator it = materialSetFlags.begin();
			it != materialSetFlags.end();
			++it)
		{
			if(!IsRuntimeFlag(it->first))
			{
				materialSetFlagsArchive->SetInt32(it->first.c_str(), it->second);
			}
		}
		archive->SetArchive("setFlags", materialSetFlagsArchive);
		SafeRelease(materialSetFlagsArchive);
				
		if(illuminationParams)
		{
			archive->SetBool("illumination.isUsed", illuminationParams->isUsed);
			archive->SetBool("illumination.castShadow", illuminationParams->castShadow);
			archive->SetBool("illumination.receiveShadow", illuminationParams->receiveShadow);
			archive->SetInt32("illumination.lightmapSize", illuminationParams->lightmapSize);
		}
	}
	
	void NMaterial::Load(KeyedArchive * archive,
						 SerializationContext* serializationContext)
	{
		DataNode::Load(archive, serializationContext);
		
		materialName = FastName(archive->GetString("materialName"));
		materialType = (NMaterial::eMaterialType)archive->GetInt32("materialType");
		materialKey = (NMaterial::NMaterialKey)archive->GetUInt64("materialKey");
		pointer = materialKey;
		
		DataNode::SetName(materialName.c_str());
		
		if(archive->IsKeyExists("materialCustomStates"))
		{
			RenderStateData stateData;
			const Map<String, VariantType*>& customRenderState = archive->GetArchive("materialCustomStates")->GetArchieveData();
			for(Map<String, VariantType*>::const_iterator it = customRenderState.begin();
				it != customRenderState.end();
				++it)
			{
				DVASSERT(it->second->AsByteArraySize() == sizeof(RenderStateData));
				const uint8* array = it->second->AsByteArray();
				memcpy(&stateData, array, sizeof(RenderStateData));
				
				UniqueHandle currentHandle = RenderManager::Instance()->AddRenderStateData(&stateData);
				instancePassRenderStates.insert(FastName(it->first.c_str()), currentHandle);
			}
		}
		
		if(archive->IsKeyExists("materialQuality"))
		{
			currentQuality = FastName(archive->GetString("materialQuality"));
		}
		else
		{
			currentQuality = serializationContext->GetDefaultMaterialQuality();
		}
		
		String materialTemplateName = archive->GetString("materialTemplate");
		if(materialTemplateName.size() > 0)
		{
			NMaterialHelper::SwitchTemplate(this, FastName(materialTemplateName.c_str()));
		}
		else
		{
			//VI: will inherit from parent probably
			materialTemplate = NULL;
		}
		
		const Map<String, VariantType*>& propsMap = archive->GetArchive("properties")->GetArchieveData();
		for(Map<String, VariantType*>::const_iterator it = propsMap.begin();
			it != propsMap.end();
			++it)
		{
			const VariantType* propVariant = it->second;
			DVASSERT(VariantType::TYPE_BYTE_ARRAY == propVariant->type);
			DVASSERT(propVariant->AsByteArraySize() >= (sizeof(uint32) +sizeof(uint32)));
			
			const uint8* ptr = propVariant->AsByteArray();
			
			SetPropertyValue(FastName(it->first),
							 *(Shader::eUniformType*)ptr,
							 *(ptr + sizeof(Shader::eUniformType)),
							 ptr + sizeof(Shader::eUniformType) + sizeof(uint8));
		}
		
		const Map<String, VariantType*>& texturesMap = archive->GetArchive("textures")->GetArchieveData();
		for(Map<String, VariantType*>::const_iterator it = texturesMap.begin();
			it != texturesMap.end();
			++it)
		{
			String relativePathname = it->second->AsString();
			SetTexture(FastName(it->first), serializationContext->GetScenePath() + relativePathname);
		}
						
		if(archive->IsKeyExists("illumination.isUsed"))
		{
			illuminationParams = new IlluminationParams();
			
			illuminationParams->isUsed = archive->GetBool("illumination.isUsed", illuminationParams->isUsed);
			illuminationParams->castShadow = archive->GetBool("illumination.castShadow", illuminationParams->castShadow);
			illuminationParams->receiveShadow = archive->GetBool("illumination.receiveShadow", illuminationParams->receiveShadow);
			illuminationParams->lightmapSize = archive->GetInt32("illumination.lightmapSize", illuminationParams->lightmapSize);
		}
						
		const Map<String, VariantType*>& flagsMap = archive->GetArchive("setFlags")->GetArchieveData();
		for(Map<String, VariantType*>::const_iterator it = flagsMap.begin();
			it != flagsMap.end();
			++it)
		{
			SetFlag(FastName(it->first), (NMaterial::eFlagValue)it->second->AsInt32());
		}
		
		if(NMaterial::MATERIALTYPE_INSTANCE == materialType)
		{
			uint64 parentKey = archive->GetUInt64("parentMaterialKey");
			serializationContext->AddBinding(parentKey, this);
		}
	}
	
	void NMaterial::SetQuality(const FastName& stateName)
	{
		DVASSERT(stateName.IsValid());
		orderedQuality = stateName;
	}
	
	bool NMaterial::ReloadQuality(bool force)
	{
		DVASSERT(orderedQuality.IsValid());
		DVASSERT(materialTemplate);
		
		if(!orderedQuality.IsValid())
		{
			orderedQuality = NMaterial::DEFAULT_QUALITY_NAME;
		}
		
		bool result = (materialTemplate->techniqueStateMap.count(orderedQuality) > 0);
		if(result &&
		   (orderedQuality != currentQuality ||
		   force))
		{
			currentQuality = orderedQuality;
			
			if(NMaterial::MATERIALTYPE_INSTANCE == materialType)
			{
				OnInstanceQualityChanged();
			}
			else if(NMaterial::MATERIALTYPE_MATERIAL == materialType)
			{
				UpdateMaterialTemplate();
				
				LoadActiveTextures();
				
				this->Retain();
				
				size_t childrenCount = children.size();
				for(size_t i = 0; i < childrenCount; ++i)
				{
					NMaterial* child = children[i];
					
					child->SetQuality(currentQuality);
					child->ReloadQuality(force);
				}
				
				//VI: TODO: review if this call is realy needed at this point
				//CleanupUnusedTextures();
				
				this->Release();
			}
			else
			{
				DVASSERT(false && "Material is not initialized properly!");
			}
		}
		
		return result;
	}

	NMaterial* NMaterial::Clone()
	{
		NMaterial* clonedMaterial = NULL;
		if(NMaterial::MATERIALTYPE_MATERIAL == materialType)
		{
			clonedMaterial = NMaterial::CreateMaterial(materialName,
															materialTemplate->name,
															currentQuality);
			
		}
		else if(NMaterial::MATERIALTYPE_INSTANCE == materialType)
		{
			clonedMaterial = NMaterial::CreateMaterialInstance();
		}
		else
		{
			DVASSERT(false && "Material is not initialized properly!");
			return clonedMaterial;
		}
		
		clonedMaterial->SetName(GetName());
				
		for(HashMap<FastName, NMaterialProperty*>::iterator it = materialProperties.begin();
			it != materialProperties.end();
			++it)
		{
			clonedMaterial->materialProperties.insert(it->first, it->second->Clone());
		}
		
		for(HashMap<FastName, TextureBucket*>::iterator it = textures.begin();
			it != textures.end();
			++it)
		{
			clonedMaterial->SetTexture(it->first, it->second->path);
		}
		
		if(illuminationParams)
		{
			clonedMaterial->illuminationParams = new IlluminationParams(*illuminationParams);
		}
				
		if(NMaterial::MATERIALTYPE_INSTANCE == materialType)
		{
			clonedMaterial->SetParent(parent);
		}
		
		for(HashMap<FastName, int32>::iterator it = materialSetFlags.begin();
			it != materialSetFlags.end();
			++it)
		{
			clonedMaterial->SetFlag(it->first, (NMaterial::eFlagValue)it->second);
		}
		
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			RenderPassInstance* currentPass = it->second;
			
			if(currentPass->dirtyState)
			{
				RenderPassInstance* clonedPass = clonedMaterial->instancePasses.at(it->first);
				DVASSERT(clonedPass);
				
				clonedPass->dirtyState = true;
				clonedPass->renderState.stateHandle = currentPass->renderState.stateHandle;
			}
		}
		
		for(HashMap<FastName, UniqueHandle>::iterator it = instancePassRenderStates.begin();
			it != instancePassRenderStates.end();
			++it)
		{
			clonedMaterial->instancePassRenderStates.insert(it->first, it->second);
			RenderManager::Instance()->RetainRenderStateData(it->second);
		}
		
		//DataNode properties
		clonedMaterial->pointer = pointer;
		clonedMaterial->scene = scene;
		clonedMaterial->index = index;
		clonedMaterial->nodeFlags = nodeFlags;

		return clonedMaterial;
	}
	
	NMaterial* NMaterial::Clone(const String& newName)
	{
		NMaterial* clonedMaterial = Clone();
		clonedMaterial->SetName(newName);
		clonedMaterial->SetMaterialName(FastName(newName));
		clonedMaterial->SetMaterialKey((NMaterial::NMaterialKey)clonedMaterial);
		
		return clonedMaterial;
	}
    
	IlluminationParams * NMaterial::GetIlluminationParams()
    {
        if(!illuminationParams)
            illuminationParams = new IlluminationParams();
        
        return illuminationParams;
    }
	
    void NMaterial::ReleaseIlluminationParams()
    {
        SafeDelete(illuminationParams);
    }
	
    void NMaterial::RemoveTexture(const FastName& textureFastName)
    {
        TextureBucket* bucket = textures.at(textureFastName);
        DVASSERT(bucket);
        
        if(bucket)
        {
            textures.erase(textureFastName);
            SafeRelease(bucket->texture);
            SafeDelete(bucket);
            
            SetTexturesDirty();
        }
    }

    void NMaterial::SetTexture(const FastName& textureFastName,
							   const FilePath& texturePath)
	{
		TextureBucket* bucket = textures.at(textureFastName);
		if(NULL == bucket)
		{
			bucket = new TextureBucket();
			bucket->texture = NULL;
			textures.insert(textureFastName, bucket);
		}
		
		if(bucket->path != texturePath)
		{
			SafeRelease(bucket->texture);
			bucket->path = texturePath;
			
			if(IsTextureActive(textureFastName))
			{
				bucket->texture = Texture::CreateFromFile(texturePath);
			}
			
			SetTexturesDirty();
		}
	}
	
	void NMaterial::SetTexture(const FastName& textureFastName,
							   Texture* texture)
	{
		TextureBucket* bucket = textures.at(textureFastName);
		if(NULL == bucket)
		{
			bucket = new TextureBucket();
			textures.insert(textureFastName, bucket);
		}

		if(texture != bucket->texture)
		{
			SafeRelease(bucket->texture);
			
			bucket->texture = SafeRetain(texture);
			bucket->path = (bucket->texture) ? bucket->texture->texDescriptor->pathname : FilePath();
			
			SetTexturesDirty();
		}
	}
	
    Texture * NMaterial::GetTexture(const FastName& textureFastName) const
	{
		TextureBucket* bucket = textures.at(textureFastName);
		return (NULL == bucket) ? NULL : bucket->texture;
	}
	
	const FilePath& NMaterial::GetTexturePath(const FastName& textureFastName) const
	{
		static FilePath invalidEmptyPath;
		TextureBucket* bucket = textures.at(textureFastName);
		return (NULL == bucket) ? invalidEmptyPath : bucket->path;
	}
	
    Texture * NMaterial::GetTexture(uint32 index) const
	{
		DVASSERT(index >= 0 && index < textures.size());
		
		TextureBucket* bucket = textures.valueByIndex(index);
		return bucket->texture;
	}
	
	const FilePath& NMaterial::GetTexturePath(uint32 index) const
	{
		DVASSERT(index >= 0 && index < textures.size());
		
		TextureBucket* bucket = textures.valueByIndex(index);
		return bucket->path;
	}
	
	const FastName& NMaterial::GetTextureName(uint32 index) const
	{
		DVASSERT(index >= 0 && index < textures.size());
		
		return textures.keyByIndex(index);
	}
	
    uint32 NMaterial::GetTextureCount() const
	{
		return textures.size();
	}
    
    void NMaterial::SetPropertyValue(const FastName & keyName,
						  Shader::eUniformType type,
						  uint32 size,
						  const void * data)
	{
		DVASSERT(data);
		DVASSERT(size);
		DVASSERT(keyName.IsValid());
		
		size_t dataSize = Shader::GetUniformTypeSize(type) * size;
		NMaterialProperty * materialProperty = materialProperties.at(keyName);
		if (materialProperty)
		{
			if (materialProperty->type != type || materialProperty->size != size)
			{
				//VI: material property type or size chnage should never happen at runtime
				DVASSERT(false && "Runtime change of material property type!");
				
				NMaterialProperty* newProp = new NMaterialProperty();
				newProp->size = size;
				newProp->type = type;
				newProp->data = new uint8[dataSize];
				
				OnMaterialPropertyRemoved(keyName);
				
				SafeDelete(materialProperty);
				materialProperties.insert(keyName, newProp);
				
				OnMaterialPropertyAdded(keyName);
				
				materialProperty = newProp;
			}
		}
		else
		{
			materialProperty = new NMaterialProperty();
			materialProperty->size = size;
			materialProperty->type = type;
			materialProperty->data = new uint8[dataSize];
			materialProperties.insert(keyName, materialProperty);
			
			OnMaterialPropertyAdded(keyName);
		}
		
		memcpy(materialProperty->data, data, dataSize);
		
		//VI: this is temporary solution. It has to be removed once lighting system + autobind system is ready
		if(IsDynamicLit() && IsLightingProperty(keyName))
		{
			UpdateLightingProperties(lights[0]);
		}
	}
	
	NMaterialProperty* NMaterial::GetPropertyValue(const FastName & keyName) const
	{
		const NMaterial* currentMaterial = this;
		NMaterialProperty * property = NULL;
		while(currentMaterial != NULL)
		{
			property = currentMaterial->materialProperties.at(keyName);
			if (property)
			{
				break;
			}
			
			currentMaterial = currentMaterial->parent;
		}
		
		return property;
	}
	
	NMaterialProperty* NMaterial::GetMaterialProperty(const FastName & keyName) const
	{
		return materialProperties.at(keyName);
	}
	
	void NMaterial::RemoveMaterialProperty(const FastName & keyName)
	{
		NMaterialProperty* prop = materialProperties.at(keyName);
		if(prop)
		{
			materialProperties.erase(keyName);
			SafeDelete(prop);
		}
		
		OnMaterialPropertyRemoved(keyName);
	}
	
	void NMaterial::SetMaterialName(const FastName& name)
	{
		materialName = name;
	}
	
	void NMaterial::SetMaterialTemplate(const NMaterialTemplate* matTemplate,
										const FastName& defaultQuality)
	{
		materialTemplate = matTemplate;
		currentQuality = defaultQuality;
		
		OnMaterialTemplateChanged();
		
		LoadActiveTextures();
		
		this->Retain();
				
		//{VI: temporray code should be removed once lighting system is up
		materialDynamicLit = (baseTechnique->GetLayersSet().count(LAYER_SHADOW_VOLUME) != 0);
		
		if(!materialDynamicLit)
		{
			uint32 passCount = baseTechnique->GetPassCount();
			for(uint32 i = 0; i < passCount; ++i)
			{
				RenderTechniquePass* pass = baseTechnique->GetPassByIndex(i);
				const FastNameSet& defines = pass->GetUniqueDefineSet();
				materialDynamicLit = materialDynamicLit ||
				defines.count(DEFINE_VERTEX_LIT) ||
				defines.count(DEFINE_PIXEL_LIT);
			}
		}
		//}
		
		size_t childrenCount = children.size();
		for(size_t i = 0; i < childrenCount; ++i)
		{
			children[i]->SetMaterialTemplate(matTemplate, defaultQuality);
		}
		
		//VI: TODO: review if this call is realy needed at this point
		//CleanupUnusedTextures();
		
		this->Release();
	}
	
	void NMaterial::OnMaterialTemplateChanged()
	{
		UpdateMaterialTemplate();
	}

	void NMaterial::OnParentChanged(NMaterial* newParent, bool inheritTemplate)
	{
		NMaterial* oldParent = parent;
		parent = SafeRetain(newParent);
		SafeRelease(oldParent);
		
		bool useParentTemplate = (inheritTemplate || NULL == materialTemplate);
		
		if(newParent && useParentTemplate)
		{
			SetMaterialTemplate(newParent->materialTemplate, newParent->currentQuality);
		}
		
		SetTexturesDirty();
	}
	
	void NMaterial::OnParentFlagsChanged()
	{
		UpdateShaderWithFlags();
	}
	
	void NMaterial::OnInstanceQualityChanged()
	{
		UpdateMaterialTemplate();
		
		LoadActiveTextures();
		
		//VI: TODO: review if this call is realy needed at this point
		//CleanupUnusedTextures();
	}
	
	void NMaterial::BuildEffectiveFlagSet(FastNameSet& effectiveFlagSet)
	{
		effectiveFlagSet.clear();
				
		BuildEffectiveFlagSetInternal(effectiveFlagSet);
	}
					
	void NMaterial::BuildEffectiveFlagSetInternal(FastNameSet& effectiveFlagSet)
    {
        if(parent)
        {
            parent->BuildEffectiveFlagSetInternal(effectiveFlagSet);
        }
    
        //VI: now when flags were set by parent let's set them from our collection or filter out overriden flags.
		for(HashMap<FastName, int32>::iterator it = materialSetFlags.begin();
			it != materialSetFlags.end();
			++it)
		{
			if(NMaterial::FlagOn == it->second)
			{
				effectiveFlagSet.Insert(it->first);
			}
            else
            {
                effectiveFlagSet.erase(it->first);
            }
		}
	}
	
	void NMaterial::ReleaseInstancePasses()
	{
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			RenderPassInstance* passInstance = it->second;
			
			//VI: TODO: make sure need to release shader
			//SafeRelease(passInstance->renderState.shader);
			
			if(passInstance->renderState.textureState != InvalidUniqueHandle)
			{
				RenderManager::Instance()->ReleaseTextureStateData(passInstance->renderState.textureState);
			}
			
			SafeDelete(it->second);
		}
		instancePasses.clear();
		
		activePassInstance = NULL;
		activePassName.Reset();
		activeRenderPass = NULL;
	}
	
	void NMaterial::UpdateMaterialTemplate()
	{
		DVASSERT(materialTemplate);
		
		ReleaseInstancePasses();
		
		FastNameSet effectiveFlags;
		BuildEffectiveFlagSet(effectiveFlags);
		
		FastName techniqueName;
		if(currentQuality.IsValid())
		{
			techniqueName = materialTemplate->techniqueStateMap.at(currentQuality);
		}
		
		if(!techniqueName.IsValid())
		{
			techniqueName = materialTemplate->techniqueStateMap.valueByIndex(0);
		}
		
		DVASSERT(techniqueName.IsValid());
		
		SafeRelease(baseTechnique);
		baseTechnique = RenderTechniqueSingleton::Instance()->CreateTechniqueByName(techniqueName);
		
		DVASSERT(baseTechnique);
		//materialSortKey = baseTechnique->GetTechniqueId();
		
		uint32 passCount = baseTechnique->GetPassCount();
		for(uint32 i = 0; i < passCount; ++i)
		{
			RenderTechniquePass* pass = baseTechnique->GetPassByIndex(i);
			UpdateRenderPass(baseTechnique->GetPassName(i), effectiveFlags, pass);
		}
		
		SetTexturesDirty();
        
        SetRenderLayers(RenderLayerManager::Instance()->GetLayerIDMaskBySet(baseTechnique->GetLayersSet()));
	}
	
	void NMaterial::UpdateRenderPass(const FastName& passName,
									 const FastNameSet& instanceDefines,
									 RenderTechniquePass* pass)
	{
		RenderState* parentRenderState = pass->GetRenderState();
		
		RenderPassInstance* passInstance = new RenderPassInstance();
		passInstance->dirtyState = (instancePassRenderStates.count(passName) != 0);
		
		if(passInstance->dirtyState)
		{
			passInstance->renderState.stateHandle = instancePassRenderStates.at(passName);
		}
		else
		{
			passInstance->renderState.stateHandle = parentRenderState->stateHandle;
		}
		
		passInstance->renderState.textureState = InvalidUniqueHandle;
		passInstance->texturesDirty = false;
		passInstance->renderState.shader = pass->CompileShader(instanceDefines);
		passInstance->renderState.scissorRect = parentRenderState->scissorRect;
		passInstance->renderState.renderer = parentRenderState->renderer;
		passInstance->renderState.color = parentRenderState->color;
		
		instancePasses.insert(passName, passInstance);
		
		BuildTextureParamsCache(passInstance);
		BuildActiveUniformsCacheParamsCache(passInstance);
	}
	
	void NMaterial::BuildTextureParamsCache(RenderPassInstance* passInstance)
	{
		Shader* shader = passInstance->renderState.shader;
		
		uint32 uniformCount = shader->GetUniformCount();
		for(uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
		{
			Shader::Uniform * uniform = shader->GetUniform(uniformIndex);
			if(uniform->shaderSemantic == UNKNOWN_SEMANTIC)
			{
				if(uniform->type == Shader::UT_SAMPLER_2D ||
				   uniform->type == Shader::UT_SAMPLER_CUBE)
				{
					ShaderAsset* asset = ShaderCache::Instance()->Get(shader->GetAssetName());
					DVASSERT(asset);
					int32 textureIndex = asset->GetDefaultValue(uniform->name).int32Value;
					passInstance->textureIndexMap.insert(uniform->name, textureIndex);
				}
			}
		}
	}
	
	void NMaterial::BuildActiveUniformsCacheParamsCache(RenderPassInstance* passInstance)
	{
		Shader* shader = passInstance->renderState.shader;
		passInstance->activeUniformsCache.clear();
		
		uint32 uniformCount = shader->GetUniformCount();
		for(uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
		{
			Shader::Uniform * uniform = shader->GetUniform(uniformIndex);
			
			if((UNKNOWN_SEMANTIC == uniform->shaderSemantic ||
			    PARAM_COLOR == uniform->shaderSemantic) &&
			   (Shader::UT_SAMPLER_2D != uniform->type &&
				Shader::UT_SAMPLER_CUBE != uniform->type)) //TODO: do something with conditional binding
			{
				NMaterialProperty* prop = GetPropertyValue(uniform->name);
				
				UniformCacheEntry entry;
				entry.uniform = uniform;
				entry.index = uniformIndex;
				entry.prop = prop;
				
				passInstance->activeUniformsCache.push_back(entry);
			}
		}
		
		passInstance->activeUniformsCachePtr = NULL;
		passInstance->activeUniformsCacheSize = 0;
		if(passInstance->activeUniformsCache.size())
		{
			passInstance->activeUniformsCachePtr = &passInstance->activeUniformsCache[0];
			passInstance->activeUniformsCacheSize = passInstance->activeUniformsCache.size();
		}
	}

	NMaterial::TextureBucket* NMaterial::GetEffectiveTextureBucket(const FastName& textureFastName) const
	{
		TextureBucket* bucket = NULL;
		const NMaterial* currentMaterial = this;
		while(currentMaterial)
		{
			bucket = currentMaterial->textures.at(textureFastName);
			
			currentMaterial = currentMaterial->parent;
		}
		
		return bucket;
	}
	
	void NMaterial::LoadActiveTextures()
	{
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			RenderPassInstance* pass = it->second;
			
			for(HashMap<FastName, int32>::iterator texIt = pass->textureIndexMap.begin();
				texIt != pass->textureIndexMap.end();
				++texIt)
			{
				GetOrLoadTextureRecursive(texIt->first);
			}
		}
	}
	
	void NMaterial::CleanupUnusedTextures()
	{
		for(HashMap<FastName, TextureBucket*>::iterator it = textures.begin();
			it != textures.end();
			++it)
		{
			if(!IsTextureActive(it->first))
			{
				SafeRelease(it->second->texture);
			}
		}
		
		if(parent)
		{
			parent->CleanupUnusedTextures();
		}
	}
	
	Texture* NMaterial::GetOrLoadTextureRecursive(const FastName& textureName)
	{
		Texture* tex = NULL;
		
		NMaterial* currentMaterial = this;
		while(currentMaterial)
		{
			TextureBucket* bucket = currentMaterial->textures.at(textureName);
			
			if(bucket)
			{
				if(NULL == bucket->texture)
				{
					bucket->texture = Texture::CreateFromFile(bucket->path);
				}
				
				tex = bucket->texture;
				break;
			}
			
			currentMaterial = currentMaterial->parent;
		}
		
		return tex;
	}
	
	bool NMaterial::IsTextureActive(const FastName& textureName) const
	{
		bool active = false;
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			RenderPassInstance* pass = it->second;
			if(pass->textureIndexMap.count(textureName) > 0)
			{
				active = true;
				break;
			}
		}
		
		if(!active)
		{
			size_t childrenCount = children.size();
			for(size_t i = 0; i < childrenCount; ++i)
			{
				active = children[i]->IsTextureActive(textureName);
				if(active)
				{
					break;
				}
			}
		}
		
		return active;
	}
	
	void NMaterial::SetTexturesDirty()
	{
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			RenderPassInstance* pass = it->second;
			pass->texturesDirty = true;
		}
		
		size_t childrenCount = children.size();
		for(size_t i = 0; i < childrenCount; ++i)
		{
			children[i]->SetTexturesDirty();
		}
	}
	
	void NMaterial::PrepareTextureState(RenderPassInstance* passInstance)
	{
		DVASSERT(passInstance);
		
		TextureStateData textureData;
		for(HashMap<FastName, int32>::iterator texIt = passInstance->textureIndexMap.begin();
			texIt != passInstance->textureIndexMap.end();
			++texIt)
		{
			textureData.textures[texIt->second] = GetOrLoadTextureRecursive(texIt->first);
			if(NULL == textureData.textures[texIt->second])
			{
				//VI: this case is mostly for ResEditor
				textureData.textures[texIt->second] = GetStubTexture(texIt->first);
			}
		}
		
		//VI: this strange code ensures we do not release textures prematurely
		//VI: in case if there's such texture state already and it's owned by us only
		UniqueHandle oldHandle = passInstance->renderState.textureState;
		passInstance->renderState.textureState = RenderManager::Instance()->AddTextureStateData(&textureData);
		
		if(InvalidUniqueHandle != oldHandle)
		{
			RenderManager::Instance()->ReleaseTextureStateData(oldHandle);
		}
		
		passInstance->texturesDirty = false;
	}
	
	Texture* NMaterial::GetStubTexture(const FastName& uniformName)
	{
		Texture* stubTex = NULL;
		
		if(NMaterial::TEXTURE_CUBEMAP == uniformName)
		{
			if(NULL == stubCubemapTexture)
			{
				stubCubemapTexture = Texture::CreatePink(Texture::TEXTURE_CUBE);
			}
			
			stubTex = stubCubemapTexture;
		}
		else
		{
			if(NULL == stub2dTexture)
			{
				stub2dTexture = Texture::CreatePink(Texture::TEXTURE_2D);
			}
			
			stubTex = stub2dTexture;
		}
		
		return stubTex;
	}
	
	void NMaterial::BindMaterialTechnique(const FastName & passName, Camera* camera)
	{
		if(activePassName != passName)
		{
			activePassName = passName;
			activeRenderPass = baseTechnique->GetPassByName(passName);
			activePassInstance = instancePasses.at(passName);
			
			DVASSERT(activeRenderPass);
			DVASSERT(activePassInstance);
		}
		
		//VI: this call is temporary solution. It will be removed once autobind system and lighting system ready
		SetupPerFrameProperties(camera);
		
		BindMaterialTextures(activePassInstance);
		
		RenderManager::Instance()->FlushState(&activePassInstance->renderState);
		
		BindMaterialProperties(activePassInstance);
	}
	
	void NMaterial::SetupPerFrameProperties(Camera* camera)
	{
		if(camera && IsDynamicLit() && lights[0])
		{
			const Matrix4 & matrix = camera->GetMatrix();
			Vector3 lightPosition0InCameraSpace = lights[0]->GetPosition() * matrix;
			
			SetPropertyValue(NMaterial::PARAM_LIGHT_POSITION0, Shader::UT_FLOAT_VEC3, 1, lightPosition0InCameraSpace.data);
		}
	}

	void NMaterial::BindMaterialTextures(RenderPassInstance* passInstance)
	{
		if(passInstance->texturesDirty)
		{
			PrepareTextureState(passInstance);
		}
	}
	
	void NMaterial::BindMaterialProperties(RenderPassInstance* passInstance)
	{
		Shader* shader = passInstance->renderState.shader;
		for(size_t i = 0; i < passInstance->activeUniformsCacheSize; ++i)
		{
			UniformCacheEntry& uniformEntry = passInstance->activeUniformsCachePtr[i];
			Shader::Uniform* uniform = uniformEntry.uniform;
			
			if(uniformEntry.prop)
			{
                RENDERER_UPDATE_STATS(materialParamUniformBindCount++);
				shader->SetUniformValueByUniform(uniform,
												 uniform->type,
												 uniform->size,
												 uniformEntry.prop->data);
			}
		}
	}
	
	void NMaterial::OnMaterialPropertyAdded(const FastName& propName)
	{
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			RenderPassInstance* pass = it->second;
			if(pass->activeUniformsCachePtr)
			{
				for(size_t i = 0; i < pass->activeUniformsCacheSize; ++i)
				{
					UniformCacheEntry& cacheEntry = pass->activeUniformsCachePtr[i];
					if(cacheEntry.uniform->name == propName)
					{
						pass->activeUniformsCachePtr[i].prop = GetPropertyValue(propName);
						break;
					}
				}
			}
		}
		
		this->Retain();
		
		size_t childrenCount = children.size();
		for(size_t i = 0; i< childrenCount; ++i)
		{
			children[i]->OnMaterialPropertyAdded(propName);
		}
		
		this->Release();
	}
	
	void NMaterial::OnMaterialPropertyRemoved(const FastName& propName)
	{
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			RenderPassInstance* pass = it->second;
			if(pass->activeUniformsCachePtr)
			{
				for(size_t i = 0; i < pass->activeUniformsCacheSize; ++i)
				{
					if(pass->activeUniformsCachePtr[i].uniform->name == propName)
					{
						pass->activeUniformsCachePtr[i].prop = GetPropertyValue(propName);
						
						break;
					}
				}
			}
		}
		
		this->Retain();
		
		size_t childrenCount = children.size();
		for(size_t i = 0; i< childrenCount; ++i)
		{
			children[i]->OnMaterialPropertyRemoved(propName);
		}
		
		this->Release();
	}
	
	void NMaterial::Draw(PolygonGroup * polygonGroup)
	{
		// TODO: Remove support of OpenGL ES 1.0 from attach render data
		RenderManager::Instance()->SetRenderData(polygonGroup->renderDataObject);
		RenderManager::Instance()->AttachRenderData();
		
		//Logger::FrameworkDebug("[Material::Draw] %s", baseTechnique->GetName().c_str());
		
		// TODO: rethink this code
		if(polygonGroup->renderDataObject->GetIndexBufferID() != 0)
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, polygonGroup->indexCount, EIF_16, 0);
		}
		else
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, polygonGroup->indexCount, EIF_16, polygonGroup->indexArray);
		}
	}
	
	void NMaterial::Draw(RenderDataObject* renderData, uint16* indices, uint16 indexCount)
	{
		DVASSERT(renderData);
		
		RenderManager::Instance()->SetRenderData(renderData);
		RenderManager::Instance()->AttachRenderData();
		
		//Logger::FrameworkDebug("[Material::Draw] %s", baseTechnique->GetName().c_str());
		
		// TODO: rethink this code
		if(renderData->GetIndexBufferID() != 0)
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, renderData->indexCount, EIF_16, 0);
		}
		else
		{
			if(renderData->indexCount)
			{
				RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, renderData->indexCount, EIF_16, renderData->indices);
			}
			else
			{
				RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, indexCount, EIF_16, indices);
			}
		}
	}
	
	void NMaterial::SetLight(uint32 index, Light * light, bool forceUpdate)
	{
		if(NMaterial::MATERIALTYPE_INSTANCE == materialType)
		{
			if(parent)
			{
				parent->SetLight(index, light, forceUpdate);
			}
			else
			{
				SetLightInternal(index, light, forceUpdate);
			}
		}
		else if(NMaterial::MATERIALTYPE_MATERIAL == materialType)
		{
			SetLightInternal(index, light, forceUpdate);

			for(size_t i = 0; i < children.size(); ++i)
			{
				children[i]->SetLightInternal(index, light, forceUpdate);
			}
		}
	}
	
	void NMaterial::SetLightInternal(int index, Light* light, bool forceUpdate)
	{
		bool changed = forceUpdate || (light != lights[index]);
		lights[index] = light;
		
		if(changed && materialDynamicLit)
		{
			UpdateLightingProperties(lights[0]);
		}
	}
	
	void NMaterial::UpdateLightingProperties(Light* light)
	{
		NMaterialProperty* propAmbientColor = GetMaterialProperty(NMaterial::PARAM_PROP_AMBIENT_COLOR);
		if(propAmbientColor)
		{
			Color lightAmbientColor = (light) ? light->GetAmbientColor() : Color(0, 0, 0, 0);
			Color materialAmbientColor = *(Color*) propAmbientColor->data;
			materialAmbientColor = materialAmbientColor * lightAmbientColor;
			SetPropertyValue(NMaterial::PARAM_LIGHT_AMBIENT_COLOR, Shader::UT_FLOAT_VEC3, 1, &materialAmbientColor);
		}
		
		NMaterialProperty* propDiffuseColor = GetMaterialProperty(NMaterial::PARAM_PROP_DIFFUSE_COLOR);
		if(propDiffuseColor)
		{
			Color lightDiffuseColor = (light) ? light->GetDiffuseColor() : Color(0, 0, 0, 0);
			Color materialDiffuseColor = *(Color*) propDiffuseColor->data;
			materialDiffuseColor = materialDiffuseColor * lightDiffuseColor;
			SetPropertyValue(NMaterial::PARAM_LIGHT_DIFFUSE_COLOR, Shader::UT_FLOAT_VEC3, 1, &materialDiffuseColor);
		}
		
		NMaterialProperty* propSpecularColor = GetMaterialProperty(NMaterial::PARAM_PROP_SPECULAR_COLOR);
		if(propSpecularColor)
		{
			Color lightSpecularColor = (light) ? light->GetSpecularColor() : Color(0, 0, 0, 0);
			Color materialSpecularColor = *(Color*) propSpecularColor->data;
			materialSpecularColor = materialSpecularColor * lightSpecularColor;
			SetPropertyValue(NMaterial::PARAM_LIGHT_SPECULAR_COLOR, Shader::UT_FLOAT_VEC3, 1, &materialSpecularColor);
		}
		
		float32 intensity = (light) ? light->GetIntensity() : 0;
        intensity = 2.0f;
		SetPropertyValue(NMaterial::PARAM_LIGHT_INTENSITY0, Shader::UT_FLOAT, 1, &intensity);
	}
	
	bool NMaterial::IsLightingProperty(const FastName& propName) const
	{
		return (NMaterial::PARAM_PROP_AMBIENT_COLOR == propName ||
				NMaterial::PARAM_PROP_DIFFUSE_COLOR == propName ||
				NMaterial::PARAM_PROP_SPECULAR_COLOR == propName);
	}
	
	const RenderStateData* NMaterial::GetRenderState(const FastName& passName) const
	{
		RenderPassInstance* pass = instancePasses.at(passName);
		DVASSERT(pass);
		
		const RenderStateData* state = NULL;
		if(pass)
		{
			state = RenderManager::Instance()->GetRenderStateData(pass->renderState.stateHandle);
		}
		
		return state;
	}
	
	void NMaterial::SubclassRenderState(const FastName& passName, RenderStateData* newState)
	{
		DVASSERT(newState);
		
		RenderPassInstance* pass = instancePasses.at(passName);
		DVASSERT(pass);

		if(pass)
		{
			DVASSERT(!pass->dirtyState || (pass->renderState.stateHandle == instancePassRenderStates.at(passName)));
			
			if(pass->dirtyState)
			{
				RenderManager::Instance()->ReleaseRenderStateData(pass->renderState.stateHandle);
			}
			
			pass->renderState.stateHandle = RenderManager::Instance()->AddRenderStateData(newState);
			pass->dirtyState = true;
			
			instancePassRenderStates.insert(passName, pass->renderState.stateHandle);
		}
	}
	
	void NMaterial::SubclassRenderState(RenderStateData* newState)
	{
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			SubclassRenderState(it->first, newState);
		}
	}
	
	void NMaterial::UpdateShaderWithFlags(bool updateChildren)
	{
		DVASSERT(baseTechnique);
		
		if(baseTechnique)
		{
			FastNameSet effectiveFlags(16);
			BuildEffectiveFlagSet(effectiveFlags);
			
			for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
				it != instancePasses.end();
				++it)
			{
				RenderPassInstance* pass = it->second;
				RenderTechniquePass* techniquePass = baseTechnique->GetPassByName(it->first);
				
				pass->renderState.shader = techniquePass->CompileShader(effectiveFlags);
				BuildActiveUniformsCacheParamsCache(pass);
			}
            
            if(updateChildren)
            {
                this->Retain();
                
                size_t childrenCount = children.size();
                for(size_t i = 0; i < childrenCount; ++i)
                {
                    children[i]->OnParentFlagsChanged();
                }
				
                this->Release();
            }
		}
	}
	
	//VI: creates material of type MATERIALTYPE_INSTANCE
	//VI: These methods DO NOT add newly created materials to the material system
	NMaterial* NMaterial::CreateMaterialInstance()
	{
		static int32 instanceCounter = 0;
		instanceCounter++;
		
		NMaterial* mat = new NMaterial();
		mat->SetMaterialType(NMaterial::MATERIALTYPE_INSTANCE);
		mat->SetMaterialKey((NMaterial::NMaterialKey)mat);
		mat->SetMaterialName(FastName(Format("Instance-%d", instanceCounter)));
		mat->SetName(mat->GetMaterialName().c_str());
		
		return mat;
	}
	
	//VI: creates material of type MATERIALTYPE_MATERIAL
	//VI: These methods DO NOT add newly created materials to the material system
	NMaterial* NMaterial::CreateMaterial(const FastName& materialName,
										 const FastName& templateName,
										 const FastName& defaultQuality)
	{
		NMaterial* mat = new NMaterial();
		mat->SetMaterialType(NMaterial::MATERIALTYPE_MATERIAL);
		mat->SetMaterialKey((NMaterial::NMaterialKey)mat); //this value may be temporary
		mat->SetMaterialName(materialName);
		mat->SetName(mat->GetMaterialName().c_str());
		
		const NMaterialTemplate* matTemplate = NMaterialTemplateCache::Instance()->Get(templateName);
		DVASSERT(matTemplate);
		mat->SetMaterialTemplate(matTemplate, defaultQuality);
		
		return mat;
	}

	//VI: creates material of type MATERIALTYPE_INSTANCE
	//VI: These methods DO NOT add newly created materials to the material system
	NMaterial* NMaterial::CreateMaterialInstance(const FastName& materialName,
												 const FastName& templateName,
												 const FastName& defaultQuality)
	{
		NMaterial* parentMat = CreateMaterial(materialName, templateName, defaultQuality);
		
		NMaterial* mat = CreateMaterialInstance();
		mat->SetParent(parentMat);
		
		SafeRelease(parentMat);
		
		return mat;
	}

	bool NMaterial::IsRuntimeFlag(const FastName& flagName)
	{
		bool result = false;
		for(size_t i = 0; i < COUNT_OF(RUNTIME_ONLY_FLAGS); ++i)
		{
			if(RUNTIME_ONLY_FLAGS[i] == flagName)
			{
				result = true;
				break;
			}
		}
		
		return result;
	}
	
	//////////////////////////////////////////////////////////////////////////////////////
	
	void NMaterialHelper::EnableStateFlags(const FastName& passName,
											 NMaterial* target,
											 uint32 stateFlags)
	{
		DVASSERT(target);
		
		const RenderStateData* currentData = target->GetRenderState(passName);
		RenderStateData newData;
		memcpy(&newData, currentData, sizeof(RenderStateData));
		
		newData.state = newData.state | stateFlags;
		
		target->SubclassRenderState(passName, &newData);
	}
	
	void NMaterialHelper::DisableStateFlags(const FastName& passName,
										   NMaterial* target,
										   uint32 stateFlags)
	{
		DVASSERT(target);
		
		const RenderStateData* currentData = target->GetRenderState(passName);
		RenderStateData newData;
		memcpy(&newData, currentData, sizeof(RenderStateData));
		
		newData.state = newData.state & ~stateFlags;
		
		target->SubclassRenderState(passName, &newData);
	}
	
	void NMaterialHelper::SetBlendMode(const FastName& passName,
											 NMaterial* target,
											 eBlendMode src,
											 eBlendMode dst)
	{
        DVASSERT(target);
        
		const RenderStateData* currentData = target->GetRenderState(passName);
		RenderStateData newData;
		memcpy(&newData, currentData, sizeof(RenderStateData));
		
		newData.sourceFactor = src;
		newData.destFactor = dst;
		
		target->SubclassRenderState(passName, &newData);
	}
	
	void NMaterialHelper::SwitchTemplate(NMaterial* material,
										 const FastName& templateName)
	{
		const NMaterialTemplate* matTemplate = NMaterialTemplateCache::Instance()->Get(templateName);
		DVASSERT(matTemplate);
		
		material->SetMaterialTemplate(matTemplate, material->currentQuality);
	}
	
	Texture* NMaterialHelper::GetEffectiveTexture(const FastName& textureName, NMaterial* mat)
	{
		DVASSERT(mat);
		
		NMaterial::TextureBucket* bucket = mat->GetEffectiveTextureBucket(textureName);
		return (bucket) ? bucket->texture : NULL;
	}
	
	bool NMaterialHelper::IsAlphatest(const FastName& passName, NMaterial* mat)
	{
		DVASSERT(mat);
		DVASSERT(mat->baseTechnique);
		
		bool result = false;
		if(mat->baseTechnique)
		{
			result = (mat->baseTechnique->GetLayersSet().count(DAVA::LAYER_ALPHA_TEST_LAYER) > 0);
		}
		
		return result;
	}
	
	bool NMaterialHelper::IsTwoSided(const FastName& passName, NMaterial* mat)
	{
		DVASSERT(mat);
		
		bool result = false;
		const RenderStateData* currentData = mat->GetRenderState(passName);
		
		result = ((currentData->state & RenderStateData::STATE_CULL) == 0);
		
		return result;
	}

    void NMaterialHelper::SetFillMode(const FastName& passName,
                                    NMaterial* mat,
                                    eFillMode fillMode)
    {
        DVASSERT(mat);
        
        const RenderStateData* currentData = mat->GetRenderState(passName);
        RenderStateData newData;
		memcpy(&newData, currentData, sizeof(RenderStateData));
		
		newData.fillMode = fillMode;
		
		mat->SubclassRenderState(passName, &newData);
    }
    
    eFillMode NMaterialHelper::GetFillMode(const FastName& passName, NMaterial* mat)
    {
        DVASSERT(mat);
        
        const RenderStateData* currentData = mat->GetRenderState(passName);
        return currentData->fillMode;
    }
        
	///////////////////////////////////////////////////////////////////////////
	///// NMaterialState::NMaterialStateDynamicTexturesInsp implementation

	const FastNameMap<NMaterial::NMaterialStateDynamicTexturesInsp::PropData>* NMaterial::NMaterialStateDynamicTexturesInsp::FindMaterialTextures(NMaterial *state) const
	{
		static FastNameMap<PropData> staticData;

		staticData.clear();

		NMaterial *parent = state;
		int source = PropData::SOURCE_SELF;

		// properties chain data
		while(NULL != parent)
		{
			HashMap<FastName, TextureBucket*>::iterator it = parent->textures.begin();
			HashMap<FastName, TextureBucket*>::iterator end = parent->textures.end();

			for(; it != end; ++it)
			{
				if(0 == staticData.count(it->first))
				{
					PropData data;
					TextureBucket *bucket = it->second;

					data.source |= source;
					data.path = bucket->path;

					staticData.Insert(it->first, data);
				}
			}

			parent = parent->GetParent();
			source = PropData::SOURCE_PARENT;
		}


		// shader data
		source = PropData::SOURCE_SHADER;
		if(state->instancePasses.size() > 0)
		{
			HashMap<FastName, RenderPassInstance*>::iterator it = state->instancePasses.begin();
			HashMap<FastName, RenderPassInstance*>::iterator end = state->instancePasses.end();

			for(; it != end; ++it)
			{
				Shader *shader = it->second->renderState.shader;
				if(NULL != shader)
				{
					int32 uniformCount = shader->GetUniformCount();
					for(int32 i = 0; i < uniformCount; ++i)
					{
						Shader::Uniform *uniform = shader->GetUniform(i);
						if( uniform->type == Shader::UT_SAMPLER_2D ||
							uniform->type == Shader::UT_SAMPLER_CUBE) // is texture
						{
							FastName propName = uniform->name;

							if(!staticData.count(propName))
							{
								PropData data;

								data.path = FilePath();
								data.source |= source;

								staticData.Insert(propName, data);
							}
							else
							{
								staticData[propName].source |= source;
							}
						}
					}
				}
			}
		}

		return &staticData;
	}

	Vector<FastName> NMaterial::NMaterialStateDynamicTexturesInsp::MembersList(void *object) const
	{
		Vector<FastName> ret;

		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);

		const FastNameMap<NMaterial::NMaterialStateDynamicTexturesInsp::PropData>* textures = FindMaterialTextures(state);

		FastNameMap<NMaterial::NMaterialStateDynamicTexturesInsp::PropData>::iterator it = textures->begin();
		FastNameMap<NMaterial::NMaterialStateDynamicTexturesInsp::PropData>::iterator end = textures->end();

		ret.reserve(textures->size());
		while(it != end)
		{
			ret.push_back(it->first);
			++it;
		}

		return ret;
	}

	InspDesc NMaterial::NMaterialStateDynamicTexturesInsp::MemberDesc(void *object, const FastName &texture) const
	{
		return InspDesc(texture.c_str());
	}
	
	VariantType NMaterial::NMaterialStateDynamicTexturesInsp::MemberAliasGet(void *object, const FastName &texture) const
	{
		VariantType ret;

		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);

		const FastNameMap<NMaterial::NMaterialStateDynamicTexturesInsp::PropData>* textures = FindMaterialTextures(state);
		if(textures->count(texture))
		{
			ret.SetFilePath(textures->at(texture).path);
		}

		return ret;
	}

	VariantType NMaterial::NMaterialStateDynamicTexturesInsp::MemberValueGet(void *object, const FastName &texture) const
	{
		VariantType ret;

		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);

		const FastNameMap<NMaterial::NMaterialStateDynamicTexturesInsp::PropData>* textures = FindMaterialTextures(state);
		if(textures->count(texture))
		{
			if(textures->at(texture).source & PropData::SOURCE_SELF)
			{
				ret.SetFilePath(textures->at(texture).path);
			}
		}

		return ret;
	}

	void NMaterial::NMaterialStateDynamicTexturesInsp::MemberValueSet(void *object, const FastName &texture, const VariantType &value)
	{
		VariantType ret;
		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);

		const FastNameMap<NMaterial::NMaterialStateDynamicTexturesInsp::PropData>* textures = FindMaterialTextures(state);
		if(textures->count(texture))
		{
			if(value.type == VariantType::TYPE_NONE)
			{
				if(state->textures.count(texture) > 0)
				{
					state->RemoveTexture(texture);
				}
			}
			else
			{
				state->SetTexture(texture, value.AsFilePath());
			}
		}
	}
	
	int NMaterial::NMaterialStateDynamicTexturesInsp::MemberFlags(void *object, const FastName &texture) const
	{
		int flags = 0;

		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);

		const FastNameMap<NMaterial::NMaterialStateDynamicTexturesInsp::PropData>* textures = FindMaterialTextures(state);
		if(textures->count(texture))
		{
			const PropData &propData = textures->at(texture);

			if(propData.source & PropData::SOURCE_SELF)
			{
				flags |= I_EDIT;
			}

			if(propData.source & PropData::SOURCE_PARENT)
			{
				flags |= I_VIEW;
			}

			if(propData.source & PropData::SOURCE_SHADER)
			{
				flags |= I_SAVE;
			}
		}

		return flags;
	}

	///////////////////////////////////////////////////////////////////////////
	///// NMaterialState::NMaterialStateDynamicPropertiesInsp implementation
	const FastNameMap<NMaterial::NMaterialStateDynamicPropertiesInsp::PropData>* NMaterial::NMaterialStateDynamicPropertiesInsp::FindMaterialProperties(NMaterial *state) const
	{
		static FastNameMap<PropData> staticData;
		
		staticData.clear();
		NMaterial *parent = state;
		int source = PropData::SOURCE_SELF;
		
		// properties chain data
		while(NULL != parent)
		{
			HashMap<FastName, NMaterialProperty* >::iterator it = parent->materialProperties.begin();
			HashMap<FastName, NMaterialProperty* >::iterator end = parent->materialProperties.end();
			
			for(; it != end; ++it)
			{
				FastName propName = it->first;

				// don't add some properties with light settings, they should be hidden for user
				if(propName != NMaterial::PARAM_LIGHT_AMBIENT_COLOR &&
					propName != NMaterial::PARAM_LIGHT_DIFFUSE_COLOR &&
					propName != NMaterial::PARAM_LIGHT_SPECULAR_COLOR)
				{
					if(0 == staticData.count(it->first))
					{
						PropData data;
						data.data = it->second->data;
						data.size = it->second->size;
						data.type = it->second->type;
						data.source |= source;
					
						staticData.Insert(propName, data);
						//printf("insert %s, %d\n", propName.c_str(), source);
					}
					else
					{
						staticData[propName].source |= source;
						//printf("update %s, %d\n", propName.c_str(), source);
					}
				}
			}
			
			parent = parent->GetParent();
			source = PropData::SOURCE_PARENT;
		}
		

		// shader data
		source = PropData::SOURCE_SHADER;
		if(state->instancePasses.size() > 0)
		{
			HashMap<FastName, RenderPassInstance*>::iterator it = state->instancePasses.begin();
			HashMap<FastName, RenderPassInstance*>::iterator end = state->instancePasses.end();

			for(; it != end; ++it)
			{
				Shader *shader = it->second->renderState.shader;
				if(NULL != shader)
				{
					int32 uniformCount = shader->GetUniformCount();
					for(int32 i = 0; i < uniformCount; ++i)
					{
						Shader::Uniform *uniform = shader->GetUniform(i);
						eShaderSemantic shaderSemantic = uniform->shaderSemantic;
						if( //!Shader::IsAutobindUniform(uniform->id) && // isn't auto-bind
							// we can't use IsAutobindUniform, because we need color to change
							// so this is copy from IsAutobindUniform with color excluded -->
							!(shaderSemantic == PARAM_WORLD_VIEW_PROJ ||
							shaderSemantic == PARAM_WORLD_VIEW ||
							shaderSemantic == PARAM_PROJ ||
							shaderSemantic == PARAM_NORMAL ||
							shaderSemantic == PARAM_GLOBAL_TIME ||
							shaderSemantic == PARAM_WORLD_TRANSLATE ||
							shaderSemantic == PARAM_WORLD_SCALE)
							// <--
							&&
							uniform->type != Shader::UT_SAMPLER_2D && uniform->type != Shader::UT_SAMPLER_CUBE) // isn't texture
						{
							FastName propName = uniform->name;

							// redefine some shader properties names
							if(propName == NMaterial::PARAM_LIGHT_AMBIENT_COLOR) propName = NMaterial::PARAM_PROP_AMBIENT_COLOR;
							else if(propName == NMaterial::PARAM_LIGHT_DIFFUSE_COLOR) propName = NMaterial::PARAM_PROP_DIFFUSE_COLOR;
							else if(propName == NMaterial::PARAM_LIGHT_SPECULAR_COLOR) propName = NMaterial::PARAM_PROP_SPECULAR_COLOR;

							if(!staticData.count(propName))
							{
								PropData data;
								data.data = NULL;
								data.size = uniform->size;
								data.type = uniform->type;
								data.source |= source;

								staticData.Insert(propName, data);
								//printf("insert %s, %d\n", propName.c_str(), source);
							}
							else
							{
								staticData[propName].source |= source;
								//printf("update %s, %d\n", propName.c_str(), source);
							}
						}
					}
				}
			}
		}
		
		return &staticData;
	}
	
	Vector<FastName> NMaterial::NMaterialStateDynamicPropertiesInsp::MembersList(void *object) const
	{
		Vector<FastName> ret;

		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);
		
		const FastNameMap<NMaterial::NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state);
		
		FastNameMap<NMaterial::NMaterialStateDynamicPropertiesInsp::PropData>::iterator it = members->begin();
		FastNameMap<NMaterial::NMaterialStateDynamicPropertiesInsp::PropData>::iterator end = members->end();

		ret.reserve(members->size());
		while(it != end)
		{
			ret.push_back(it->first);
			++it;
		}

		return ret;
	}
	
	InspDesc NMaterial::NMaterialStateDynamicPropertiesInsp::MemberDesc(void *object, const FastName &member) const
	{
		return InspDesc(member.c_str());
	}
	
	int NMaterial::NMaterialStateDynamicPropertiesInsp::MemberFlags(void *object, const FastName &member) const
	{
		int flags = 0;
		
		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);
		
		const FastNameMap<NMaterial::NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state);
		if(members->count(member))
		{
			const PropData &propData = members->at(member);

			if(propData.source & PropData::SOURCE_SELF)
			{
				flags |= I_EDIT;
			}

			if(propData.source & PropData::SOURCE_PARENT)
			{
				flags |= I_VIEW;
			}

			if(propData.source & PropData::SOURCE_SHADER)
			{
				flags |= I_SAVE;
			}
		}
		
		return flags;
	}
	
	VariantType NMaterial::NMaterialStateDynamicPropertiesInsp::MemberAliasGet(void *object, const FastName &member) const
	{
		VariantType ret;

		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);

		const FastNameMap<NMaterial::NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state);
		if(members->count(member))
		{
			const PropData &prop = members->at(member);
			ret = getVariant(member, prop);
		}

		return ret;
	}

	VariantType NMaterial::NMaterialStateDynamicPropertiesInsp::MemberValueGet(void *object, const FastName &member) const
	{
		VariantType ret;
		
		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);
		
		const FastNameMap<NMaterial::NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state);
		if(members->count(member))
		{
			const PropData &prop = members->at(member);
			if(prop.source & PropData::SOURCE_SELF)
			{
				ret = getVariant(member, prop);
			}
		}
		
		return ret;
	}

	VariantType NMaterial::NMaterialStateDynamicPropertiesInsp::getVariant(const FastName &propName, const PropData &prop) const
	{
		VariantType ret;

		// self or parent property
		if(NULL != prop.data)
		{
			switch(prop.type)
			{
				case Shader::UT_FLOAT:
					ret.SetFloat(*(float32*) prop.data);
					break;

				case Shader::UT_FLOAT_VEC2:
					ret.SetVector2(*(Vector2*) prop.data);
					break;

				case Shader::UT_FLOAT_VEC3:
					if(isColor(propName))
					{
						float32 *color = (float32*) prop.data;
						ret.SetColor(Color(color[0], color[1], color[2], 1.0));
					}
					else
					{
						ret.SetVector3(*(Vector3*) prop.data);
					}
					break;

				case Shader::UT_FLOAT_VEC4:
					if(isColor(propName))
					{
						float32 *color = (float32*) prop.data;
						ret.SetColor(Color(color[0], color[1], color[2], color[3]));
					}
					else
					{
						ret.SetVector4(*(Vector4*) prop.data);
					}
					break;

				case Shader::UT_INT:
					ret.SetInt32(*(int32*) prop.data);
					break;

				case Shader::UT_INT_VEC2:
				case Shader::UT_INT_VEC3:
				case Shader::UT_INT_VEC4:
					DVASSERT(false);
					//TODO: add a way to set int[]
					break;

				case Shader::UT_BOOL:
					ret.SetBool((*(int32*) prop.data) != 0);
					break;

				case Shader::UT_BOOL_VEC2:
				case Shader::UT_BOOL_VEC3:
				case Shader::UT_BOOL_VEC4:
					DVASSERT(false);
					//TODO: add a way to set bool[]
					break;

				case Shader::UT_FLOAT_MAT2:
					ret.SetMatrix2(*(Matrix2*) prop.data);
					break;

				case Shader::UT_FLOAT_MAT3:
					ret.SetMatrix3(*(Matrix3*) prop.data);
					break;

				case Shader::UT_FLOAT_MAT4:
					ret.SetMatrix4(*(Matrix4*) prop.data);
					break;

				case Shader::UT_SAMPLER_2D:
					ret.SetInt32(*(int32*) prop.data);
					break;

				case Shader::UT_SAMPLER_CUBE:
					ret.SetInt32(*(int32*) prop.data);
					break;

				default:
					DVASSERT(false);
					break;
			}

		}
		// shader property that is not set in self or parent properties
		else
		{
			switch(prop.type)
			{
				case Shader::UT_FLOAT: ret.SetFloat(0);	break;
				case Shader::UT_FLOAT_VEC2:	ret.SetVector2(DAVA::Vector2()); break;
				case Shader::UT_FLOAT_VEC3:	isColor(propName) ? ret.SetColor(DAVA::Color()) : ret.SetVector3(DAVA::Vector3()); break;
				case Shader::UT_FLOAT_VEC4:	isColor(propName) ? ret.SetColor(DAVA::Color()) : ret.SetVector4(DAVA::Vector4()); break;
				case Shader::UT_INT: ret.SetInt32(0); break;
				case Shader::UT_BOOL: ret.SetBool(false); break;
				case Shader::UT_FLOAT_MAT2:	ret.SetMatrix2(DAVA::Matrix2()); break;
				case Shader::UT_FLOAT_MAT3:	ret.SetMatrix3(DAVA::Matrix3()); break;
				case Shader::UT_FLOAT_MAT4:	ret.SetMatrix4(DAVA::Matrix4()); break;
				case Shader::UT_SAMPLER_2D:	ret.SetInt32(0); break;
				case Shader::UT_SAMPLER_CUBE: ret.SetInt32(0); break;

				case Shader::UT_INT_VEC2:
				case Shader::UT_INT_VEC3:
				case Shader::UT_INT_VEC4:
					DVASSERT(false);
					//TODO: add a way to set int[]
					break;

				case Shader::UT_BOOL_VEC2:
				case Shader::UT_BOOL_VEC3:
				case Shader::UT_BOOL_VEC4:
					DVASSERT(false);
					//TODO: add a way to set bool[]
					break;

				default:
					DVASSERT(false);
					break;
			}
		}

		return ret;
	}

	bool NMaterial::NMaterialStateDynamicPropertiesInsp::isColor(const FastName &propName) const
	{
		return (propName == NMaterial::PARAM_PROP_AMBIENT_COLOR ||
				propName == NMaterial::PARAM_PROP_DIFFUSE_COLOR ||
				propName == NMaterial::PARAM_PROP_SPECULAR_COLOR ||
				propName == NMaterial::PARAM_FOG_COLOR ||
				propName == NMaterial::PARAM_FLAT_COLOR ||
                propName == NMaterial::PARAM_SPEED_TREE_LEAF_COLOR_MUL
                );
	}
	
	void NMaterial::NMaterialStateDynamicPropertiesInsp::MemberValueSet(void *object, const FastName &member, const VariantType &value)
	{
		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);

		const FastNameMap<NMaterial::NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state);
		if(members->count(member))
		{
			PropData prop = members->at(member);
			int propSize = prop.size;
			Shader::eUniformType propType = prop.type;
		
			if(value.GetType() == VariantType::TYPE_NONE)
			{
				// empty variant value should remove this property
				state->RemoveMaterialProperty(member);
			}
			else
			{
				switch(prop.type)
				{
					case Shader::UT_FLOAT:
					{
						float32 val = value.AsFloat();
						state->SetPropertyValue(member, propType, propSize, &val);
					}
						break;
				
					case Shader::UT_FLOAT_VEC2:
					{
						const Vector2& val = value.AsVector2();
						state->SetPropertyValue(member, propType, propSize, &val);
					}
						break;
				
					case Shader::UT_FLOAT_VEC3:
					{
						Vector3 val;
				
						if(isColor(member))
						{
							Color c = value.AsColor();
							val = Vector3(c.r, c.g, c.b);
						}
						else
						{
							val = value.AsVector3();
						}
				
						state->SetPropertyValue(member, propType, propSize, &val);
					}
						break;
				
					case Shader::UT_FLOAT_VEC4:
					{
						Vector4 val;
				
						if(isColor(member))
						{
							Color c = value.AsColor();
							val = Vector4(c.r, c.g, c.b, c.a);
						}
						else
						{
							val = value.AsVector4();
						}
				
						state->SetPropertyValue(member, propType, propSize, &val);
					}
						break;
				
					case Shader::UT_INT:
					{
						int32 val = value.AsInt32();
						state->SetPropertyValue(member, propType, propSize, &val);
					}
						break;
				
					case Shader::UT_INT_VEC2:
					case Shader::UT_INT_VEC3:
					case Shader::UT_INT_VEC4:
					{
						DVASSERT(false);
						//TODO: add a way to set int[]
					}
						break;
				
					case Shader::UT_BOOL:
					{
						bool val = value.AsBool();
						state->SetPropertyValue(member, propType, propSize, &val);
					}
						break;
				
					case Shader::UT_BOOL_VEC2:
					case Shader::UT_BOOL_VEC3:
					case Shader::UT_BOOL_VEC4:
					{
						DVASSERT(false);
						//TODO: add a way to set bool[]
					}
						break;
				
					case Shader::UT_FLOAT_MAT2:
					{
						const Matrix2& val = value.AsMatrix2();
						state->SetPropertyValue(member, propType, propSize, &val);
					}
						break;
				
					case Shader::UT_FLOAT_MAT3:
					{
						const Matrix3& val = value.AsMatrix3();
						state->SetPropertyValue(member, propType, propSize, &val);
					}
						break;
				
					case Shader::UT_FLOAT_MAT4:
					{
						const Matrix3& val = value.AsMatrix3();
						state->SetPropertyValue(member, propType, propSize, &val);
					}
						break;
				
					case Shader::UT_SAMPLER_2D:
						//VI: samplers are set by config materials
						break;
				
					case Shader::UT_SAMPLER_CUBE:
						//VI: samplers are set by config materials
						break;
				
					default:
						DVASSERT(false);
						break;
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////
	///// NMaterialState::NMaterialStateDynamicTexturesInsp implementation

	Vector<FastName> NMaterial::NMaterialStateDynamicFlagsInsp::MembersList(void *object) const
	{
		static Vector<FastName> ret;

 		if(0 == ret.size())
 		{
			ret.reserve(3);
 			ret.push_back(FLAG_VERTEXFOG);
 			ret.push_back(FLAG_FLATCOLOR);
 			ret.push_back(FLAG_TEXTURESHIFT);
            ret.push_back(FLAG_TEXTURE0_ANIMATION_SHIFT);
 		}
		return ret;
	}

	InspDesc NMaterial::NMaterialStateDynamicFlagsInsp::MemberDesc(void *object, const FastName &member) const
	{
		return InspDesc(member.c_str());
	}

	VariantType NMaterial::NMaterialStateDynamicFlagsInsp::MemberValueGet(void *object, const FastName &member) const
	{
		VariantType ret;
		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);
 
		ret.SetBool(state->IsFlagEffective(member));

		return ret;
	}

	void NMaterial::NMaterialStateDynamicFlagsInsp::MemberValueSet(void *object, const FastName &member, const VariantType &value)
	{
		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);

		NMaterial::eFlagValue newValue = NMaterial::FlagOff;
		if(value.AsBool())
		{
			newValue = NMaterial::FlagOn;
		}

		state->SetFlag(member, newValue);
	}

	int NMaterial::NMaterialStateDynamicFlagsInsp::MemberFlags(void *object, const FastName &member) const
	{
		return I_VIEW | I_EDIT;
	}
};
