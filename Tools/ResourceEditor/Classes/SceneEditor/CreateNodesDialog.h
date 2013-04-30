#ifndef __CREATE_NODES_DIALOG_H__
#define __CREATE_NODES_DIALOG_H__

#include "DAVAEngine.h"
#include "DraggableDialog.h"
#include "Constants.h"


using namespace DAVA;

class CreateNodesDialogDelegeate
{
public:

    virtual void DialogClosed(int32 retCode) = 0;
};

class NodesPropertyControl;
class CreateNodesDialog: public DraggableDialog
{
    
public:
    enum eRetCode
    {
        RCODE_CANCEL = 0,
        RCODE_OK,
    };
    
public:
    CreateNodesDialog(const Rect & rect);
    virtual ~CreateNodesDialog();

	virtual void WillDisappear();
    
    void SetDelegate(CreateNodesDialogDelegeate *delegate);
    void SetScene(Scene *_scene);

    void CreateNode(ResourceEditor::eNodeType nodeType);
    Entity *GetSceneNode();

protected:

    void SetHeader(const WideString &headerText);
    
    void OnCancel(BaseObject * object, void * userData, void * callerData);
    void OnOk(BaseObject * object, void * userData, void * callerData);

    CreateNodesDialogDelegeate *dialogDelegate;
    
    NodesPropertyControl *propertyList;
  
    UIStaticText *header;
    Entity *sceneNode;
    Scene *scene;

	Rect propertyRect;
};



#endif // __CREATE_NODES_DIALOG_H__