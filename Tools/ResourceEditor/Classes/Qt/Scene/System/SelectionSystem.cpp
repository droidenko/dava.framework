#include "Qt/Scene/System/SelectionSystem.h"
#include "Qt/Scene/System/CameraSystem.h"
#include "Qt/Scene/System/CollisionSystem.h"
#include "Qt/Scene/System/HoodSystem.h"
#include "Qt/Scene/System/ModifSystem.h"

#include <QApplication>
#include "Qt/Scene/SceneEditorProxy.h"

SceneSelectionSystem::SceneSelectionSystem(DAVA::Scene * scene, SceneCollisionSystem *collSys, HoodSystem *hoodSys)
	: DAVA::SceneSystem(scene)
	, collisionSystem(collSys)
	, hoodSystem(hoodSys)
	, selectionDrawFlags(SELECTION_FILL_SHAPE | SELECTION_DRAW_CORNERS)
	, applyOnPhaseEnd(false)
	, curPivotPoint(SELECTION_COMMON_CENTER)
{

}

SceneSelectionSystem::~SceneSelectionSystem()
{

}

void SceneSelectionSystem::Update(DAVA::float32 timeElapsed)
{
	UpdateHoodPos();
}

void SceneSelectionSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	if(DAVA::UIEvent::PHASE_BEGAN == event->phase)
	{
		// we can select only if mouse isn't over hood axis
		if(EM_AXIS_NONE == hoodSystem->GetPassingAxis())
		{
			if(event->tid == DAVA::UIEvent::BUTTON_1)
			{
				const EntityGroup* collisionEntities = collisionSystem->RayTestFromCamera();
				EntityGroup selectableItems = GetSelecetableFromCollision(collisionEntities);

				DAVA::Entity *firstEntity = selectableItems.GetEntity(0);
				DAVA::Entity *nextEntity = NULL;

				// search possible next item only if now there is no selection or is only single selection
				if(curSelections.Size() <= 1)
				{
					bool found = false;

					// find first after currently selected items
					for(size_t i = 0; i < selectableItems.Size(); i++)
					{
						DAVA::Entity *entity = selectableItems.GetEntity(i);
						if(curSelections.HasEntity(entity))
						{
							if((i + 1) < selectableItems.Size())
							{
								found = true;

								nextEntity = selectableItems.GetEntity(i + 1);
								break;
							}
						}
					}

					if(!found)
					{
						nextEntity = selectableItems.GetEntity(0);
					}
				}

				int curKeyModifiers = QApplication::keyboardModifiers();
				if(curKeyModifiers & Qt::ControlModifier)
				{
					AddSelection(firstEntity);
				}
				else if(curKeyModifiers & Qt::AltModifier)
				{
					RemSelection(firstEntity);
				}
				else
				{
					// if new selection is NULL or is one of already selected items
					// we should change current selection only on phase end
					if(nextEntity == NULL || NULL != curSelections.IntersectedEntity(collisionEntities))
					{
						applyOnPhaseEnd = true;
						lastSelection = nextEntity;
					}
					else
					{
						SetSelection(nextEntity);
					}
				}
			}
		}
	}
	else if(DAVA::UIEvent::PHASE_ENDED == event->phase)
	{
		if(event->tid == DAVA::UIEvent::BUTTON_1)
		{
			if(applyOnPhaseEnd)
			{
				applyOnPhaseEnd = false;
				SetSelection(lastSelection);
			}
		}
	}
}

