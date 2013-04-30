#include "Qt/Scene/SceneEditorProxy.h"
#include "Qt/Scene/System/CameraSystem.h"
#include "Qt/Scene/System/GridSystem.h"
#include "Qt/Scene/System/CollisionSystem.h"
#include "Qt/Scene/System/SelectionSystem.h"
#include "Qt/Scene/System/ModifSystem.h"
#include "Qt/Scene/System/HoodSystem.h"

// framework
#include "Scene3D/SceneFileV2.h"

SceneEditorProxy::SceneEditorProxy()
	: Scene()
{
	cameraSystem = new SceneCameraSystem(this);
	AddSystem(cameraSystem, 0);

	gridSystem = new SceneGridSystem(this);
	AddSystem(gridSystem, 0);

	collisionSystem = new SceneCollisionSystem(this);
	AddSystem(collisionSystem, 0);

	hoodSystem = new HoodSystem(this, cameraSystem);
	AddSystem(hoodSystem, 0);

	selectionSystem = new SceneSelectionSystem(this, collisionSystem, hoodSystem);
	AddSystem(selectionSystem, 0);

	modifSystem = new EntityModificationSystem(this, collisionSystem, cameraSystem, hoodSystem);
	AddSystem(modifSystem, 0);
}

SceneEditorProxy::~SceneEditorProxy()
{ }

bool SceneEditorProxy::Load(const DAVA::FilePath &path)
{
	bool ret = false;

	Entity * rootNode = GetRootNode(path);
	if(rootNode)
	{
		ret = true;

		DAVA::Vector<DAVA::Entity*> tmpEntities;
		int entitiesCount = rootNode->GetChildrenCount();

		tmpEntities.reserve(entitiesCount);

		// remember all child pointers, but don't add them to scene in this cycle
		// because when entity is adding it is automatically removing from its old hierarchy
		for (DAVA::int32 i = 0; i < entitiesCount; ++i)
		{
			tmpEntities.push_back(rootNode->GetChild(i));
		}

		// now we can safely add entities into our hierarchy
		for (DAVA::int32 i = 0; i < (DAVA::int32) tmpEntities.size(); ++i)
		{
			AddNode(tmpEntities[i]);
		}

		curScenePath = path;
	}

	return ret;
}

bool SceneEditorProxy::Save(const DAVA::FilePath &path)
{
	bool ret = false;

	DAVA::SceneFileV2 *file = new DAVA::SceneFileV2();
	file->EnableDebugLog(false);

	DAVA::SceneFileV2::eError err = file->SaveScene(path, this);
	ret = (DAVA::SceneFileV2::ERROR_NO_ERROR == err);

	if(ret)
	{
		curScenePath = path;
	}

	SafeRelease(file);

	return ret;
}

bool SceneEditorProxy::Save()
{
	return Save(curScenePath);
}

DAVA::FilePath SceneEditorProxy::GetScenePath()
{
	return curScenePath;
}

void SceneEditorProxy::SetScenePath(const DAVA::FilePath &newScenePath)
{
	curScenePath = newScenePath;
}

void SceneEditorProxy::Update(float timeElapsed)
{
	Scene::Update(timeElapsed);

	gridSystem->Update(timeElapsed);
	cameraSystem->Update(timeElapsed);
	collisionSystem->Update(timeElapsed);
	hoodSystem->Update(timeElapsed);
	selectionSystem->Update(timeElapsed);
	modifSystem->Update(timeElapsed);
}

void SceneEditorProxy::PostUIEvent(DAVA::UIEvent *event)
{
	gridSystem->ProcessUIEvent(event);
	cameraSystem->ProcessUIEvent(event);
	collisionSystem->ProcessUIEvent(event);
	hoodSystem->ProcessUIEvent(event);
	selectionSystem->ProcessUIEvent(event);
	modifSystem->ProcessUIEvent(event);
}

void SceneEditorProxy::SetViewportRect(const DAVA::Rect &newViewportRect)
{
	cameraSystem->SetViewportRect(newViewportRect);
}

void SceneEditorProxy::Draw()
{
	Scene::Draw();

	gridSystem->Draw();
	cameraSystem->Draw();
	collisionSystem->Draw();
	selectionSystem->Draw();
	hoodSystem->Draw();
	modifSystem->Draw();
}
