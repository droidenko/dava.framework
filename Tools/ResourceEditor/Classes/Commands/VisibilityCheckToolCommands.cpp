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

#include <QFileDialog>
#include "VisibilityCheckToolCommands.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Main/QtUtils.h"
#include "../SceneEditor/EditorBodyControl.h"

CommandSaveTextureVisibilityTool::CommandSaveTextureVisibilityTool()
:	Command(COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_SAVE_TEXTURE_VISIBILITY_TOOL)
{
}

void CommandSaveTextureVisibilityTool::Execute()
{
    FilePath currentPath = FileSystem::Instance()->GetUserDocumentsPath();
	QString filePath = QFileDialog::getSaveFileName(NULL,
													QString("Save texture"),
													QString(currentPath.GetAbsolutePathname().c_str()),
													QString("PNG image (*.png)"));

	FilePath selectedPathname = PathnameToDAVAStyle(filePath);

	if(!selectedPathname.IsEmpty())
	{
		SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
		if(screen)
		{
			screen->VisibilityToolSaveTexture(selectedPathname);
		}
	}
}

CommandPlacePointVisibilityTool::CommandPlacePointVisibilityTool(const Vector2& newVisibilityPoint, const Vector2& oldVisibilityPoint, bool oldPointIsSet, Image* oldImage)
:	Command(COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_PLACE_POINT_VISIBILITY_TOOL)
,	point(newVisibilityPoint)
,	oldPoint(oldVisibilityPoint)
,	oldPointIsSet(oldPointIsSet)
{
	commandName = "Place Visibility Point";

	this->oldImage = SafeRetain(oldImage);
}

CommandPlacePointVisibilityTool::~CommandPlacePointVisibilityTool()
{
	SafeRelease(oldImage);
}

void CommandPlacePointVisibilityTool::Execute()
{
	LandscapeEditorVisibilityCheckTool* editor = GetEditor();
	if (editor)
		editor->SetVisibilityPoint(point);
	else
		SetState(STATE_INVALID);
}

void CommandPlacePointVisibilityTool::Cancel()
{
	LandscapeEditorVisibilityCheckTool* editor = GetEditor();
	if (editor)
		editor->RestorePointState(oldPoint, oldPointIsSet, oldImage);
}

LandscapeEditorVisibilityCheckTool* CommandPlacePointVisibilityTool::GetEditor()
{
	LandscapeEditorVisibilityCheckTool* editor = NULL;

	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = dynamic_cast<LandscapeEditorVisibilityCheckTool*>(screen->FindCurrentBody()->bodyControl->GetLandscapeEditor(SceneEditorScreenMain::ELEMID_VISIBILITY_CHECK_TOOL));
	}

	return editor;
}

CommandPlaceAreaVisibilityTool::CommandPlaceAreaVisibilityTool(const Vector2& areaPoint, uint32 areaSize, Image* oldImage)
:	Command(COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_PLACE_AREA_VISIBILITY_TOOL),
	point(areaPoint),
	size(areaSize),
	redoImage(NULL)
{
	commandName = "Place Visibility Area";

	this->oldImage = SafeRetain(oldImage);
}

CommandPlaceAreaVisibilityTool::~CommandPlaceAreaVisibilityTool()
{
	SafeRelease(oldImage);
}

void CommandPlaceAreaVisibilityTool::Execute()
{
	LandscapeEditorVisibilityCheckTool* editor = GetEditor();
	if (editor)
	{
		if (!redoImage)
		{
			editor->SetVisibilityArea(point, size);
			redoImage = editor->StoreAreaState();
		}
		else
			editor->RestoreAreaState(redoImage);
	}
	else
		SetState(STATE_INVALID);
}

void CommandPlaceAreaVisibilityTool::Cancel()
{
	LandscapeEditorVisibilityCheckTool* editor = GetEditor();
	if (editor)
		editor->RestoreAreaState(oldImage);
}

LandscapeEditorVisibilityCheckTool* CommandPlaceAreaVisibilityTool::GetEditor()
{
	LandscapeEditorVisibilityCheckTool* editor = NULL;

	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = dynamic_cast<LandscapeEditorVisibilityCheckTool*>(screen->FindCurrentBody()->bodyControl->GetLandscapeEditor(SceneEditorScreenMain::ELEMID_VISIBILITY_CHECK_TOOL));
	}

	return editor;
}
