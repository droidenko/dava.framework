#ifndef __RESOURCE_EDITOR_SWITCH_INDEX_COMMANDS_H__
#define __RESOURCE_EDITOR_SWITCH_INDEX_COMMANDS_H__

#include "Command.h"
#include "DAVAEngine.h"
#include "Constants.h"
#include "Qt/DockSetSwitchIndex/SetSwitchIndexHelper.h"

namespace DAVA
{
class CommandToggleSetSwitchIndex: public Command
{
public:
	CommandToggleSetSwitchIndex(uint32 value, SetSwitchIndexHelper::eSET_SWITCH_INDEX state);

protected:
	uint32	value;
	SetSwitchIndexHelper::eSET_SWITCH_INDEX	swtichState;

    virtual void Execute();
};

};
#endif // #ifndef __RESOURCE_EDITOR_SWITCH_INDEX_COMMANDS_H__