#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include <QObject>

// framework
#include "Base/StaticSingleton.h"
#include "Scene3D/Entity.h"

class SceneEditorProxy;

class SceneSignals : public QObject, public DAVA::StaticSingleton<SceneSignals>
{
	Q_OBJECT

signals:
	void MouseOver(SceneEditorProxy *scene, DAVA::Entity *entity);
	void MouseOverSelection(SceneEditorProxy *scene, DAVA::Entity *entity);

	void Selected(SceneEditorProxy *scene, DAVA::Entity *entity);
	void Deselected(SceneEditorProxy *scene, DAVA::Entity *entity);

public:
	void EmitMouseOver(SceneEditorProxy *scene, DAVA::Entity *entity);
	void EmitMouseOverSelection(SceneEditorProxy *scene, DAVA::Entity *entity);

	void EmitSelected(SceneEditorProxy *scene, DAVA::Entity *entity);
	void EmitDeselected(SceneEditorProxy *scene, DAVA::Entity *entity);
};

#endif // __SCENE_MANAGER_H__
