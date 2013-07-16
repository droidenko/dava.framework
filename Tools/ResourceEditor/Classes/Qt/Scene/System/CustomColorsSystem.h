/*==================================================================================
 Copyright (c) 2008, DAVA Consulting, LLC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the DAVA Consulting, LLC nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/

#ifndef __RESOURCEEDITORQT__CUSTOMCOLORSSYSTEM__
#define __RESOURCEEDITORQT__CUSTOMCOLORSSYSTEM__

#include "Entity/SceneSystem.h"
#include "EditorScene.h"

class SceneCollisionSystem;
class SceneSelectionSystem;
class EntityModificationSystem;
class LandscapeEditorDrawSystem;

class CustomColorsSystem: public DAVA::SceneSystem
{
public:
	CustomColorsSystem(Scene* scene);
	virtual ~CustomColorsSystem();
	
	bool EnableLandscapeEditing();
	bool DisableLandscapeEdititing();
	bool IsLandscapeEditingEnabled() const;
	
	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	
	void SetBrushSize(int32 brushSize);
	void SetColor(int32 colorIndex);

	void SaveTexture(const FilePath& filePath);
	void LoadTexture(const FilePath& filePath);
	FilePath GetCurrentSaveFileName();

protected:
	bool enabled;
	
	SceneCollisionSystem* collisionSystem;
	SceneSelectionSystem* selectionSystem;
	EntityModificationSystem* modifSystem;
	LandscapeEditorDrawSystem* drawSystem;
	
	int32 landscapeSize;
	Texture* cursorTexture;
	uint32 cursorSize;
	uint32 curToolSize;
	Sprite* toolImageSprite;
	
	//	Sprite* colorSprite;
	Color drawColor;
	
	bool isIntersectsLandscape;
	Vector2 cursorPosition;
	Vector2 prevCursorPos;
	
	Rect updatedRectAccumulator;
	
	bool editingIsEnabled;
	
	Image* originalImage;
	
	void UpdateCursorPosition();
	void UpdateToolImage(bool force = false);
	void UpdateBrushTool(float32 timeElapsed);
	Image* CreateToolImage(int32 sideSize, const FilePath& filePath);
	
	void AddRectToAccumulator(const Rect& rect);
	void ResetAccumulatorRect();
	Rect GetUpdatedRect();
	
	void StoreOriginalState();
	void CreateUndoPoint();

	void StoreSaveFileName(const FilePath& filePath);

	FilePath GetScenePath();
	String GetRelativePathToScenePath(const FilePath& absolutePath);
	FilePath GetAbsolutePathFromScenePath(const String& relativePath);
	String GetRelativePathToProjectPath(const FilePath& absolutePath);
	FilePath GetAbsolutePathFromProjectPath(const String& relativePath);
};

#endif /* defined(__RESOURCEEDITORQT__CUSTOMCOLORSSYSTEM__) */