#include "DataGraph.h"
#include "ControlsFactory.h"

#include "EditorScene.h"
#include "EditorSettings.h"
#include "PropertyControlCreator.h"

DataGraph::DataGraph(GraphBaseDelegate *newDelegate, const Rect &rect)
    :   GraphBase(newDelegate, rect)
    ,   workingNode(NULL)
{
    CreateGraphPanel(rect);
}

DataGraph::~DataGraph()
{
}

void DataGraph::SelectNode(BaseObject *node)
{
    workingNode = dynamic_cast<DataNode *>(node);
    UpdatePropertyPanel();
    
    RefreshGraph();
}


void DataGraph::CreateGraphPanel(const Rect &rect)
{
    GraphBase::CreateGraphPanel(rect);
    
    Rect graphRect = graphPanel->GetRect();
    graphRect.dy -= (ControlsFactory::BUTTON_HEIGHT);
    graphTree = new UIHierarchy(graphRect);
    ControlsFactory::CusomizeListControl(graphTree);
    ControlsFactory::SetScrollbar(graphTree);
    graphTree->SetCellHeight(ControlsFactory::CELL_HEIGHT);
    graphTree->SetDelegate(this);
    graphTree->SetClipContents(true);
    graphPanel->AddControl(graphTree);

    
    float32 leftSideWidth = (float32)EditorSettings::Instance()->GetLeftPanelWidth();
    UIButton * refreshButton = ControlsFactory::CreateButton(Rect(0, graphRect.dy, 
                                                                  leftSideWidth,ControlsFactory::BUTTON_HEIGHT), 
                                                             LocalizedString(L"panel.refresh"));
    refreshButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &DataGraph::OnRefreshGraph));
    graphPanel->AddControl(refreshButton);
    SafeRelease(refreshButton);
}

void DataGraph::FillCell(UIHierarchyCell *cell, void *node)
{
    //Temporary fix for loading of UI Interface to avoid reloading of texrures to different formates.
    // 1. Reset default format before loading of UI
    // 2. Restore default format after loading of UI from stored settings.
    Texture::SetDefaultFileFormat(NOT_FILE);

    DataNode *n = (DataNode *)node;
    UIStaticText *text =  (UIStaticText *)cell->FindByName("_Text_");
    text->SetText(StringToWString(n->GetName()));
    
    UIControl *icon = cell->FindByName("_Icon_");
    icon->SetSprite("~res:/Gfx/UI/SceneNode/datanode", 0);

    UIControl *marker = cell->FindByName("_Marker_");
    marker->SetVisible(false);
    
    if(n == workingNode)
    {
        cell->SetSelected(true, false);
    }
    else
    {
        cell->SetSelected(false, false);
    }
    
    Texture::SetDefaultFileFormat((ImageFileFormat)EditorSettings::Instance()->GetTextureViewFileFormat());
}

void DataGraph::SelectHierarchyNode(UIHierarchyNode * node)
{
    DataNode * dataNode = dynamic_cast<DataNode*>((BaseObject*)node->GetUserNode());
    if (dataNode)
    {
        workingNode = dataNode;
        UpdatePropertyPanel();
    }
}


void DataGraph::UpdatePropertyPanel()
{
    if(workingNode && (NULL != graphTree->GetParent()))
    {
		RecreatePropertiesPanelForNode(workingNode);
        if(!propertyControl->GetParent())
        {
            propertyPanel->AddControl(propertyControl);
        }
    }
    else
    {
        if(propertyControl && propertyControl->GetParent())
        {
            propertyPanel->RemoveControl(propertyControl);
        }
    }
}

void DataGraph::RecreatePropertiesPanelForNode(DataNode * node)
{
	if(propertyControl && propertyControl->GetParent())
	{
		propertyPanel->RemoveControl(propertyControl);
	}
	SafeRelease(propertyControl);
    
	propertyControl = PropertyControlCreator::Instance()->CreateControlForNode(node, propertyPanelRect, false);
    SafeRetain(propertyControl);
	propertyControl->SetDelegate(this);
	propertyControl->SetWorkingScene(workingScene);
    propertyControl->ReadFrom(node);
}


void DataGraph::OnRefreshGraph(BaseObject * obj, void *, void *)
{
    RefreshGraph();
}

void DataGraph::RefreshGraph()
{
    bool force = true;
    if(force || (NULL != graphPanel->GetParent()))
    {
        dataNodes.clear();
        
        if(workingScene && workingScene->GetSelection())
        {
            workingScene->GetSelection()->GetDataNodes(dataNodes);
        }
        
        graphTree->Refresh();
    }
}


bool DataGraph::IsNodeExpandable(UIHierarchy *forHierarchy, void *forNode)
{
    if (forNode) 
    {
        return ((DataNode*)forNode)->GetChildrenCount() > 0;
    }
    
    return dataNodes.size() > 0;
}

int32 DataGraph::ChildrenCount(UIHierarchy *forHierarchy, void *forParent)
{
    if (forParent) 
    {
        return ((DataNode*)forParent)->GetChildrenCount();
    }
    
    return dataNodes.size();
}

void * DataGraph::ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index)
{
    if (forParent) 
    {
        return ((DataNode*)forParent)->GetChild(index);
    }
    
    Set<DataNode *>::const_iterator it = dataNodes.begin();
    Set<DataNode *>::const_iterator endIt = dataNodes.end();
    for(int32 i = 0; it != endIt; ++it, ++i)
    {
        if(i == index)
        {
            return (*it);
        }
    }
    
    return NULL;
}



