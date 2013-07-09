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

#include "ControlCommands.h"
#include "CommandsController.h"

using namespace DAVA;

ControlsMoveCommand::ControlsMoveCommand(const HierarchyTreeController::SELECTEDCONTROLNODES& controls, const Vector2& delta)
{
	this->controls = controls;
	this->delta = delta;
}

void ControlsMoveCommand::Execute()
{
	ApplyMove(this->delta);
    
    // Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

void ControlsMoveCommand::Rollback()
{
	// Return the controls to the previous positions.
	Vector2 prevDelta = -1 * delta;
	ApplyMove(prevDelta);

    // Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

void ControlsMoveCommand::ApplyMove(const Vector2& moveDelta)
{
	for (HierarchyTreeController::SELECTEDCONTROLNODES::iterator iter = this->controls.begin();
         iter != this->controls.end(); iter ++)
    {
        BaseMetadata* baseMetadata = GetMetadataForTreeNode((*iter)->GetId());
		
        // This command is NOT state-aware and contains one and only param.
        baseMetadata->SetActiveParamID(0);
        baseMetadata->ApplyMove(moveDelta);
		
        SAFE_DELETE(baseMetadata);
    }
}

ControlResizeCommand::ControlResizeCommand(HierarchyTreeNode::HIERARCHYTREENODEID nodeId, const Rect& originalRect, const Rect& newRect)
{
	this->nodeId = nodeId;
	this->originalRect = originalRect;
	this->newRect = newRect;
}

void ControlResizeCommand::Execute()
{
	ApplyResize(originalRect, newRect);
	
	// Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

void ControlResizeCommand::Rollback()
{
	// Return the controls to the previous size.
	ApplyResize(newRect, originalRect);
	
    // Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

void ControlResizeCommand::ApplyResize(const Rect& prevRect, const Rect& updatedRect)
{
	
    BaseMetadata* baseMetadata = GetMetadataForTreeNode(nodeId);

    // This command is NOT state-aware and contains one and only param.
    baseMetadata->SetActiveParamID(0);
    baseMetadata->ApplyResize(prevRect, updatedRect);
    
    SAFE_DELETE(baseMetadata);
}