void SceneSelectionSystem::Draw()
{
	if(curSelections.Size() > 0)
	{
		int oldState = DAVA::RenderManager::Instance()->GetState();
		DAVA::eBlendMode oldBlendSrc = DAVA::RenderManager::Instance()->GetSrcBlend();
		DAVA::eBlendMode oldBlendDst = DAVA::RenderManager::Instance()->GetDestBlend();

		int newState = DAVA::RenderState::STATE_BLEND | DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_WRITE;
		if(!(selectionDrawFlags & SELECTION_NO_DEEP_TEST))
		{
			newState |= DAVA::RenderState::STATE_DEPTH_TEST;
		}

		DAVA::RenderManager::Instance()->SetState(newState);
		DAVA::RenderManager::Instance()->SetBlendMode(DAVA::BLEND_SRC_ALPHA, DAVA::BLEND_ONE_MINUS_SRC_ALPHA);

		for (DAVA::uint32 i = 0; i < curSelections.Size(); i++)
		{
			DAVA::AABBox3 selectionBox = curSelections.GetBbox(i);

			// draw selection share
			if(selectionDrawFlags & SELECTION_DRAW_SHAPE)
			{
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f));
				DAVA::RenderHelper::Instance()->DrawBox(selectionBox);
			}
			// draw selection share
			else if(selectionDrawFlags & SELECTION_DRAW_CORNERS)
			{
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f));
				DAVA::RenderHelper::Instance()->DrawCornerBox(selectionBox);
			}

			// fill selection shape
			if(selectionDrawFlags & SELECTION_FILL_SHAPE)
			{
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 1.0f, 0.15f));
				DAVA::RenderHelper::Instance()->FillBox(selectionBox);
			}
		}

		DAVA::RenderManager::Instance()->SetBlendMode(oldBlendSrc, oldBlendDst);
		DAVA::RenderManager::Instance()->SetState(oldState);
	}
}

void SceneSelectionSystem::SetSelection(DAVA::Entity *entity)
{
	curSelections.Clear();

	if(NULL != entity)
	{
		EntityGroupItem selectableItem = GetSelectableEntity(entity);
		curSelections.Add(selectableItem);
	}

	UpdateHoodPos();
}

void SceneSelectionSystem::AddSelection(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
		EntityGroupItem selectableItem = GetSelectableEntity(entity);
		curSelections.Add(selectableItem);
	}

	UpdateHoodPos();
}

void SceneSelectionSystem::RemSelection(DAVA::Entity *entity)
{
	curSelections.Rem(entity);

	UpdateHoodPos();
}

const EntityGroup* SceneSelectionSystem::GetSelection() const
{
	return &curSelections;
}

void SceneSelectionSystem::SelectedItemsWereModified()
{
	// don't change selection on phase end
	applyOnPhaseEnd = false;
}

void SceneSelectionSystem::SetPivotPoint(int pp)
{
	curPivotPoint = pp;
}

int SceneSelectionSystem::GetPivotPoint() const
{
	return curPivotPoint;
}

void SceneSelectionSystem::UpdateHoodPos() const
{
	if(curSelections.Size() > 0)
	{
		DAVA::Vector3 p;
		switch (curPivotPoint)
		{
		case SELECTION_ENTITY_CENTER:
			p = curSelections.GetBbox(0).GetCenter();
			break;
		default:
			p = curSelections.GetCommonBbox().GetCenter();
			break;
		}

		hoodSystem->SetPosition(p);
		hoodSystem->Show();
	}
	else
	{
		hoodSystem->SetPosition(DAVA::Vector3(0, 0, 0));
		hoodSystem->Hide();
	}
}

EntityGroup SceneSelectionSystem::GetSelecetableFromCollision(const EntityGroup *collisionEntities)
{
	EntityGroup ret;

	if(NULL != collisionEntities)
	{
		for(size_t i = 0; i < collisionEntities->Size(); ++i)
		{
			DAVA::Entity *entity = collisionEntities->GetEntity(i);
			EntityGroupItem item = GetSelectableEntity(entity);

			ret.Add(item);
		}
	}

	return ret;
}

EntityGroupItem SceneSelectionSystem::GetSelectableEntity(DAVA::Entity* entity)
{
	EntityGroupItem ret;
	DAVA::Entity *solidEntity = entity;
	
	if(NULL != entity)
	{
		ret.bbox.AddAABBox(collisionSystem->GetBoundingBox(entity));

		// find real solid entity
		solidEntity = entity;
		while(NULL != solidEntity && !solidEntity->GetSolid())
		{
			solidEntity = solidEntity->GetParent();
			ret.bbox.AddAABBox(collisionSystem->GetBoundingBox(solidEntity));
		}

		// if there is no solid entity, try to find lod parent entity
		if(NULL == solidEntity)
		{
			// find entity that has lod component
			solidEntity = entity;
			while(NULL != solidEntity && NULL == solidEntity->GetComponent(DAVA::Component::LOD_COMPONENT))
			{
				solidEntity = solidEntity->GetParent();
			}
		}

		// still not found?
		if(NULL == solidEntity)
		{
			// let it current entity bo be tread as solid
			solidEntity = entity;
		}

		ret.entity = entity;
		ret.solidEntity = solidEntity;
	}

	return ret;
}