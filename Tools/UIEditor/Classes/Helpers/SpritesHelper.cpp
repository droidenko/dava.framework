//
//  SpritesHelper.cpp
//  UIEditor
//
//  Created by Yuri Coder on 12/23/13.
//
//

#include "SpritesHelper.h"
#include "UIControlStateHelper.h"

#include "Models/HierarchyTreePlatformNode.h"
#include "Models/HierarchyTreeScreenNode.h"

Set<Sprite*> SpritesHelper::EnumerateSprites(const HierarchyTreeNode* rootNode)
{
    Set<Sprite*> resultSprites;
    for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator platformIter = rootNode->GetChildNodes().begin(); platformIter != rootNode->GetChildNodes().end(); ++platformIter)
    {
        const HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>(*platformIter);
        if (!platformNode)
        {
            continue;
        }
        
        for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator screenIter = platformNode->GetChildNodes().begin(); screenIter != platformNode->GetChildNodes().end(); ++screenIter)
        {
            const HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(*screenIter);
            if (!screenNode)
            {
                continue;
            }
            
            for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator controlIter = screenNode->GetChildNodes().begin(); controlIter != screenNode->GetChildNodes().end(); ++controlIter)
            {
                const HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(*controlIter);
                BuildSpritesListRecursive(controlNode, resultSprites);
            }
        }
    }
    
    return resultSprites;
}

void SpritesHelper::BuildSpritesListRecursive(const HierarchyTreeControlNode* controlNode, Set<Sprite*>& sprites)
{
    if (!controlNode)
    {
        return;
    }
    
    UIButton* buttonControl = dynamic_cast<UIButton*>(controlNode->GetUIObject());
    UIStaticText* staticTextControl = dynamic_cast<UIStaticText*>(controlNode->GetUIObject());
    if (buttonControl)
    {
        BuildSpritesList(sprites, buttonControl);
    }
    else if (staticTextControl)
    {
        BuildSpritesList(sprites, staticTextControl);
    }
    else if (controlNode->GetUIObject() && controlNode->GetUIObject()->GetSprite())
    {
        sprites.insert(controlNode->GetUIObject()->GetSprite());
    }

    // Repeat for all children.
    const HierarchyTreeNode::HIERARCHYTREENODESLIST& childNodes = controlNode->GetChildNodes();
    for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = childNodes.begin(); iter != childNodes.end(); iter ++)
    {
        const HierarchyTreeControlNode* childNode = dynamic_cast<const HierarchyTreeControlNode*>(*iter);
        BuildSpritesListRecursive(childNode, sprites);
    }
}

void SpritesHelper::SetPixelization(const HierarchyTreeNode* rootNode, bool value)
{
    Set<Sprite*> sprites = EnumerateSprites(rootNode);
    SetPixelization(sprites, value);
}

void SpritesHelper::SetPixelization(Set<Sprite*>& spritesList, bool value)
{
    for (Set<Sprite*>::iterator iter = spritesList.begin(); iter != spritesList.end(); iter ++)
    {
        SetPixelization(*iter, value);
    }
}

void SpritesHelper::SetPixelization(Sprite* sprite, bool value)
{
    if (!sprite)
    {
        return;
    }

    int32 frameCount = sprite->GetFrameCount();
    for (int32 i = 0; i < frameCount; i ++)
    {
        Texture* texture = sprite->GetTexture(i);
        if (!texture || !texture->GetDescriptor())
        {
            continue;
        }

        Texture::TextureFilter minFilter = value ? Texture::FILTER_NEAREST : (Texture::TextureFilter)texture->GetDescriptor()->settings.minFilter;
        Texture::TextureFilter magFilter = value ? Texture::FILTER_NEAREST : (Texture::TextureFilter)texture->GetDescriptor()->settings.magFilter;
        texture->SetMinMagFilter(minFilter, magFilter);
    }
}

void SpritesHelper::SetPixelization(UIButton* button, bool value)
{
    Set<Sprite*> spritesList;
    BuildSpritesList(spritesList, button);
    SetPixelization(spritesList, value);
}

void SpritesHelper::SetPixelization(UIStaticText* staticText, bool value)
{
    Set<Sprite*> spritesList;
    BuildSpritesList(spritesList, staticText);
    SetPixelization(spritesList, value);
}

void SpritesHelper::BuildSpritesList(Set<Sprite*>& spritesList, UIButton* button)
{
    // Button might contain backgrounds and/or texts, check both.
    int32 statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int32 i = 0; i < statesCount; ++i)
    {
        Sprite* buttonSprite = button->GetStateSprite(UIControlStateHelper::GetUIControlState(i));
        if (buttonSprite)
        {
            spritesList.insert(buttonSprite);
        }

        UIStaticText* textControl = button->GetStateTextControl(i);
        BuildSpritesList(spritesList, textControl);
    }
}

void SpritesHelper::BuildSpritesList(Set<Sprite*>& spritesList, UIStaticText* staticText)
{
    if (staticText && staticText->GetTextBlock() && staticText->GetTextBlock()->IsSpriteReady())
    {
        spritesList.insert(staticText->GetTextBlock()->GetSprite());
    }
}
