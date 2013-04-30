#include "ParticleEditorCommands.h"
#include "DAVAEngine.h"
#include "SceneEditor/SceneEditorScreenMain.h"
#include "SceneEditor/EditorBodyControl.h"
#include "SceneEditor/SceneGraph.h"
#include "SceneEditor/EditorSettings.h"

#include "ParticlesEditorQT/Nodes/BaseParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/EmitterParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/LayerParticleEditorNode.h"

#include "Qt/Main/QtUtils.h"
#include "Qt/Main/QtMainWindowHandler.h"
#include "Qt/Scene/SceneData.h"
#include "Qt/Scene/SceneDataManager.h"
#include "Qt/DockParticleEditor/ParticlesEditorController.h"

#include <QFileDialog>
#include <QString>

#include "Scene3D/Components/ParticleEffectComponent.h"

using namespace DAVA;

/////////////////////////////////////////////////////////////////////////////////////////////
// Yuri Coder, 03/12/2012. New commands for Particle Editor QT.

CommandUpdateEffect::CommandUpdateEffect(ParticleEffectComponent* particleEffect):
Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
	this->particleEffect = particleEffect;
}

void CommandUpdateEffect::Init(float32 playbackSpeed)
{
	this->playbackSpeed = playbackSpeed;
}

void CommandUpdateEffect::Execute()
{
	DVASSERT(particleEffect);
	particleEffect->SetPlaybackSpeed(playbackSpeed);
}

CommandUpdateEmitter::CommandUpdateEmitter(ParticleEmitter* emitter):
	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
	this->emitter = emitter;
}

void CommandUpdateEmitter::Init(ParticleEmitter::eType emitterType,
								RefPtr<PropertyLine<float32> > emissionRange,
								RefPtr<PropertyLine<Vector3> > emissionVector,
								RefPtr<PropertyLine<float32> > radius,
								RefPtr<PropertyLine<Color> > colorOverLife,
								RefPtr<PropertyLine<Vector3> > size,
								float32 life,
								float32 playbackSpeed)
{
	this->emitterType = emitterType;
	this->emissionRange = emissionRange;
	this->emissionVector = emissionVector;
	this->radius = radius;
	this->colorOverLife = colorOverLife;
	this->size = size;
	this->life = life;
	this->playbackSpeed = playbackSpeed;
}

void CommandUpdateEmitter::Execute()
{
	DVASSERT(emitter);

	emitter->emitterType = emitterType;
	emitter->emissionRange = emissionRange;
	emitter->emissionVector = emissionVector;
	emitter->radius = radius;
	emitter->colorOverLife = colorOverLife;
	emitter->size = size;
	emitter->SetLifeTime(life);
	emitter->SetPlaybackSpeed(playbackSpeed);
}

CommandUpdateParticleLayer::CommandUpdateParticleLayer(ParticleEmitter* emitter, ParticleLayer* layer) :
	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
	this->emitter = emitter;
	this->layer = layer;
}

void CommandUpdateParticleLayer::Init(const QString& layerName,
									  ParticleLayer::eType layerType,
									  bool isDisabled,
									  bool additive,
									  bool isLong,
									  Sprite* sprite,
									  RefPtr< PropertyLine<float32> > life,
									  RefPtr< PropertyLine<float32> > lifeVariation,
									  RefPtr< PropertyLine<float32> > number,
									  RefPtr< PropertyLine<float32> > numberVariation,
									  RefPtr< PropertyLine<Vector2> > size,
									  RefPtr< PropertyLine<Vector2> > sizeVariation,
									  RefPtr< PropertyLine<Vector2> > sizeOverLife,
									  RefPtr< PropertyLine<float32> > velocity,
									  RefPtr< PropertyLine<float32> > velocityVariation,
									  RefPtr< PropertyLine<float32> > velocityOverLife,
									  RefPtr< PropertyLine<float32> > spin,
									  RefPtr< PropertyLine<float32> > spinVariation,
									  RefPtr< PropertyLine<float32> > spinOverLife,
									  RefPtr< PropertyLine<Color> > colorRandom,
									  RefPtr< PropertyLine<float32> > alphaOverLife,
									  RefPtr< PropertyLine<Color> > colorOverLife,
									  RefPtr< PropertyLine<float32> > angle,
									  RefPtr< PropertyLine<float32> > angleVariation,

									  float32 startTime,
									  float32 endTime,
									  bool frameOverLifeEnabled,
									  float32 frameOverLifeFPS)
{
	this->layerName = layerName;
	this->layerType = layerType;
	this->isDisabled = isDisabled;
	this->additive = additive;
	this->isLong = isLong;
	this->sprite = sprite;
	this->life = life;
	this->lifeVariation = lifeVariation;
	this->number = number;
	this->numberVariation = numberVariation;
	this->size = size;
	this->sizeVariation = sizeVariation;
	this->sizeOverLife = sizeOverLife;
	this->velocity = velocity;
	this->velocityVariation = velocityVariation;
	this->velocityOverLife = velocityOverLife;
	this->spin = spin;
	this->spinVariation = spinVariation;
	this->spinOverLife = spinOverLife;

	this->colorRandom = colorRandom;
	this->alphaOverLife = alphaOverLife;
	this->colorOverLife = colorOverLife;
	this->frameOverLife = frameOverLife;
	this->angle = angle;
	this->angleVariation = angleVariation;

	this->startTime = startTime;
	this->endTime = endTime;
	this->frameOverLifeEnabled = frameOverLifeEnabled;
	this->frameOverLifeFPS = frameOverLifeFPS;
}


