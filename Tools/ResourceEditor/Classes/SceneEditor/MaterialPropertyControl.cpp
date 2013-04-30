#include "MaterialPropertyControl.h"
#include "EditorSettings.h"
#include "SceneValidator.h"

#include "ControlsFactory.h"

#include "Qt/Main/QtUtils.h"

static const String textureNames[] = 
{
    "Diffuse texture", 
    "Decal texture",
    "Detail texture",
    "Normal map"
};

static const int32 textureTypes[] = 
{
    Material::TEXTURE_DIFFUSE,
    Material::TEXTURE_DECAL,
    Material::TEXTURE_DETAIL,
    Material::TEXTURE_NORMALMAP
};

MaterialPropertyControl::MaterialPropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
}

MaterialPropertyControl::~MaterialPropertyControl()
{

}

void MaterialPropertyControl::ReadFrom(DataNode * dataNode)
{
	NodesPropertyControl::ReadFrom(dataNode);

    Material *material = dynamic_cast<Material*> (dataNode);
	DVASSERT(material);

    propertyList->AddSection("property.material", GetHeaderState("property.material", true));
    
    propertyList->AddStringProperty("property.material.name", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetStringPropertyValue("property.material.name", material->GetName());
    
    
    Vector<String> materialTypes;
    for (int i = 0; i < Material::MATERIAL_TYPES_COUNT; i++) 
    {
        materialTypes.push_back(Material::GetTypeName((Material::eType)i));
    }

    int32 materialType = material->type;
    propertyList->AddComboProperty("property.material.type", materialTypes);
    propertyList->SetComboPropertyIndex("property.material.type", materialType);

    
    propertyList->AddFilepathProperty(textureNames[ETT_DIFFUSE], TextureDescriptor::GetSupportedTextureExtensions());
    SetFilepathValue(material, ETT_DIFFUSE);
    
    if (    (Material::MATERIAL_UNLIT_TEXTURE_DECAL == materialType)
        ||  (Material::MATERIAL_VERTEX_LIT_DECAL == materialType))
    {
        propertyList->AddFilepathProperty(textureNames[ETT_DECAL], TextureDescriptor::GetSupportedTextureExtensions());
        SetFilepathValue(material, ETT_DECAL);
    }
    
    if (    (Material::MATERIAL_UNLIT_TEXTURE_DETAIL == materialType)
        ||  (Material::MATERIAL_VERTEX_LIT_DETAIL == materialType))
    {
        propertyList->AddFilepathProperty(textureNames[ETT_DETAIL], TextureDescriptor::GetSupportedTextureExtensions());
        SetFilepathValue(material, ETT_DETAIL);
    }
    
    if (    (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE == materialType)
        ||  (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR == materialType)
        ||  (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP == materialType)
		||  (Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP == materialType))
    {
        propertyList->AddFilepathProperty(textureNames[ETT_NORMAL_MAP], TextureDescriptor::GetSupportedTextureExtensions());
        SetFilepathValue(material, ETT_NORMAL_MAP);
    }

    
    propertyList->AddBoolProperty("property.material.isopaque");
    propertyList->SetBoolPropertyValue("property.material.isopaque", material->GetOpaque());
    
    propertyList->AddBoolProperty("property.material.twosided");
    propertyList->SetBoolPropertyValue("property.material.twosided", material->GetTwoSided());

	bool isAlphablend = material->GetAlphablend();
	propertyList->AddBoolProperty("property.material.alphablend");
	propertyList->SetBoolPropertyValue("property.material.alphablend", isAlphablend);
	if(isAlphablend)
	{
		Vector<String> blendTypes;
		for (int i = 0; i < BLEND_MODE_COUNT; i++) 
		{
			blendTypes.push_back(BLEND_MODE_NAMES[i]);
		}
		propertyList->AddComboProperty("property.material.blendSrc", blendTypes);
		propertyList->SetComboPropertyIndex("property.material.blendSrc", material->GetBlendSrc());

		propertyList->AddComboProperty("property.material.blendDst", blendTypes);
		propertyList->SetComboPropertyIndex("property.material.blendDst", material->GetBlendDest());
	}
    
    if (    (Material::MATERIAL_VERTEX_LIT_TEXTURE == materialType) 
        ||  (Material::MATERIAL_VERTEX_LIT_DECAL == materialType)
        ||  (Material::MATERIAL_VERTEX_LIT_DETAIL == materialType)
        ||  (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE == materialType)
        ||  (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR == materialType)
        ||  (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP == materialType))
    {
        propertyList->AddColorProperty("property.material.ambientcolor");
        propertyList->SetColorPropertyValue("property.material.ambientcolor", material->GetAmbientColor());
        
        propertyList->AddColorProperty("property.material.diffusecolor");
        propertyList->SetColorPropertyValue("property.material.diffusecolor", material->GetDiffuseColor());
    }
    
    if (    (Material::MATERIAL_VERTEX_LIT_TEXTURE == materialType)
        ||  (Material::MATERIAL_VERTEX_LIT_DECAL == materialType)
        ||  (Material::MATERIAL_VERTEX_LIT_DETAIL == materialType)
        ||  (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR == materialType)
        ||  (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP == materialType))
    {
        propertyList->AddColorProperty("property.material.specularcolor");
        propertyList->SetColorPropertyValue("property.material.specularcolor", material->GetSpecularColor());
        
        propertyList->AddFloatProperty("property.material.specularshininess");
        propertyList->SetFloatPropertyValue("property.material.specularshininess", material->GetShininess());
    }
    
    {   //FOG
        ControlsFactory::AddFogSubsection(propertyList, material->IsFogEnabled(), material->GetFogDensity(), material->GetFogColor());
    }
}

void MaterialPropertyControl::SetFilepathValue(Material *material, int32 type)
{
    if (material->GetTexture((Material::eTextureLevel)textureTypes[type]))
    {
        propertyList->SetFilepathPropertyValue(textureNames[type], material->GetTextureName((Material::eTextureLevel)textureTypes[type]));
    }
    else 
    {
        propertyList->SetFilepathPropertyValue(textureNames[type], FilePath());
    }

}


void MaterialPropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if("property.material.isopaque" == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetOpaque(newValue);
    }
    else if("property.material.twosided" == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetTwoSided(newValue);
    }
	else if("property.material.alphablend" == forKey)
	{
		Material *material = dynamic_cast<Material *> (currentDataNode);
		material->SetAlphablend(newValue);
		ReadFrom(currentDataNode);
	}
    else if (String("property.material.fogenabled") == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetFog(newValue);
    }

    NodesPropertyControl::OnBoolPropertyChanged(forList, forKey, newValue);
}

