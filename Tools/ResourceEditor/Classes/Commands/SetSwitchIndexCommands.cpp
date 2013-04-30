#include "SetSwitchIndexCommands.h"

#include "SceneEditor/SceneEditorScreenMain.h"
#include "Qt/Main/QtUtils.h"
#include <QFileDialog>
#include "SceneEditor/EditorBodyControl.h"
#include "Qt/DockSetSwitchIndex/SetSwitchIndexHelper.h"


CommandToggleSetSwitchIndex::CommandToggleSetSwitchIndex(uint32 value, SetSwitchIndexHelper::eSET_SWITCH_INDEX state)
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
    this->value = value;
	this->swtichState = state;
}

void CommandToggleSetSwitchIndex::Execute()
{
	SetSwitchIndexHelper::ProcessSwitchIndexUpdate(this->value, this->swtichState);

}
