//
//  SpritesHelper.h
//  UIEditor
//
//  Created by Yuri Coder on 12/23/13.
//
//

#ifndef __SPRITESHELPER__H__
#define __SPRITESHELPER__H__

#include "DAVAEngine.h"

#include "Models/HierarchyTreeNode.h"
#include "Models/HierarchyTreeControlNode.h"

using namespace DAVA;

class SpritesHelper
{
public:
    // Get the list of the sprites in the Hierarchy Tree.
    static Set<Sprite*> EnumerateSprites(const HierarchyTreeNode* rootNode);

    // Pixelize one and all textures (apply the FILTER_NEAREST filter).
    static void SetPixelization(Sprite* sprite, bool value);
    static void SetPixelization(Set<Sprite*>& sprite, bool value);
    static void SetPixelization(UIButton* button, bool value);
    static void SetPixelization(UIStaticText* staticText, bool value);

    // Set the pixelization value for all sprites.
    static void SetPixelization(const HierarchyTreeNode* rootNode, bool value);

protected:
    // Do the checks needed for the static text and add its sprite to the list.
    static void AddStaticTextSprite(Set<Sprite*>& spritesList, UIStaticText* staticText);
    
    // Build the sprites list to be reloaded in a recursive way.
    static void BuildSpritesListRecursive(const HierarchyTreeControlNode* controlNode, Set<Sprite*>& spritesToReload);

    // Build sprites list - controls-specific functions.
    static void BuildSpritesList(Set<Sprite*>& spritesList, UIButton* button);
    static void BuildSpritesList(Set<Sprite*>& spritesList, UIStaticText* staticText);
};

#endif /* defined(__SPRITESHELPER__H__) */