void CommandUpdateParticleLayer::Execute()
{
	layer->layerName = layerName.toAscii().constData();
	layer->isDisabled = isDisabled;
	layer->SetAdditive(additive);
	layer->SetLong(isLong);
	layer->life = life;
	layer->lifeVariation = lifeVariation;
	layer->number = number;
	layer->numberVariation = numberVariation;
	layer->size = size;
	layer->sizeVariation = sizeVariation;
	layer->sizeOverLifeXY = sizeOverLife;
	layer->velocity = velocity;
	layer->velocityVariation = velocityVariation;
	layer->velocityOverLife = velocityOverLife;
	layer->spin = spin;
	layer->spinVariation = spinVariation;
	layer->spinOverLife = spinOverLife;

	layer->colorRandom = colorRandom;
	layer->alphaOverLife = alphaOverLife;
	layer->colorOverLife = colorOverLife;
	
	layer->frameOverLifeEnabled = frameOverLifeEnabled;
	layer->frameOverLifeFPS = frameOverLifeFPS;

	layer->angle = angle;
	layer->angleVariation = angleVariation;

	layer->startTime = startTime;
	layer->endTime = endTime;

	// This code must be after layer->frameOverlife set call, since setSprite
	// may change the frames.
	if (layer->GetSprite() != sprite)
	{
		emitter->Stop();
		layer->SetSprite(sprite);
		emitter->Play();
	}
	
	// The same is for emitter type.
	if (layer->type != layerType)
	{
		emitter->Stop();
		layer->type = layerType;
		emitter->Play();
	}
	
	// "IsLong" flag.
	if (layer->IsLong() != isLong)
	{
		emitter->Stop();
		layer->SetLong(isLong);
		emitter->Play();
	}


	SceneDataManager::Instance()->RefreshParticlesLayer(layer);
}

CommandUpdateParticleLayerTime::CommandUpdateParticleLayerTime(ParticleLayer* layer) :
	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
	this->layer = layer;
}

void CommandUpdateParticleLayerTime::Init(float32 startTime, float32 endTime)
{
	this->startTime = startTime;
	this->endTime = endTime;
}

void CommandUpdateParticleLayerTime::Execute()
{
	layer->startTime = startTime;
	layer->endTime = endTime;
}

CommandUpdateParticleLayerEnabled::CommandUpdateParticleLayerEnabled(ParticleLayer* layer, bool isEnabled) :
	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
	this->layer = layer;
	this->isEnabled = isEnabled;
}

void CommandUpdateParticleLayerEnabled::Execute()
{
	if (this->layer)
	{
		this->layer->isDisabled = !isEnabled;
		ParticlesEditorController::Instance()->RefreshSelectedNode(true);
	}
}

CommandUpdateParticleForce::CommandUpdateParticleForce(ParticleLayer* layer, uint32 forceId) :
	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
	this->layer = layer;
	this->forceId = forceId;
}