void MaterialPropertyControl::OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor)
{
    if("property.material.ambientcolor" == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetAmbientColor(newColor);
    } 
    else if("property.material.diffusecolor" == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetDiffuseColor(newColor);
    }
    else if("property.material.specularcolor" == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetSpecularColor(newColor);
    }
    else if("property.material.fogcolor" == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetFogColor(newColor);
    }

    PropertyListDelegate::OnColorPropertyChanged(forList, forKey, newColor);
}

void MaterialPropertyControl::OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey)
{
    if ("property.material.type" == forKey) 
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetType((Material::eType)newItemIndex);
        
        ReadFrom(currentDataNode);
        
        SceneValidator::Instance()->ValidateSceneAndShowErrors(material->GetScene());
    }
	else if ("property.material.blendSrc" == forKey) 
	{
		Material *material = dynamic_cast<Material *> (currentDataNode);
		material->SetBlendSrc((eBlendMode)newItemIndex);
	}
	else if ("property.material.blendDst" == forKey) 
	{
		Material *material = dynamic_cast<Material *> (currentDataNode);
		material->SetBlendDest((eBlendMode)newItemIndex);
	}

    NodesPropertyControl::OnComboIndexChanged(forList, forKey, newItemIndex, newItemKey);
}

void MaterialPropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if ("property.material.specularshininess" == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetShininess(newValue);
    }
    else if ("property.material.dencity" == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetFogDensity(newValue);
    }

    NodesPropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}

void MaterialPropertyControl::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const FilePath &newValue)
{
	Set<String> errorLog;
    
    if(!newValue.IsEmpty())
    {
        bool isCorrect = SceneValidator::Instance()->ValidateTexturePathname(newValue, errorLog);
        if(!isCorrect)
        {
            ShowErrorDialog(errorLog);
            return;
        }
    }
    
    FilePath descriptorPathname = newValue.IsEmpty() ? FilePath() : TextureDescriptor::GetDescriptorPathname(newValue);
    for (int32 i = 0; i < ME_TEX_COUNT; i++)
    {
        if (forKey == textureNames[i]) 
        {
            Material *material = dynamic_cast<Material *> (currentDataNode);

            material->SetTexture((Material::eTextureLevel)textureTypes[i], descriptorPathname);
            Texture *tx = material->GetTexture((Material::eTextureLevel)textureTypes[i]);
            if(tx)
            {
                if(tx != Texture::GetPinkPlaceholder())
                {
                    SceneValidator::Instance()->ValidateTextureAndShowErrors(tx, material->GetTextureName((Material::eTextureLevel)textureTypes[i]),
                                                                             Format("Material: %s. TextureLevel %d.", material->GetName().c_str(), textureTypes[i]));
                }
            }
            else 
            {
                propertyList->SetFilepathPropertyValue(textureNames[i], FilePath());
            }
            break;
        }
    }
    
    NodesPropertyControl::OnFilepathPropertyChanged(forList, forKey, newValue);
}

void MaterialPropertyControl::OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if("property.material.name" == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetName(newValue);
    }

    NodesPropertyControl::OnStringPropertyChanged(forList, forKey, newValue);
}


