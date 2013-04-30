#include "SceneSignals.h"

void SceneSignals::EmitMouseOver(SceneEditorProxy *scene, DAVA::Entity *entity)
{
	emit MouseOver(scene, entity);
}

void SceneSignals::EmitMouseOverSelection(SceneEditorProxy *scene, DAVA::Entity *entity)
{
	emit MouseOverSelection(scene, entity);
}

void SceneSignals::EmitSelected(SceneEditorProxy *scene, DAVA::Entity *entity)
{
	emit Selected(scene, entity);
}

void SceneSignals::EmitDeselected(SceneEditorProxy *scene, DAVA::Entity *entity)
{
	emit Deselected(scene, entity);
}