void CommandUpdateParticleForce::Init(RefPtr< PropertyLine<Vector3> > force,
									  RefPtr< PropertyLine<Vector3> > forcesVariation,
									RefPtr< PropertyLine<float32> > forcesOverLife)
{
	this->force = force;
	this->forcesVariation = forcesVariation;
	this->forcesOverLife = forcesOverLife;
}

void CommandUpdateParticleForce::Execute()
{
	layer->UpdateForce(forceId, force, forcesVariation, forcesOverLife);
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Yuri Coder, 03/12/2012. New commands for Particle Editor QT.

CommandAddParticleEmitter::CommandAddParticleEmitter() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandAddParticleEmitter::Execute()
{
    // This command is done through Main Window to reuse the existing code.
    QtMainWindowHandler::Instance()->CreateParticleEmitterNode();
}


CommandStartStopParticleEffect::CommandStartStopParticleEffect(bool isStart) :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
    this->isStart = isStart;
}

void CommandStartStopParticleEffect::Execute()
{
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    EffectParticleEditorNode* effectNode = dynamic_cast<EffectParticleEditorNode*>(selectedNode);
    if (!effectNode || !effectNode->GetRootNode())
    {
        return;
    }
    ParticleEffectComponent* effectComponent = effectNode->GetParticleEffectComponent();
    if (this->isStart)
    {
        effectComponent->Start();
    }
    else
    {
        effectComponent->Stop();
    }
}

CommandRestartParticleEffect::CommandRestartParticleEffect() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandRestartParticleEffect::Execute()
{
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    EffectParticleEditorNode* effectNode = dynamic_cast<EffectParticleEditorNode*>(selectedNode);
    if (!effectNode || !effectNode->GetRootNode() || !effectNode->GetRootNode())
    {
        return;
    }
    
	ParticleEffectComponent * effectComponent = effectNode->GetParticleEffectComponent();
	DVASSERT(effectComponent);
    effectComponent->Restart();
}

