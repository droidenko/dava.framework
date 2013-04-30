#ifndef __RESOURCE_EDITOR_CUSTOM_COLOR_COMMANDS_H__
#define __RESOURCE_EDITOR_CUSTOM_COLOR_COMMANDS_H__

#include "Command.h"
#include "DAVAEngine.h"
#include "Constants.h"
#include "SceneEditor/LandscapeEditorCustomColors.h"

using namespace DAVA;

class CommandSaveTextureCustomColors: public Command
{
public:
    CommandSaveTextureCustomColors();
protected:
    virtual void Execute();
};

class CommandLoadTextureCustomColors: public Command
{
public:
    CommandLoadTextureCustomColors();
protected:
    virtual void Execute();
};

class CommandDrawCustomColors: public Command
{
public:
	CommandDrawCustomColors(Image* originalImage, Image* newImage);
	virtual ~CommandDrawCustomColors();
protected:
	Image* undoImage;
	Image* redoImage;

	LandscapeEditorCustomColors* GetEditor();

	virtual void Execute();
	virtual void Cancel();
};


#endif // #ifndef __RESOURCE_EDITOR_CUSTOM_COLOR_COMMANDS_H__