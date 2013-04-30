#ifndef __RESOURCE_EDITOR_MATERIAL_VIEW_OPTIONS_COMMANDS_H__
#define __RESOURCE_EDITOR_MATERIAL_VIEW_OPTIONS_COMMANDS_H__

#include "Command.h"
#include "DAVAEngine.h"
#include "Constants.h"
#include "Qt/DockSetSwitchIndex/SetSwitchIndexHelper.h"

namespace DAVA
{
class CommandChangeMaterialViewOption: public Command
{
public:
	CommandChangeMaterialViewOption(Material::eViewOptions value);

protected:
	Material::eViewOptions	value;

    virtual void Execute();
};

};
#endif // #ifndef __RESOURCE_EDITOR_MATERIAL_VIEW_OPTIONS_COMMANDS_H__