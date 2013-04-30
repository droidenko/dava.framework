#include "CustomColorCommands.h"

#include "SceneEditor/SceneEditorScreenMain.h"
#include "Qt/Main/QtUtils.h"
#include <QFileDialog>
#include "SceneEditor/EditorBodyControl.h"

CommandSaveTextureCustomColors::CommandSaveTextureCustomColors()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
    
}

void CommandSaveTextureCustomColors::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(!screen)
		return;

	FilePath selectedPathname = screen->CustomColorsGetCurrentSaveFileName();

	if(selectedPathname.IsEmpty())
	{
		selectedPathname = FilePath(screen->CurrentScenePathname().GetDirectory());
	}

	QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save texture"), QString(selectedPathname.GetAbsolutePathname().c_str()), QString("PNG image (*.png)"));

	selectedPathname = PathnameToDAVAStyle(filePath);

	if(!selectedPathname.IsEmpty())
		screen->CustomColorsSaveTexture(selectedPathname);
}

CommandLoadTextureCustomColors::CommandLoadTextureCustomColors()
:	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandLoadTextureCustomColors::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(!screen)
		return;

	FilePath currentPath = screen->CustomColorsGetCurrentSaveFileName();

	if(currentPath.IsEmpty())
	{
		currentPath = FilePath(screen->CurrentScenePathname().GetDirectory());
	}

	FilePath selectedPathname = GetOpenFileName(String("Load texture"), currentPath, String("PNG image (*.png)"));
	if(!selectedPathname.IsEmpty())
	{
		screen->CustomColorsLoadTexture(selectedPathname);
	}
}


CommandDrawCustomColors::CommandDrawCustomColors(Image* originalImage, Image* newImage)
:	Command(COMMAND_UNDO_REDO)
{
	commandName = "Custom Color Draw";

	undoImage = SafeRetain(originalImage);
	redoImage = SafeRetain(newImage);
}

CommandDrawCustomColors::~CommandDrawCustomColors()
{
	SafeRelease(undoImage);
	SafeRelease(redoImage);
}

void CommandDrawCustomColors::Execute()
{
	LandscapeEditorCustomColors* editor = GetEditor();
	if (editor == NULL || redoImage == NULL)
	{
		SetState(STATE_INVALID);
		return;
	}

	editor->RestoreState(redoImage);
}

void CommandDrawCustomColors::Cancel()
{
	LandscapeEditorCustomColors* editor = GetEditor();
	if (editor && undoImage)
		editor->RestoreState(undoImage);
}

LandscapeEditorCustomColors* CommandDrawCustomColors::GetEditor()
{
	LandscapeEditorCustomColors* editor = NULL;

	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = dynamic_cast<LandscapeEditorCustomColors*>(screen->FindCurrentBody()->bodyControl->GetLandscapeEditor(SceneEditorScreenMain::ELEMID_CUSTOM_COLORS));
	}

	return editor;
}