CommandAddParticleEmitterLayer::CommandAddParticleEmitterLayer() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandAddParticleEmitterLayer::Execute()
{
    // Need to know selected Particle Emitter Layers Root.
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    EmitterParticleEditorNode* emitterNode = dynamic_cast<EmitterParticleEditorNode*>(selectedNode);
    if (!emitterNode)
    {
        return;
    }

	ParticleEffectComponent * effectComponent = emitterNode->GetParticleEffectComponent();
	DVASSERT(effectComponent);
    effectComponent->Stop();
	
    // Lets select this node when the tree will be rebuilt.
    LayerParticleEditorNode* layerNode = ParticlesEditorController::Instance()->AddParticleLayerToNode(emitterNode);
    if (layerNode)
    {
        layerNode->SetMarkedToSelection(true);
    }

	effectComponent->Restart();

    // Update the scene graph.
    QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandRemoveParticleEmitterLayer::CommandRemoveParticleEmitterLayer() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandRemoveParticleEmitterLayer::Execute()
{
    // Need to know selected Layer Node to remove it.
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    LayerParticleEditorNode* layerNode = dynamic_cast<LayerParticleEditorNode*>(selectedNode);
    if (!layerNode || !layerNode->GetRootNode())
    {
        return;
    }

    // Mark the "parent" Emitter Node to selection.
    layerNode->GetEmitterEditorNode()->SetMarkedToSelection(true);

	ParticleEffectComponent * effectComponent = layerNode->GetParticleEffectComponent();

	if (effectComponent)
	{
		effectComponent->Stop();
	}

    ParticlesEditorController::Instance()->RemoveParticleLayerNode(layerNode);

	if (effectComponent)
	{
		effectComponent->Restart();
	}
	
    // Update the scene graph.
    QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandCloneParticleEmitterLayer::CommandCloneParticleEmitterLayer() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandCloneParticleEmitterLayer::Execute()
{
    // Need to know selected Layer Node to remove it.
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    LayerParticleEditorNode* layerNode = dynamic_cast<LayerParticleEditorNode*>(selectedNode);
    if (!layerNode)
    {
        return;
    }
    
    LayerParticleEditorNode* clonedNode = ParticlesEditorController::Instance()->CloneParticleLayerNode(layerNode);
    if (clonedNode)
    {
        clonedNode->SetMarkedToSelection(true);
    }
    
    // Update the scene graph.
    QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandAddParticleEmitterForce::CommandAddParticleEmitterForce() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandAddParticleEmitterForce::Execute()
{
    // Need to know selected Layer Node to add the Force to.
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    LayerParticleEditorNode* layerNode = dynamic_cast<LayerParticleEditorNode*>(selectedNode);
    if (!layerNode || !layerNode->GetRootNode())
    {
        return;
    }

	ParticleEffectComponent * effectComponent = layerNode->GetParticleEffectComponent();
	DVASSERT(effectComponent);
	effectComponent->Stop();
    ForceParticleEditorNode* forceNode = ParticlesEditorController::Instance()->AddParticleForceToNode(layerNode);
    if (forceNode)
    {
        forceNode->SetMarkedToSelection(true);
    }
	effectComponent->Restart();
	
    // Update the scene graph.
    QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandRemoveParticleEmitterForce::CommandRemoveParticleEmitterForce() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandRemoveParticleEmitterForce::Execute()
{
    // Need to know selected Layer Node to add the Force to.
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    ForceParticleEditorNode* forceNode = dynamic_cast<ForceParticleEditorNode*>(selectedNode);
    if (!forceNode || !forceNode->GetRootNode())
    {
        return;
    }

    // Mark the "parent" Layer Node to selection.
    forceNode->GetLayerEditorNode()->SetMarkedToSelection(true);

	
	ParticleEffectComponent * effectComponent = forceNode->GetParticleEffectComponent();
	if (effectComponent)
	{
		effectComponent->Stop();
	}

    ParticlesEditorController::Instance()->RemoveParticleForceNode(forceNode);

	if (effectComponent)
	{
		effectComponent->Restart();
	}

    // Update the scene graph.
    QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandLoadParticleEmitterFromYaml::CommandLoadParticleEmitterFromYaml() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandLoadParticleEmitterFromYaml::Execute()
{
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    EmitterParticleEditorNode* emitterNode = dynamic_cast<EmitterParticleEditorNode*>(selectedNode);
    if (!emitterNode || !emitterNode->GetEmitterNode())
    {
        return;
    }
    
    QString projectPath = QString(EditorSettings::Instance()->GetParticlesConfigsPath().GetAbsolutePathname().c_str());
	Logger::Debug("Project path: %s", projectPath.toAscii().constData());
	QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open Particle Emitter Yaml file"),
                                                    projectPath, QString("YAML File (*.yaml)"));
	if (filePath.isEmpty())
    {
		return;
    }

    // In case this emitter already has Editor Nodes - remove them before loading.
    ParticlesEditorController::Instance()->CleanupParticleEmitterEditorNode(emitterNode);
    ParticleEmitter* emitter = emitterNode->GetParticleEmitter();

    if(!emitter)
    {
    	return;
    }

    emitter->LoadFromYaml(filePath.toAscii().constData());

	// Perform the validation of the Yaml file loaded.
	String validationMessage;
	if (ParticlesEditorSceneDataHelper::ValidateParticleEmitter(emitter, validationMessage) == false)
	{
		ShowErrorDialog(validationMessage);
	}

    QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandSaveParticleEmitterToYaml::CommandSaveParticleEmitterToYaml(bool forceAskFilename) :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
    this->forceAskFilename = forceAskFilename;
}

void CommandSaveParticleEmitterToYaml::Execute()
{
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    EmitterParticleEditorNode* emitterNode = dynamic_cast<EmitterParticleEditorNode*>(selectedNode);
    if (!emitterNode || !emitterNode->GetEmitterNode())
    {
        return;
    }

    ParticleEmitter * emitter = emitterNode->GetParticleEmitter();
    if (!emitter)
    {
        return;
    }

	FilePath yamlPath = emitter->GetConfigPath();
    if (this->forceAskFilename || yamlPath.IsEmpty() )
    {
        QString projectPath = QString(EditorSettings::Instance()->GetParticlesConfigsPath().GetAbsolutePathname().c_str());
        QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save Particle Emitter YAML file"),
                                                        projectPath, QString("YAML File (*.yaml)"));
 
        if (filePath.isEmpty())
        {
            return;
        }
        
        yamlPath = FilePath(filePath.toAscii().constData());
    }

    emitter->SaveToYaml(yamlPath);
}
