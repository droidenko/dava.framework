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

#include "ParticleEditorWidget.h"
#include "EmitterLayerWidget.h"
#include "LayerForceWidget.h"
#include "ParticlesEditorController.h"
#include "ui_mainwindow.h"
#include <QScrollBar>

ParticleEditorWidget::ParticleEditorWidget(QWidget *parent/* = 0*/) :
	QScrollArea(parent)
{
	setWidgetResizable(true);
	
	emitterLayerWidget = NULL;
	layerForceWidget = NULL;
	emitterPropertiesWidget = NULL;
	effectPropertiesWidget = NULL;

	connect(ParticlesEditorController::Instance(),
			SIGNAL(EffectSelected(Entity*)),
			this,
			SLOT(OnEffectSelected(Entity*)));
	connect(ParticlesEditorController::Instance(),
			SIGNAL(EmitterSelected(Entity*, BaseParticleEditorNode*)),
			this,
			SLOT(OnEmitterSelected(Entity*, BaseParticleEditorNode*)));
	connect(ParticlesEditorController::Instance(),
			SIGNAL(LayerSelected(Entity*, ParticleLayer*, BaseParticleEditorNode*, bool)),
			this,
			SLOT(OnLayerSelected(Entity*, ParticleLayer*, BaseParticleEditorNode*, bool)));
	connect(ParticlesEditorController::Instance(),
			SIGNAL(ForceSelected(Entity*, ParticleLayer*, int32, BaseParticleEditorNode*)),
			this,
			SLOT(OnForceSelected(Entity*, ParticleLayer*, int32, BaseParticleEditorNode*)));

	connect(ParticlesEditorController::Instance(),
			SIGNAL(NodeDeselected(BaseParticleEditorNode*)),
			this,
			SLOT(OnNodeDeselected(BaseParticleEditorNode*)));
}

ParticleEditorWidget::~ParticleEditorWidget()
{
	DeleteOldWidget();
}

void ParticleEditorWidget::DeleteOldWidget()
{
	SAFE_DELETE(emitterLayerWidget);
	SAFE_DELETE(layerForceWidget);
	SAFE_DELETE(emitterPropertiesWidget);
	SAFE_DELETE(effectPropertiesWidget);
}

void ParticleEditorWidget::OnEffectSelected(Entity* effectNode)
{
	ParticleEffectComponent* effect = NULL;
	if (effectNode)
	{
		effect = cast_if_equal<ParticleEffectComponent*>(effectNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
		if (!effect)
		{
			return;
		}
		
		if (effectPropertiesWidget && effectPropertiesWidget->GetEffect() == effect)
		{
			return;
		}
	}

	DeleteOldWidget();
	if (!effect)
	{
		emit ChangeVisible(false);
		return;
	}
	
	emit ChangeVisible(true);
	effectPropertiesWidget = new ParticleEffectPropertiesWidget(this);
	effectPropertiesWidget->Init(effect);

	setWidget(effectPropertiesWidget);
}

void ParticleEditorWidget::OnEmitterSelected(Entity* emitterNode, BaseParticleEditorNode* editorNode)
{
	ParticleEmitter* emitter = NULL;
	if (emitterNode)
	{
		emitter = GetEmitter(emitterNode);
		if (!emitter)
		{
			return;
		}

		if (emitterPropertiesWidget &&
			emitterPropertiesWidget->GetEmitter() == emitter)
			return;
	}
	
	DeleteOldWidget();
	
	if (!emitterNode)
	{
		emit ChangeVisible(false);
		return;
	}
	
	emit ChangeVisible(true);
	if (!emitter)
		return;

	emitterPropertiesWidget = new ParticleEmitterPropertiesWidget(this);
	emitterPropertiesWidget->Init(emitter, true);

	setWidget(emitterPropertiesWidget);
	connect(emitterPropertiesWidget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));

	if (editorNode)
	{
		KeyedArchive* stateProps = editorNode->GetExtraData();
		emitterPropertiesWidget->RestoreVisualState(stateProps);
		
		int32 scrollValue = stateProps->GetInt32("EDITOR_SCROLL_VALUE", 0);
		verticalScrollBar()->setValue(scrollValue);
	}

	UpdateParticleEditorWidgets();
}

void ParticleEditorWidget::OnLayerSelected(Entity* emitterNode, ParticleLayer* layer, BaseParticleEditorNode* editorNode, bool forceRefresh)
{
	ParticleEmitter* emitter = NULL;
	if (emitterNode)
	{
		emitter =  GetEmitter(emitterNode);
		if (!emitter)
		{
			return;
		}
		if (!forceRefresh && emitterLayerWidget &&
			emitterLayerWidget->GetLayer() == layer &&
			emitterLayerWidget->GetEmitter() == emitter &&
			!forceRefresh)
			return;
	}

	DeleteOldWidget();
	
	if (!emitterNode || !layer)
	{
		emit ChangeVisible(false);
		return;
	}

	emit ChangeVisible(true);
	if (!emitter)
		return;

	emitterLayerWidget = new EmitterLayerWidget(this);
	emitterLayerWidget->Init(emitter, layer, true);

	setWidget(emitterLayerWidget);
	connect(emitterLayerWidget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));

	if (editorNode)
	{
		KeyedArchive* stateProps = editorNode->GetExtraData();
		emitterLayerWidget->RestoreVisualState(stateProps);
		
		int32 scrollValue = stateProps->GetInt32("EDITOR_SCROLL_VALUE", 0);
		verticalScrollBar()->setValue(scrollValue);
	}

	UpdateParticleEditorWidgets();
}

