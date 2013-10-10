/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "RulerToolActions.h"
#include "../Qt/Scene/SceneEditor2.h"
#include "../Qt/Scene/SceneSignals.h"

#include "../Qt/Main/QtUtils.h"

ActionEnableRulerTool::ActionEnableRulerTool(SceneEditor2* forSceneEditor)
:	CommandAction(CMDID_ENABLE_RULER_TOOL)
,	sceneEditor(forSceneEditor)
{
}

void ActionEnableRulerTool::Redo()
{
	if (sceneEditor == NULL)
	{
		return;
	}
	
	bool enabled = sceneEditor->rulerToolSystem->IsLandscapeEditingEnabled();
	if (enabled)
	{
		return;
	}
	
	sceneEditor->DisableTools(SceneEditor2::LANDSCAPE_TOOLS_ALL);
	
	bool success = !sceneEditor->IsToolsEnabled(SceneEditor2::LANDSCAPE_TOOLS_ALL);
	
	if (!success || !sceneEditor->rulerToolSystem->EnableLandscapeEditing())
	{
		ShowErrorDialog(ResourceEditor::RULER_TOOL_ENABLE_ERROR);
	}
	
	SceneSignals::Instance()->EmitRulerToolToggled(sceneEditor);
}

ActionDisableRulerTool::ActionDisableRulerTool(SceneEditor2* forSceneEditor)
:	CommandAction(CMDID_DISABLE_RULER_TOOL)
,	sceneEditor(forSceneEditor)
{
}

void ActionDisableRulerTool::Redo()
{
	if (sceneEditor == NULL)
	{
		return;
	}
	
	bool disabled = !sceneEditor->rulerToolSystem->IsLandscapeEditingEnabled();
	if (disabled)
	{
		return;
	}
	
	disabled = sceneEditor->rulerToolSystem->DisableLandscapeEdititing();
	if (!disabled)
	{
		ShowErrorDialog(ResourceEditor::RULER_TOOL_DISABLE_ERROR);
	}
	
	SceneSignals::Instance()->EmitRulerToolToggled(sceneEditor);
}