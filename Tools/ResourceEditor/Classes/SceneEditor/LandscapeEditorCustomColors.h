#ifndef __LANDSCAPE_EDITOR_CUSTOM_COLOR_H__
#define __LANDSCAPE_EDITOR_CUSTOM_COLOR_H__

#include "DAVAEngine.h"
#include "LandscapeEditorBase.h"
#include "LandscapeToolsPanel.h"
#include "LandscapeEditorPropertyControl.h"


using namespace DAVA;

class EditorHeightmap;
class CommandDrawCustomColors;
class LandscapeEditorCustomColors
    :   public LandscapeEditorBase
    ,   public LandscapeEditorPropertyControlDelegate

{
    
public:
    
    LandscapeEditorCustomColors(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl, const Rect &toolsRect);
    virtual ~LandscapeEditorCustomColors();
    
	virtual void Draw(const UIGeometricData &geometricData);

    virtual NodesPropertyControl *GetPropertyControl(const Rect &rect);
    
    virtual bool SetScene(EditorScene *newScene);
	virtual void SaveTexture();
    
    //LE property control delegate
    virtual void LandscapeEditorSettingsChanged(LandscapeEditorSettings *settings);
    virtual void TextureWillChanged(const String &forKey);
    virtual void TextureDidChanged(const String &forKey);
    

	void SetColor(const Color &newColor);
	void SetRadius(int radius);
	void SaveColorLayer(const FilePath &pathName);
	void LoadColorLayer(const FilePath &pathName);
	FilePath GetCurrentSaveFileName();

	void ClearSceneResources();

	Image* StoreState();
	void RestoreState(Image* image);

	virtual void UpdateLandscapeTilemap(Texture* texture);

protected:

    virtual void InputAction(int32 phase, bool intersects);
    virtual void HideAction();
    virtual void ShowAction();
    virtual void SaveTextureAction(const FilePath &pathToFile);
	virtual void UpdateCursor();

	void StoreOriginalState();
	void CreateUndoPoint();

	FilePath GetScenePath();
	String GetRelativePathToScenePath(const FilePath& absolutePath);
	FilePath GetAbsolutePathFromScenePath(const String& relativePath);
	String GetRelativePathToProjectPath(const FilePath& absolutePath);
	FilePath GetAbsolutePathFromProjectPath(const String& relativePath);
	void StoreSaveFileName(const FilePath& fileName);

	void LoadTextureAction(const FilePath& pathToFile);

    virtual void RecreateHeightmapNode();
	void UpdateCircleTexture(bool setTransparent);


	void PrepareRenderLayers();

    void PerformLandscapeDraw();
    void DrawCircle(Vector<Vector<bool> >& matrixForCircle) ;
    uint8*	DrawFilledCircleWithFormat(uint32 radius, DAVA::PixelFormat format, bool setTransparent);

	bool wasTileMaskToolUpdate;

    LandscapeEditorSettings *settings;

    Color paintColor;

    bool editingIsEnabled;

	Texture*	circleTexture;
	Sprite*		colorSprite;
	Texture*	texSurf;

	int32		radius;

	bool		isCursorTransparent;
	bool		isFogEnabled;

	bool unsavedChanges;

	Image* originalTexture;
};


#endif //__LANDSCAPE_EDITOR_CUSTOM_COLOR_H__