void ParticleEditorWidget::OnForceSelected(Entity* emitterNode, ParticleLayer* layer, int32 forceIndex, BaseParticleEditorNode* editorNode)
{
	ParticleEmitter* emitter = NULL;
	if (emitterNode)
	{
		emitter =  GetEmitter(emitterNode);
		if (!emitter)
		{
			return;
		}
		if (layerForceWidget &&
			layerForceWidget->GetLayer() == layer &&
			layerForceWidget->GetForceIndex() == forceIndex &&
			layerForceWidget->GetEmitter() == emitter)
			return;
	}

	DeleteOldWidget();
	
	if (!emitterNode || !layer)
	{
		emit ChangeVisible(false);
		return;
	}
	
	emit ChangeVisible(true);
	if (!emitter)
		return;
	
	layerForceWidget = new LayerForceWidget(this);
	layerForceWidget->Init(emitter, layer, forceIndex, true);

	setWidget(layerForceWidget);
	connect(layerForceWidget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));

	if (editorNode)
	{
		KeyedArchive* stateProps = editorNode->GetExtraData();
		layerForceWidget->RestoreVisualState(stateProps);

		int32 scrollValue = stateProps->GetInt32("EDITOR_SCROLL_VALUE", 0);
		verticalScrollBar()->setValue(scrollValue);
	}

	UpdateParticleEditorWidgets();
}

void ParticleEditorWidget::OnNodeDeselected(BaseParticleEditorNode* particleEditorNode)
{
	if (!particleEditorNode)
		return;

	KeyedArchive* stateProps = particleEditorNode->GetExtraData();

	if (emitterLayerWidget)
		emitterLayerWidget->StoreVisualState(stateProps);
	if (layerForceWidget)
		layerForceWidget->StoreVisualState(stateProps);
	if (emitterPropertiesWidget)
		emitterPropertiesWidget->StoreVisualState(stateProps);

	stateProps->SetInt32("EDITOR_SCROLL_VALUE", verticalScrollBar()->value());
}

void ParticleEditorWidget::OnUpdate()
{
	if (emitterLayerWidget)
		emitterLayerWidget->Update();
	if (layerForceWidget)
		layerForceWidget->Update();
	if (emitterPropertiesWidget)
		emitterPropertiesWidget->Update();
}

void ParticleEditorWidget::OnValueChanged()
{
	emit ValueChanged();
	
	// Update the particle editor widgets when the value on the emitter layer is changed.
	UpdateParticleEditorWidgets();
}

void ParticleEditorWidget::UpdateParticleEditorWidgets()
{
	if (emitterPropertiesWidget && emitterPropertiesWidget->GetEmitter())
	{
		UpdateVisibleTimelinesForParticleEmitter();
		return;
	}
	
	if (emitterLayerWidget && emitterLayerWidget->GetLayer())
	{
		UpdateWidgetsForLayer();
		return;
	}
}

void ParticleEditorWidget::UpdateVisibleTimelinesForParticleEmitter()
{
	// Safety check.
	if (!emitterPropertiesWidget || !emitterPropertiesWidget->GetEmitter())
	{
		return;
	}

	// Update the visibility of particular timelines based on the emitter type.
	bool radiusTimeLineVisible = false;
	bool sizeTimeLineVisible = false;

	bool emissionVectorTimeLineVisible = true;

	switch (emitterPropertiesWidget->GetEmitter()->emitterType)
	{
		case DAVA::ParticleEmitter::EMITTER_ONCIRCLE:
		case DAVA::ParticleEmitter::EMITTER_SHOCKWAVE:
		{
			radiusTimeLineVisible = true;
			emissionVectorTimeLineVisible = false;
			break;
		}
			
		case DAVA::ParticleEmitter::EMITTER_RECT:
		{
			sizeTimeLineVisible = true;
		}
		
		default:
		{
			break;
		}
	}
	
	emitterPropertiesWidget->GetEmitterRadiusTimeline()->setVisible(radiusTimeLineVisible);
	emitterPropertiesWidget->GetEmitterSizeTimeline()->setVisible(sizeTimeLineVisible);
	emitterPropertiesWidget->GetEmissionVectorTimeline()->setVisible(emissionVectorTimeLineVisible);
}

void ParticleEditorWidget::UpdateWidgetsForLayer()
{
	if (!emitterLayerWidget || !emitterLayerWidget->GetLayer())
	{
		return;
	}
	
	bool isSuperemitter = (emitterLayerWidget->GetLayer()->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES);
	emitterLayerWidget->SetSuperemitterMode(isSuperemitter);
}
