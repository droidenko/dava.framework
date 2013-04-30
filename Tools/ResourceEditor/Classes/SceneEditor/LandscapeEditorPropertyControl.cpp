#include "LandscapeEditorPropertyControl.h"
#include "Qt/Main/QtUtils.h"

//*********************  LandscapeEditorSettings  **********************
LandscapeEditorSettings::LandscapeEditorSettings()
{
    maskSize = 1024;
    ResetAll();
}

void LandscapeEditorSettings::ResetAll()
{
    redMask = false;
    greenMask = false;
    blueMask = false;
    alphaMask = false;
}

//*********************  LandscapeEditorPropertyControl  **********************
LandscapeEditorPropertyControl::LandscapeEditorPropertyControl(const Rect & rect, bool createNodeProperties, eEditorMode mode)
    :	LandscapePropertyControl(rect, createNodeProperties)
{
    settings = new LandscapeEditorSettings();
    settings->redMask = true;
    delegate = NULL;
    editorMode = mode;
}

LandscapeEditorPropertyControl::~LandscapeEditorPropertyControl()
{
    SafeRelease(settings);
}

void LandscapeEditorPropertyControl::Input(DAVA::UIEvent *currentInput)
{
    if(UIEvent::PHASE_KEYCHAR == currentInput->phase)
    {
        if(IsKeyModificatorPressed(DVKEY_CTRL))
        {
            if(DVKEY_1 == currentInput->tid)
            {
                OnTexturePreviewPropertyChanged(propertyList, "landscapeeditor.maskred", true);
            }
            if(DVKEY_2 == currentInput->tid)
            {
                OnTexturePreviewPropertyChanged(propertyList, "landscapeeditor.maskgreen", true);
            }
            if(DVKEY_3 == currentInput->tid)
            {
                OnTexturePreviewPropertyChanged(propertyList, "landscapeeditor.maskblue", true);
            }
            if(DVKEY_4 == currentInput->tid)
            {
                OnTexturePreviewPropertyChanged(propertyList, "landscapeeditor.maskalpha", true);
            }
        }
    }

    LandscapePropertyControl::Input(currentInput);
}

void LandscapeEditorPropertyControl::OnTexturePreviewPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if(HEIGHT_EDITOR_MODE == editorMode)
    {
        LandscapePropertyControl::OnTexturePreviewPropertyChanged(forList, forKey, newValue);
    }
    else if(MASK_EDITOR_MODE == editorMode)
    {
        if("landscapeeditor.maskred" == forKey)
        {
            settings->ResetAll();
            settings->redMask = newValue;
            SetValuesFromSettings();
            if(delegate)
            {
                delegate->LandscapeEditorSettingsChanged(settings);
            }
        }
        else if("landscapeeditor.maskgreen" == forKey)
        {
            settings->ResetAll();
            settings->greenMask = newValue;
            SetValuesFromSettings();
            if(delegate)
            {
                delegate->LandscapeEditorSettingsChanged(settings);
            }
        }
        else if("landscapeeditor.maskblue" == forKey)
        {
            settings->ResetAll();
            settings->blueMask = newValue;
            SetValuesFromSettings();
            if(delegate)
            {
                delegate->LandscapeEditorSettingsChanged(settings);
            }
        }
        else if("landscapeeditor.maskalpha" == forKey)
        {
            settings->ResetAll();
            settings->alphaMask = newValue;
            SetValuesFromSettings();
            if(delegate)
            {
                delegate->LandscapeEditorSettingsChanged(settings);
            }
        }
        else 
        {
            LandscapePropertyControl::OnTexturePreviewPropertyChanged(forList, forKey, newValue);
        }
    }
}



void LandscapeEditorPropertyControl::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const FilePath &newValue)
{
    if(delegate)
    {
        if(     (("property.landscape.texture.tilemask" == forKey))// && (MASK_EDITOR_MODE == editorMode))
            ||  (("property.landscape.heightmap" == forKey) && (HEIGHT_EDITOR_MODE == editorMode)))
        {
            delegate->TextureWillChanged(forKey);
        }
    }
    
    LandscapePropertyControl::OnFilepathPropertyChanged(forList, forKey, newValue);
    
    if(delegate)
    {
        if(     (("property.landscape.texture.tilemask" == forKey))// && (MASK_EDITOR_MODE == editorMode))
           ||  (("property.landscape.heightmap" == forKey) && (HEIGHT_EDITOR_MODE == editorMode)))
        {
            delegate->TextureDidChanged(forKey);
        }
    }
}


void LandscapeEditorPropertyControl::SetValuesFromSettings()
{
    if(MASK_EDITOR_MODE == editorMode)
    {
        //LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
		Landscape *landscape = GetLandscape();
		if (landscape)
        {
			propertyList->SetTexturePreviewPropertyValue("landscapeeditor.maskred", settings->redMask,
														 landscape->GetTexture(Landscape::TEXTURE_TILE0));
			propertyList->SetTexturePreviewPropertyValue("landscapeeditor.maskgreen", settings->greenMask,
														 landscape->GetTexture(Landscape::TEXTURE_TILE1));
			propertyList->SetTexturePreviewPropertyValue("landscapeeditor.maskblue", settings->blueMask,
														 landscape->GetTexture(Landscape::TEXTURE_TILE2));
			propertyList->SetTexturePreviewPropertyValue("landscapeeditor.maskalpha", settings->alphaMask,
														 landscape->GetTexture(Landscape::TEXTURE_TILE3));
		}
    }
}


void LandscapeEditorPropertyControl::ReadFrom(DAVA::Entity *sceneNode)
{
    LandscapePropertyControl::ReadFrom(sceneNode);
    
    if(MASK_EDITOR_MODE == editorMode)
    {
        propertyList->AddSection("landscapeeditor.landscapeeditor", GetHeaderState("landscapeeditor.landscapeeditor", true));
        
        propertyList->AddIntProperty("landscapeeditor.size", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetIntPropertyValue("landscapeeditor.size", settings->maskSize);
        
        propertyList->AddTexturePreviewProperty("landscapeeditor.maskred", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddTexturePreviewProperty("landscapeeditor.maskgreen", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddTexturePreviewProperty("landscapeeditor.maskblue", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddTexturePreviewProperty("landscapeeditor.maskalpha", PropertyList::PROPERTY_IS_EDITABLE);
        
        SetValuesFromSettings();
    }
}

void LandscapeEditorPropertyControl::SetDelegate(LandscapeEditorPropertyControlDelegate *newDelegate)
{
    delegate = newDelegate;
}

LandscapeEditorSettings * LandscapeEditorPropertyControl::Settings()
{
    return settings;
}

