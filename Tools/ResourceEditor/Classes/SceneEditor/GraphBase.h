#ifndef __GRAPH_BASE_H__
#define __GRAPH_BASE_H__

#include "DAVAEngine.h"
#include "NodesPropertyControl.h"

using namespace DAVA;

class GraphBaseDelegate
{
public:
    virtual bool LandscapeEditorActive() = 0;
    virtual NodesPropertyControl *GetPropertyControl(const Rect &rect) = 0;
};

//class EditorScene;
#include "EditorScene.h"

class GraphBase: 
        public BaseObject, 
        public UIHierarchyDelegate, 
        public NodesPropertyDelegate
{
    
public:
    GraphBase(GraphBaseDelegate *newDelegate, const Rect &rect);
    virtual ~GraphBase();
    
    UIControl * GetGraphPanel();
    UIControl * GetPropertyPanel();
    
    void SetScene(EditorScene *scene);
    
    bool GraphOnScreen();
    bool PropertiesOnScreen();
    void UpdateMatricesForCurrentNode();
    
    virtual void SelectNode(BaseObject *node) = 0;
    virtual void UpdatePropertyPanel() = 0;
    
    virtual void RemoveWorkingNode() {};
    virtual void RefreshGraph();
    
    //NodesPropertyDelegate
    virtual void NodesPropertyChanged(const String& forKey);
    
protected:

    //NodesPropertyDelegate
    virtual UIHierarchyCell *CellForNode(UIHierarchy *forHierarchy, void *node);
    virtual void OnCellSelected(UIHierarchy *forHierarchy, UIHierarchyCell *selectedCell);
    
    virtual void FillCell(UIHierarchyCell *cell, void *node) = 0;
    virtual void SelectHierarchyNode(UIHierarchyNode * node) = 0;

    void OnRefreshPropertyControl(BaseObject * object, void * userData, void * callerData);
    
    virtual void CreateGraphPanel(const Rect &rect);
    void CreatePropertyPanel(const Rect &rect);

	// Returns TRUE if it is enough to rebuild selected node for this property.
	bool IsRebuildSelectedNodeEnough(const String& propertyName);

    UIControl *graphPanel;
    UIControl *propertyPanel;

    UIHierarchy * graphTree;

    NodesPropertyControl *propertyControl;
	Rect propertyPanelRect;
    
    GraphBaseDelegate *delegate;
    EditorScene *workingScene;
};



#endif // __GRAPH_BASE_H__