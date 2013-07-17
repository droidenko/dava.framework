/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Scene/SceneEditor2.h"
#include "Scene/SceneSignals.h"

// framework
#include "Scene3D/SceneFileV2.h"

SceneEditor2::SceneEditor2()
	: Scene()
{
	EditorCommandNotify *notify = new EditorCommandNotify(this);
	commandStack.SetNotify(notify);
	SafeRelease(notify);

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
	
	particlesSystem = new ParticlesDebugDrawSystem(this);
	AddSystem(particlesSystem, (1 << DAVA::Component::PARTICLE_EFFECT_COMPONENT));

	modifSystem = new EntityModificationSystem(this, collisionSystem, cameraSystem, hoodSystem);
	AddSystem(modifSystem, 0);

	landscapeEditorDrawSystem = new LandscapeEditorDrawSystem(this);
	AddSystem(landscapeEditorDrawSystem, 0);

	heightmapEditorSystem = new HeightmapEditorSystem(this);
	AddSystem(heightmapEditorSystem, 0);

	tilemaskEditorSystem = new TilemaskEditorSystem(this);
	AddSystem(tilemaskEditorSystem, 0);

	customColorsSystem = new CustomColorsSystem(this);
	AddSystem(customColorsSystem, 0);

	visibilityToolSystem = new VisibilityToolSystem(this);
	AddSystem(visibilityToolSystem, 0);

	structureSystem = new StructureSystem(this);
	AddSystem(structureSystem, 0);

	editorLightSystem = new EditorLightSystem(this);
	AddSystem(editorLightSystem, 0);

	SceneSignals::Instance()->EmitOpened(this);
}

SceneEditor2::~SceneEditor2()
{
    RemoveSystem(editorLightSystem, 0);
    SafeDelete(editorLightSystem);

	SceneSignals::Instance()->EmitClosed(this);
}

bool SceneEditor2::Load(const DAVA::FilePath &path)
{
	bool ret = false;

	structureSystem->LockSignals();

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

	structureSystem->Init();
	structureSystem->UnlockSignals();

	SceneSignals::Instance()->EmitLoaded(this);
	return ret;
}

bool SceneEditor2::Save(const DAVA::FilePath &path)
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

	SceneSignals::Instance()->EmitSaved(this);
	return ret;
}

bool SceneEditor2::Save()
{
	return Save(curScenePath);
}

DAVA::FilePath SceneEditor2::GetScenePath()
{
	return curScenePath;
}

void SceneEditor2::SetScenePath(const DAVA::FilePath &newScenePath)
{
	curScenePath = newScenePath;
}

bool SceneEditor2::CanUndo() const
{
	return commandStack.CanUndo();
}

bool SceneEditor2::CanRedo() const
{
	return commandStack.CanRedo();
}

void SceneEditor2::Undo()
{
	commandStack.Undo();
}

void SceneEditor2::Redo()
{
	commandStack.Redo();
}

void SceneEditor2::BeginBatch(const DAVA::String &text)
{
	commandStack.BeginBatch(text);
}

void SceneEditor2::EndBatch()
{
	commandStack.EndBatch();
}

void SceneEditor2::Exec(Command2 *command)
{
	commandStack.Exec(command);
}

void SceneEditor2::Update(float timeElapsed)
{
	Scene::Update(timeElapsed);
	gridSystem->Update(timeElapsed);
	cameraSystem->Update(timeElapsed);
	collisionSystem->Update(timeElapsed);
	hoodSystem->Update(timeElapsed);
	selectionSystem->Update(timeElapsed);
	modifSystem->Update(timeElapsed);
	landscapeEditorDrawSystem->Update(timeElapsed);
	heightmapEditorSystem->Update(timeElapsed);
	tilemaskEditorSystem->Update(timeElapsed);
	customColorsSystem->Update(timeElapsed);
	visibilityToolSystem->Update(timeElapsed);
	structureSystem->Update(timeElapsed);
	particlesSystem->Update(timeElapsed);
	editorLightSystem->Update(timeElapsed);
}

void SceneEditor2::PostUIEvent(DAVA::UIEvent *event)
{
	gridSystem->ProcessUIEvent(event);
	cameraSystem->ProcessUIEvent(event);
	collisionSystem->ProcessUIEvent(event);
	hoodSystem->ProcessUIEvent(event);
	selectionSystem->ProcessUIEvent(event);
	modifSystem->ProcessUIEvent(event);
	heightmapEditorSystem->ProcessUIEvent(event);
	tilemaskEditorSystem->ProcessUIEvent(event);
	customColorsSystem->ProcessUIEvent(event);
	visibilityToolSystem->ProcessUIEvent(event);
	structureSystem->ProcessUIEvent(event);
	particlesSystem->ProcessUIEvent(event);
}

void SceneEditor2::SetViewportRect(const DAVA::Rect &newViewportRect)
{
	cameraSystem->SetViewportRect(newViewportRect);
}

void SceneEditor2::Draw()
{
	Scene::Draw();

	gridSystem->Draw();
	cameraSystem->Draw();
	collisionSystem->Draw();
	selectionSystem->Draw();
	hoodSystem->Draw();
	modifSystem->Draw();
	structureSystem->Draw();
	tilemaskEditorSystem->Draw();
	particlesSystem->Draw();
}

void SceneEditor2::EditorCommandProcess(const Command2 *command, bool redo)
{
	gridSystem->ProcessCommand(command, redo);
	cameraSystem->ProcessCommand(command, redo);
	collisionSystem->ProcessCommand(command, redo);
	selectionSystem->ProcessCommand(command, redo);
	hoodSystem->ProcessCommand(command, redo);
	modifSystem->ProcessCommand(command, redo);
	structureSystem->ProcessCommand(command, redo);
	particlesSystem->ProcessCommand(command, redo);
	editorLightSystem->ProcessCommand(command, redo);
}

void SceneEditor2::AddEditorEntity( Entity *editorEntity )
{
	if(GetChildrenCount())
	{
		InsertBeforeNode(editorEntity, GetChild(0));
	}
	else
	{
		AddNode(editorEntity);
	}
}

SceneEditor2::EditorCommandNotify::EditorCommandNotify(SceneEditor2 *_editor)
	: editor(_editor)
{ }

void SceneEditor2::EditorCommandNotify::Notify(const Command2 *command, bool redo)
{
	if(NULL != editor)
	{
		editor->EditorCommandProcess(command, redo);
	}
}
