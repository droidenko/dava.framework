#ifndef __SCENE_SELECTION_SYSTEM_H__
#define __SCENE_SELECTION_SYSTEM_H__

#include "Qt/Scene/EntityGroup.h"

// framework
#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

class SceneCollisionSystem;
class HoodSystem;

class SceneSelectionSystem : public DAVA::SceneSystem
{
	friend class SceneEditorProxy;
	friend class EntityModificationSystem;

public:
	enum SelectionDrawFlags
	{
		DEBUG_DRAW_NOTHING = 0x0,

		SELECTION_DRAW_SHAPE = 0x1,
		SELECTION_FILL_SHAPE = 0x2,
		SELECTION_DRAW_CORNERS = 0x4,
		SELECTION_NO_DEEP_TEST = 0x10,

		DEBUG_DRAW_ALL = 0xFFFFFFFF
	};

	enum SelectionPivotPoint
	{
		SELECTION_ENTITY_CENTER,
		SELECTION_COMMON_CENTER
	};

	SceneSelectionSystem(DAVA::Scene * scene, SceneCollisionSystem *collSys, HoodSystem *hoodSys);
	~SceneSelectionSystem();

	void SetSelection(DAVA::Entity *entity);
	void AddSelection(DAVA::Entity *entity);
	void RemSelection(DAVA::Entity *entity);

	const EntityGroup* GetSelection() const;

	void SetDrawFlags(int flags);
	int GetDrawFlags() const;

	void SetPivotPoint(int pp);
	int GetPivotPoint() const;

	void UpdateHoodPos() const;
	void SelectedItemsWereModified();

protected:
	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();

	EntityGroup GetSelecetableFromCollision(const EntityGroup *collisionEntities);
	EntityGroupItem GetSelectableEntity(DAVA::Entity* entity);

private:
	int selectionDrawFlags;
	bool applyOnPhaseEnd;

	SceneCollisionSystem *collisionSystem;
	HoodSystem* hoodSystem;

	EntityGroup curSelections;
	DAVA::Entity *lastSelection;

	int  curPivotPoint;
};

#endif //__SCENE_SELECTION_SYSTEM_H__
