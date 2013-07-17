/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "EditorListDelegate.h"
#include "Utils/Utils.h"
#include "UI/UIAggregatorControl.h"

#include "HierarchyTreeAggregatorNode.h"
#include "HierarchyTreeAggregatorControlNode.h"
#include "HierarchyTreeController.h"
#include "EditorFontManager.h"

#define CELLS_COUNT 3
#define DEFAULT_AGGREGATOR_ID 0

namespace DAVA
{
	EditorListDelegate::EditorListDelegate(const Rect &rect, UIList::eListOrientation orientation /*ORIENTATION_VERTICAL*/
																			, bool rectInAbsoluteCoordinates/* = FALSE*/)
	: UIControl(rect, rectInAbsoluteCoordinates)
	{
		if (orientation == UIList::ORIENTATION_VERTICAL)
		{
			cellSize = Vector2(rect.dx, (rect.dy / CELLS_COUNT));
		}
		else
		{
			cellSize = Vector2((rect.dx / CELLS_COUNT), rect.dy);
		}
		aggregatorID = DEFAULT_AGGREGATOR_ID;
	}
	
	EditorListDelegate::~EditorListDelegate()
	{
	}
	
	void EditorListDelegate::SetAggregatorID(int32 id)
	{
		aggregatorID = id;
	}
	
	int32 EditorListDelegate::GetAggregatorID()
	{
		return aggregatorID;
	}
	
	void EditorListDelegate::SetCellSize(const Vector2 &size)
	{
		cellSize = size;
	}
	
	// UIListDelegate implementation	
	int32 EditorListDelegate::ElementsCount(UIList *)
	{
		return CELLS_COUNT;
	}
	
	UIListCell *EditorListDelegate::CellAtIndex(UIList *forList, int32 index)
	{
    	// Try to get cell from the reusable cells store
    	UIListCell *cell = forList->GetReusableCell("Cell");
		if (!cell)
		{
			cell = new UIListCell(Rect(0.0f, 0.0f, cellSize.x, cellSize.y), "Cell");
		}
		else
		{
			cell->SetSize(Vector2(cellSize.x, cellSize.y));
			// Reset reusable cells relative positions - new proper positions will be calculated at UIList::AddCellAtPost() method
			cell->SetPosition(Vector2(0.0f, 0.0f));
		}
		
		// Get aggregator node
		HierarchyTreeNode *node = HierarchyTreeController::Instance()->GetTree().GetNode(aggregatorID);
		HierarchyTreeAggregatorNode *aggregatorNode = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
	
		cell->RemoveAllControls();
		
		if (aggregatorNode)
		{
			UIAggregatorControl* control = new UIAggregatorControl();
			control->CopyDataFrom(aggregatorNode->GetScreen());
			cell->AddControl(control);
			
			control->SetRightAlignEnabled(true);
			control->SetLeftAlignEnabled(true);
			control->SetTopAlignEnabled(true);
			control->SetBottomAlignEnabled(true);
		}
		else
		{			
			cell->SetStateFont(UIControl::STATE_NORMAL, EditorFontManager::Instance()->GetDefaultFont());
			cell->SetStateText(UIControl::STATE_NORMAL, StringToWString(Format("Cell %d",index)));
			cell->SetSelected(true);
		}
   
    	return cell;
	}

	int32 EditorListDelegate::CellHeight(UIList *, int32)
	{
    	return cellSize.y;
	}
	
	int32 EditorListDelegate::CellWidth(UIList *, int32)
	{
		return cellSize.x;
	}
	
	void EditorListDelegate::SaveToYaml(UIList *forList, YamlNode *)
	{
		HierarchyTreeNode *node = HierarchyTreeController::Instance()->GetTree().GetNode(aggregatorID);
		HierarchyTreeAggregatorNode *aggregatorNode = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
		if (aggregatorNode)
		{
			forList->SetAggregatorPath(aggregatorNode->GetPath());
		}
		else
		{
			forList->SetAggregatorPath(String());
		}
	}
};