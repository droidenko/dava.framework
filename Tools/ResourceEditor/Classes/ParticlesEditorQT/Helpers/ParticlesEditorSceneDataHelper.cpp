//
//  ParticlesEditorSceneDataHelper.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 12/25/12.
//
//

#include "ParticlesEditorSceneDataHelper.h"
#include "Qt/DockParticleEditor/ParticlesEditorController.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

using namespace DAVA;

bool ParticlesEditorSceneDataHelper::AddSceneNode(Entity* node) const
{
	ParticleEmitter * emitter = GetEmitter(node);
    if (emitter)
    {
        ParticlesEditorController::Instance()->AddParticleEmitterNodeToScene(node);
        return true;
    }
    
	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(node->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    if (effectComponent == NULL)
    {
        // Not relevant.
        return false;
    }
	
    // Register in Particles Editor.
    ParticlesEditorController::Instance()->RegisterParticleEffectNode(node);
    return false;
}

void ParticlesEditorSceneDataHelper::RemoveSceneNode(Entity *node) const
{
    // If the Particle Effect node is removed - unregister it.
	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(node->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    if (effectComponent)
    {
        ParticlesEditorController::Instance()->UnregiserParticleEffectNode(node);
    }

    // If the Particle Emitter node is removed - remove it from the tree.
    ParticleEmitter * emitter = GetEmitter(node);
    if (emitter)
    {
        ParticlesEditorController::Instance()->RemoveParticleEmitterNode(node);
    }
}

bool ParticlesEditorSceneDataHelper::ValidateParticleEmitter(ParticleEmitter * emitter, String& validationMsg)
{
	if (!emitter)
	{
		return true;
	}
	
	if (emitter->Is3DFlagCorrect())
	{
		return true;
	}
	
	validationMsg = Format("\"3d\" flag value is wrong for Particle Emitter Configuration file %s. Please verify whether you are using the correct configuration file.", emitter->GetConfigPath().GetAbsolutePathname().c_str());
	return false;
}
