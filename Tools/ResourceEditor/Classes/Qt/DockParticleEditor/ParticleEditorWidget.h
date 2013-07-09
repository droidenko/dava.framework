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

#ifndef __ResourceEditorQt__ParticleEditorWidget__
#define __ResourceEditorQt__ParticleEditorWidget__

#include <QScrollArea>

#include <DAVAEngine.h>
#include "ParticleEffectPropertiesWidget.h"
#include "ParticleEmitterPropertiesWidget.h"
#include "ParticlesEditorQT/Nodes/LayerParticleEditorNode.h"

class EmitterLayerWidget;
class LayerForceWidget;

using namespace DAVA;

class ParticleEditorWidget: public QScrollArea
{
    Q_OBJECT
    
public:
    explicit ParticleEditorWidget(QWidget *parent = 0);
    ~ParticleEditorWidget();
	
protected slots:
	void OnEffectSelected(Entity* effectNode);
	void OnEmitterSelected(Entity* emitterNode, BaseParticleEditorNode* editorNode);
    void OnLayerSelected(Entity* emitterNode, ParticleLayer* layer, BaseParticleEditorNode* editorNode, bool forceRefresh);
    void OnForceSelected(Entity* emitterNode, ParticleLayer* layer, int32 forceIndex, BaseParticleEditorNode* editorNode);

	void OnUpdate();
	void OnValueChanged();
	void OnNodeDeselected(BaseParticleEditorNode* particleEditorNode);
	
signals:
	void ChangeVisible(bool bVisible);
	void ValueChanged();
	
private:
	void DeleteOldWidget();
	void UpdateParticleEditorWidgets();
	
	// Update the visible timelines for the particular Particle Emitter elements.
	void UpdateVisibleTimelinesForParticleEmitter();
	
	// Update visible widgets for the layer.
	void UpdateWidgetsForLayer();

private:
	ParticleEffectPropertiesWidget* effectPropertiesWidget;
	EmitterLayerWidget* emitterLayerWidget;
	LayerForceWidget* layerForceWidget;
	ParticleEmitterPropertiesWidget* emitterPropertiesWidget;
};

#endif /* defined(__ResourceEditorQt__ParticleEditorWidget__) */
