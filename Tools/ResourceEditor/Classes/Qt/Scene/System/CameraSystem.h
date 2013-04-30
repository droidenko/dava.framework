#ifndef __SCENE_CAMERA_SYSTEM_H__
#define __SCENE_CAMERA_SYSTEM_H__

#include "Entity/SceneSystem.h"
#include "Render/Highlevel/Camera.h"
#include "UI/UIEvent.h"

class SceneCameraSystem : public DAVA::SceneSystem
{
	friend class SceneEditorProxy;

public:
	SceneCameraSystem(DAVA::Scene * scene);
	~SceneCameraSystem();

	DAVA::Vector3 GetPointDirection(const DAVA::Vector2 &point);
	DAVA::Vector3 GetCameraPosition();

	void SetMoveSeep(DAVA::float32 speed);
	DAVA::float32 GetMoveSpeed();

	void SetViewportRect(const DAVA::Rect &rect);
	const DAVA::Rect GetViewportRect();

	DAVA::Vector2 GetScreenPos(const DAVA::Vector3 &pos3);

protected:
	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();

protected:
	DAVA::Rect viewportRect;

	DAVA::float32 curSpeed;
	DAVA::Camera* curSceneCamera;

	DAVA::Vector2 rotateStartPoint;
	DAVA::Vector2 rotateStopPoint;

	DAVA::float32 curViewAngleZ, curViewAngleY;
	const DAVA::float32 maxViewAngle;

	void ProcessKeyboardMove(float timeElapsed);

	void RecalcCameraViewAngles();
	void MouseMoveCameraPosition();
	void MouseMoveCameraDirection();
	void MouseMoveCameraPosAndDirByLockedPoint(const DAVA::Vector3 &point);
};

#endif
