#include "MaterialViewOptionsCommands.h"

#include "SceneEditor/SceneEditorScreenMain.h"
#include "Qt/Main/QtUtils.h"
#include <QFileDialog>
#include "SceneEditor/EditorBodyControl.h"
#include "Qt/Scene/SceneDataManager.h"


CommandChangeMaterialViewOption::CommandChangeMaterialViewOption(Material::eViewOptions value)
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
    this->value = value;
}

void CommandChangeMaterialViewOption::Execute()
{
	for(int i = 0; i < SceneDataManager::Instance()->SceneCount(); ++i)
	{
		Scene *scene = SceneDataManager::Instance()->SceneGet(i)->GetScene();
		List<DAVA::Material*> materials;
		scene->GetDataNodes(materials);

		for(List<Material*>::iterator it = materials.begin(); it != materials.end(); it++)
		{
			(*it)->SetViewOption(value);
		}
	}
}
