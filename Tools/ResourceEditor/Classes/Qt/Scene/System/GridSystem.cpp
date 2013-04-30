#include "Qt/Scene/System/GridSystem.h"

// framework
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

SceneGridSystem::SceneGridSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
{

}

SceneGridSystem::~SceneGridSystem()
{

}

void SceneGridSystem::Update(float timeElapsed)
{

}

void SceneGridSystem::ProcessUIEvent(DAVA::UIEvent *event)
{

}

void SceneGridSystem::Draw()
{
	const DAVA::float32 GRIDMAX = 500.0f;
	const DAVA::float32 GRIDSTEP = 10.0f;
	DAVA::RenderManager* rm = DAVA::RenderManager::Instance();
	DAVA::RenderHelper* rh = DAVA::RenderHelper::Instance();

	DAVA::uint32 oldState = rm->GetState();	

	rm->SetState(DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_WRITE | DAVA::RenderState::STATE_DEPTH_TEST); 
	rm->SetColor(0.4f, 0.4f, 0.4f, 1.0f);
	for(DAVA::float32 x = -GRIDMAX; x <= GRIDMAX; x += GRIDSTEP)
	{
		DAVA::Vector3 v1(x, -GRIDMAX, 0);
		DAVA::Vector3 v2(x, GRIDMAX, 0);

		DAVA::Vector3 v3(-GRIDMAX, x, 0);
		DAVA::Vector3 v4(GRIDMAX, x, 0);

		if (x!= 0.0f)
		{
			rh->DrawLine(v1, v2);
			rh->DrawLine(v3, v4);		
		}
	}

	rm->SetColor(0.0f, 0.0f, 0.0f, 1.0f);
	rh->DrawLine(DAVA::Vector3(-GRIDMAX, 0, 0), DAVA::Vector3(GRIDMAX, 0, 0));
	rh->DrawLine(DAVA::Vector3(0, -GRIDMAX, 0), DAVA::Vector3(0, GRIDMAX, 0));

	rm->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	rm->SetState(oldState);
}
