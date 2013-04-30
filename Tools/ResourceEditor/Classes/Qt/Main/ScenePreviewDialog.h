#ifndef __SCENE_PREVIEW_DIALOG_H__
#define __SCENE_PREVIEW_DIALOG_H__

#include "DAVAEngine.h"
#include "EditorScene.h"
#include "SceneEditor/ExtendedDialog.h"

using namespace DAVA;


class ScenePreviewControl;
class ScenePreviewDialog: public ExtendedDialog
{
    
public:
    ScenePreviewDialog();
    virtual ~ScenePreviewDialog();
    
    void Show(const FilePath &scenePathname);
    virtual void Close();
    
protected:

    virtual const Rect GetDialogRect() const;
    virtual void UpdateSize();
    
    void OnClose(BaseObject *, void *, void *);

    ScenePreviewControl *preview;
    UIStaticText *errorMessage;
};



#endif // __SCENE_PREVIEW_DIALOG_H__