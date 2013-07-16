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

#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include <QObject>

#include "Scene/EntityGroup.h"
#include "Scene/SceneEditor2.h"

// framework
#include "Base/StaticSingleton.h"
#include "Scene3D/Entity.h"

class SceneEditor2;

class SceneSignals : public QObject, public DAVA::StaticSingleton<SceneSignals>
{
	Q_OBJECT

signals:
	// scene
	void Opened(SceneEditor2 *scene);
	void Closed(SceneEditor2 *scene);

	void Loaded(SceneEditor2 *scene);
	void Saved(SceneEditor2 *scene);

	void Activated(SceneEditor2 *scene);
	void Deactivated(SceneEditor2 *scene);

	void StructureChanged(SceneEditor2 *scene, DAVA::Entity *parent);

	// entities
	void Selected(SceneEditor2 *scene, DAVA::Entity *entity);
	void Deselected(SceneEditor2 *scene, DAVA::Entity *entity);

	// mouse
	void MouseOver(SceneEditor2 *scene, const EntityGroup *entities);
	void MouseOverSelection(SceneEditor2 *scene, const EntityGroup *entities);

	// particles
	void EffectSelected(DAVA::Entity* effectNode);
	void EmitterSelected(DAVA::Entity* emitterNode);
	void LayerSelected(DAVA::ParticleLayer* layer, bool forceRefresh);
	void ForceSelected(DAVA::ParticleLayer* layer, DAVA::int32 forceIndex);

	void UpdateDropperHeight(SceneEditor2* scene, double height);
	void UpdateVisibilityButtonsState(SceneEditor2* scene);
	void NeedSaveCustomColorsTexture(SceneEditor2* scene);

public:
	void EmitOpened(SceneEditor2 *scene) { emit Opened(scene); }
	void EmitClosed(SceneEditor2 *scene) { emit Closed(scene); }

	void EmitLoaded(SceneEditor2 *scene) { emit Loaded(scene); }
	void EmitSaved(SceneEditor2 *scene) { emit Saved(scene); }

	void EmitActivated(SceneEditor2 *scene) { emit Activated(scene); }
	void EmitDeactivated(SceneEditor2 *scene) { emit Deactivated(scene); }

	void EmitStructureChanged(SceneEditor2 *scene, DAVA::Entity *parent) { emit StructureChanged(scene, parent); }

	void EmitSelected(SceneEditor2 *scene, DAVA::Entity *entity) { emit Selected(scene, entity); }
	void EmitDeselected(SceneEditor2 *scene, DAVA::Entity *entity)  { emit Deselected(scene, entity); }

	void EmitUpdateDropperHeight(SceneEditor2* scene, DAVA::float32 height) { emit UpdateDropperHeight(scene, (double)height); };
	void EmitUpdateVisibilityButtonsState(SceneEditor2* scene) { emit UpdateVisibilityButtonsState(scene); };
	void EmitNeedSaveCustomColorsTexture(SceneEditor2* scene) { emit NeedSaveCustomColorsTexture(scene); };

	void EmitMouseOver(SceneEditor2 *scene, const EntityGroup *entities) { emit MouseOver(scene, entities); }
	void EmitMouseOverSelection(SceneEditor2 *scene, const EntityGroup *entities) { emit MouseOverSelection(scene, entities); }

	void EmitEffectSelected(DAVA::Entity* effectNode) { emit EffectSelected(effectNode); };
	void EmitEmitterSelected(DAVA::Entity* emitterNode) { emit EmitterSelected(emitterNode); };
	void EmitLayerSelected(DAVA::ParticleLayer* layer, bool forceRefresh)
	{
		emit LayerSelected(layer, forceRefresh);
	};
	void EmitForceSelected(DAVA::ParticleLayer* layer, DAVA::int32 forceIndex)
	{
		emit ForceSelected(layer, forceIndex);
	};
};

#endif // __SCENE_MANAGER_H__
