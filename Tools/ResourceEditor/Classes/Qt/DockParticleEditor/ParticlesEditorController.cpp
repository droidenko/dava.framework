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
#include "ParticlesEditorController.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "ParticlesEditorQT/Helpers/ParticlesEditorNodeNameHelper.h"

#include "../StringConstants.h"

using namespace DAVA;

#define LIFETIME_FOR_NEW_PARTICLE_EMITTER 4.0f

ParticlesEditorController::ParticlesEditorController(QObject* parent) :
    QObject(parent)
{
    this->selectedNode = NULL;
}

ParticlesEditorController::~ParticlesEditorController()
{
    Cleanup();
}

EffectParticleEditorNode* ParticlesEditorController::RegisterParticleEffectNode(Entity* effectNode, bool autoStart)
{
    if (!effectNode)
    {
        Logger::Warning("ParticlesEditorController::RegisterParticleEffectNode(): node is NULL!");
        return NULL;
    }



    EffectParticleEditorNode* rootNode = new EffectParticleEditorNode(effectNode);
    this->particleEffectNodes[effectNode] = rootNode;
    if (autoStart)
    {
    	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(effectNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
		DVASSERT(effectComponent);
        effectComponent->Start();
    }

    return rootNode;
}

void ParticlesEditorController::UnregiserParticleEffectNode(Entity* effectNode)
{
    if (!effectNode)
    {
        Logger::Warning("ParticlesEditorController::UnregiserParticleEffectNode(): node is NULL!");
        return;
    }

    PARTICLESEFFECTITER iter = this->particleEffectNodes.find(effectNode);
    if (iter == this->particleEffectNodes.end())
    {
        Logger::Warning("ParticlesEditorController::UnregiserParticleEffectNode(): node is not found!");
        return;
    }

    this->particleEffectNodes.erase(iter);
    SAFE_DELETE(iter->second);
}

void ParticlesEditorController::Cleanup()
{
    // TODO! IMPLEMENT!
}

bool ParticlesEditorController::IsBelongToParticlesEditor(SceneGraphItem* sceneGraphItem)
{
    ExtraUserData* extraUserData = sceneGraphItem->GetExtraUserData();
    if (!extraUserData)
    {
        return false;
    }

    // Whether the node belongs to Particle Editor?
    BaseParticleEditorNode* baseParticleEditorNode = dynamic_cast<BaseParticleEditorNode*>(extraUserData);
    return (baseParticleEditorNode != NULL);
}

bool ParticlesEditorController::ShouldDisplayPropertiesInSceneEditor(SceneGraphItem *sceneGraphItem)
{
    ExtraUserData* extraUserData = sceneGraphItem->GetExtraUserData();
    if (!extraUserData)
    {
        // Non-Particle Editor at all.
        return true;
    }

    if (dynamic_cast<EffectParticleEditorNode*>(extraUserData))
    {
        // This is Particle Effect node, it has properties.
        return true;
    }
    if (dynamic_cast<EmitterParticleEditorNode*>(extraUserData))
    {
        // This is Particle Emitter node, it has properties.
        return true;
    }

    return false;
}

EffectParticleEditorNode* ParticlesEditorController::GetRootForParticleEffectNode(Entity* effectNode)
{
    PARTICLESEFFECTITER iter = this->particleEffectNodes.find(effectNode);
    if (iter == this->particleEffectNodes.end())
    {
        return NULL;
    }
    
    return iter->second;
}

void ParticlesEditorController::SetSelectedNode(SceneGraphItem* selectedItem, bool isEmitEvent)
{
    if (IsBelongToParticlesEditor(selectedItem) == false)
    {
        Logger::Warning("ParticlesEditorController::SetSelectedNode(): attempt to select wrong node!");
        return;
    }
    
	if (isEmitEvent)
	{
		EmitNodeWillBeDeselected();
	}
    this->selectedNode = dynamic_cast<BaseParticleEditorNode*>(selectedItem->GetExtraUserData());
    if (isEmitEvent)
    {
        EmitSelectedNodeChanged();
    }
}

void ParticlesEditorController::CleanupSelectedNode()
{
    this->selectedNode = NULL;
    EmitSelectedNodeChanged();
}

void ParticlesEditorController::EmitNodeWillBeDeselected()
{
	if (this->selectedNode == NULL)
		return;

	emit NodeDeselected(this->selectedNode);
}

void ParticlesEditorController::EmitSelectedNodeChanged(bool forceRefresh)
{
    if (this->selectedNode == NULL)
    {
        emit EmitterSelected(NULL, this->selectedNode);
        return;
    }

    // Determine the exact node type and emit the event needed.
	EffectParticleEditorNode* effectEditorNode = dynamic_cast<EffectParticleEditorNode*>(this->selectedNode);
    if (effectEditorNode)
    {
		emit EmitterSelected(NULL, this->selectedNode);
		emit EffectSelected(effectEditorNode->GetRootNode());
        return;
    }
	
    EmitterParticleEditorNode* emitterEditorNode = dynamic_cast<EmitterParticleEditorNode*>(this->selectedNode);
    if (emitterEditorNode)
    {
        emit EmitterSelected(emitterEditorNode->GetEmitterNode(), this->selectedNode);
        return;
    }
    
    LayerParticleEditorNode* layerEditorNode = dynamic_cast<LayerParticleEditorNode*>(this->selectedNode);
    if (layerEditorNode)
    {
        emit LayerSelected(layerEditorNode->GetEmitterNode(), layerEditorNode->GetLayer(), this->selectedNode, forceRefresh);
        return;
    }
    
    ForceParticleEditorNode* forceEditorNode = dynamic_cast<ForceParticleEditorNode*>(this->selectedNode);
    if (forceEditorNode)
    {
        emit ForceSelected(forceEditorNode->GetEmitterNode(), forceEditorNode->GetLayer(),
                           forceEditorNode->GetForceIndex(), this->selectedNode);
        return;
    }

    // Cleanip the selection in case we don't know what to do.
    Logger::Warning("ParticlesEditorController::EmitSelectedNodeChanged() - unknown selected node type!");
    EmitterSelected(NULL, this->selectedNode);
}

void ParticlesEditorController::AddParticleEmitterNodeToScene(Entity* emitterSceneNode)
{
    // We are adding new Emitter to the Particle Effect node just selected.
    Entity* effectNode = NULL;
    BaseParticleEditorNode* selectedNode = GetSelectedNode();
    if (selectedNode)
    {
        effectNode = selectedNode->GetRootNode();
    }
    
    EffectParticleEditorNode* effectEditorNode = GetRootForParticleEffectNode(effectNode);
    if (effectNode && effectEditorNode)
    {
        EmitterParticleEditorNode* emitterEditorNode = new EmitterParticleEditorNode(effectNode, emitterSceneNode,
                                                                                     QString::fromStdString(emitterSceneNode->GetName()));
		
		ParticleEmitter * emitter = GetEmitter(emitterSceneNode);
		if (!emitter)
		{
		    return;
		}
		emitter->SetLifeTime(LIFETIME_FOR_NEW_PARTICLE_EMITTER);
		
		String newName = ParticlesEditorNodeNameHelper::GetNewNodeName(emitterSceneNode->GetName(), effectNode);
		emitterSceneNode->SetName(newName);

        effectNode->AddNode(emitterSceneNode);
        effectEditorNode->AddChildNode(emitterEditorNode);
    }
}

void ParticlesEditorController::RemoveParticleEmitterNode(Entity* emitterSceneNode)
{
    // Lookup for such node.
    EffectParticleEditorNode* effectEditorNode = NULL;
    EmitterParticleEditorNode* emitterEditorNode = NULL;

    FindEmitterEditorNode(emitterSceneNode, &effectEditorNode, &emitterEditorNode);

    if (effectEditorNode && emitterEditorNode)
    {
		CleanupSelectedNodeIfDeleting(emitterEditorNode);
        effectEditorNode->RemoveChildNode(emitterEditorNode);
    }
}

void ParticlesEditorController::CleanupParticleEmitterEditorNode(EmitterParticleEditorNode* emitterNode)
{
    // Leave the node itself, but cleanup all the children.
    while (!emitterNode->GetChildren().empty())
    {
        emitterNode->RemoveChildNode(emitterNode->GetChildren().front());
    }
}

LayerParticleEditorNode* ParticlesEditorController::AddParticleLayerToNode(EmitterParticleEditorNode* emitterNode)
{
    if (!emitterNode)
    {
        return NULL;
    }
    
    ParticleEmitter* emitter = emitterNode->GetParticleEmitter();
    if (!emitter)
    {
        return NULL;
    }
    
    // Create the new layer.
    ParticleLayer *layer;
    if(emitter->GetIs3D())
    {
        layer = new ParticleLayer3D(emitter);
    }
    else
    {
        layer = new ParticleLayer();
    }

	layer->startTime = 0;
    layer->endTime = LIFETIME_FOR_NEW_PARTICLE_EMITTER;
	layer->life = new PropertyLineValue<float32>(emitter->GetLifeTime());

    layer->layerName = ParticlesEditorNodeNameHelper::GetNewLayerName(ResourceEditor::LAYER_NODE_NAME, emitter);

    emitter->AddLayer(layer);

    // Create the new node and add it to the tree.
    LayerParticleEditorNode* layerNode = new LayerParticleEditorNode(emitterNode, layer);
    emitterNode->AddChildNode(layerNode);

    SafeRelease(layer);

    return layerNode;
}

LayerParticleEditorNode* ParticlesEditorController::CloneParticleLayerNode(LayerParticleEditorNode* layerToClone)
{
    if (!layerToClone || !layerToClone->GetLayer())
    {
        return NULL;
    }
    
    EmitterParticleEditorNode* emitterNode = layerToClone->GetEmitterEditorNode();
    if (!emitterNode)
    {
        return NULL;
    }

    ParticleEmitter* emitter = emitterNode->GetParticleEmitter();
    if (!emitter)
    {
        return NULL;
    }

    ParticleLayer* clonedLayer = layerToClone->GetLayer()->Clone();
    emitter->AddLayer(clonedLayer);
    
    LayerParticleEditorNode* clonedEditorNode = new LayerParticleEditorNode(emitterNode, clonedLayer);
    emitterNode->AddChildNode(clonedEditorNode);

    return clonedEditorNode;
}

void ParticlesEditorController::RemoveParticleLayerNode(LayerParticleEditorNode* layerToRemove)
{
    if (!layerToRemove)
    {
        return;
    }
    
    EmitterParticleEditorNode* emitterNode = layerToRemove->GetEmitterEditorNode();
    if (!emitterNode)
    {
        return;
    }

    ParticleEmitter* emitter = emitterNode->GetParticleEmitter();
    if (!emitter)
    {
        return;
    }
    
    // Lookup for the layer to be removed.
    int32 layerIndex = layerToRemove->GetLayerIndex();
    if (layerIndex == -1)
    {
        return;
    }

	// Reset the selected node in case it is one to be removed.
	CleanupSelectedNodeIfDeleting(layerToRemove);

    // Remove the node from the layers list and also from the emitter.
    emitter->RemoveLayer(layerIndex);
    
    emitterNode->RemoveChildNode(layerToRemove);
}

ForceParticleEditorNode* ParticlesEditorController::AddParticleForceToNode(LayerParticleEditorNode* layerNode)
{
    if (!layerNode)
    {
        return NULL;
    }
    
    ParticleLayer* layer = layerNode->GetLayer();
    if (!layer)
    {
        return NULL;
    }
    
    // Add the new Force to the Layer.
	ParticleForce* newForce = new ParticleForce(RefPtr<PropertyLine<Vector3> >(new PropertyLineValue<Vector3>(Vector3(0, 0, 0))),
												RefPtr<PropertyLine<Vector3> >(NULL), RefPtr<PropertyLine<float32> >(NULL));
	layer->AddForce(newForce);

    // Create the node for the new layer.
    int newLayerIndex = layer->forces.size() - 1;
    ForceParticleEditorNode* forceNode = new ForceParticleEditorNode(layerNode, newLayerIndex);
    layerNode->AddChildNode(forceNode);

    // Update the names for the forces.
    layerNode->UpdateForcesIndices();
    
    return forceNode;
}

void ParticlesEditorController::RemoveParticleForceNode(ForceParticleEditorNode* forceNode)
{
    if (!forceNode || !forceNode->GetLayerEditorNode())
    {
        return;
    }
    
    LayerParticleEditorNode* layerNode = forceNode->GetLayerEditorNode();
    ParticleLayer* layer = layerNode->GetLayer();
    if (!layer)
    {
        return;
    }

	// If the selected node is one to be removed - clean it up.
	CleanupSelectedNodeIfDeleting(forceNode);

    // Remove the force from the emitter...
    int forceIndex = forceNode->GetForceIndex();
	layer->RemoveForce(forceIndex);
    
    // ...and from the tree.
    layerNode->RemoveChildNode(forceNode);
    
    // Done removing, recalculate the indices and names.
    layerNode->UpdateForcesIndices();
}

void ParticlesEditorController::FindEmitterEditorNode(Entity* emitterSceneNode,
                                                      EffectParticleEditorNode** effectEditorNode,
                                                      EmitterParticleEditorNode** emitterEditorNode)
{
    for (PARTICLESEFFECTITER iter = particleEffectNodes.begin(); iter != particleEffectNodes.end();
         iter ++)
    {
        const BaseParticleEditorNode::PARTICLEEDITORNODESLIST& emitterEditorNodes = iter->second->GetChildren();
        for (List<BaseParticleEditorNode*>::const_iterator innerIter = emitterEditorNodes.begin();
             innerIter != emitterEditorNodes.end(); innerIter ++)
        {
            EmitterParticleEditorNode* innerNode = dynamic_cast<EmitterParticleEditorNode*>(*innerIter);
            if (innerNode && innerNode->GetEmitterNode() == emitterSceneNode)
            {
                *effectEditorNode = iter->second;
                *emitterEditorNode = innerNode;
                break;
            }
        }
        
        // If the emitter editor found during inner loop - break the outer one too.
        if (*effectEditorNode && *emitterEditorNode)
        {
            break;
        }
    }
}

bool ParticlesEditorController::MoveEmitter(EmitterParticleEditorNode* movedItemEmitterNode, EffectParticleEditorNode* newEffectParentNode)
{
	if (!movedItemEmitterNode || !newEffectParentNode)
	{
		return false;
	}
	
	if (movedItemEmitterNode->GetParentNode() == newEffectParentNode)
	{
		// No sence in moving Emitter to the same parent it currently belongs.
		return false;
	}

	// Move the Emitter to the new Effect inside the Particles Editor hierarchy...
	BaseParticleEditorNode* oldEffectParentNode = movedItemEmitterNode->GetParentNode();
	oldEffectParentNode->RemoveChildNode(movedItemEmitterNode, false);
	newEffectParentNode->AddChildNode(movedItemEmitterNode);

	// and inside the SceneGraph.
	Entity* movedNode = movedItemEmitterNode->GetEmitterNode();
	Entity* newParentNode = newEffectParentNode->GetRootNode();
	Entity* oldParentNode = oldEffectParentNode->GetRootNode();

    SafeRetain(movedNode);
    oldParentNode->RemoveNode(movedNode);
    newParentNode->AddNode(movedNode);
    SafeRelease(movedNode);

	return true;
}

bool ParticlesEditorController::MoveLayer(LayerParticleEditorNode* moveItemNode, LayerParticleEditorNode* moveAboveNode)
{
	if (!moveItemNode || !moveAboveNode)
	{
		return false;
	}
	
	if (moveAboveNode->GetParentNode() == moveItemNode->GetParentNode())
	{
		// We are just changing the layer's order inside the same node.
		return ChangeLayersOrderInSameEmitter(moveItemNode, moveAboveNode);
	}
	else
	{
		// We are also changing the "parent" emitters.
		return ChangeLayersOrderInDifferentEmitters(moveItemNode, moveAboveNode);
	}
}

bool ParticlesEditorController::MoveLayer(LayerParticleEditorNode* moveItemNode, EmitterParticleEditorNode* newEmitterNode)
{
	if (!moveItemNode || !newEmitterNode)
	{
		return false;
	}
	
	if (moveItemNode->GetParentNode() == newEmitterNode)
	{
		// No need to move the layer inside the same emitter.
		return false;
	}
	
	return MoveLayerToEmitter(moveItemNode, newEmitterNode);
}

bool ParticlesEditorController::ChangeLayersOrderInSameEmitter(LayerParticleEditorNode* movedItemNode, LayerParticleEditorNode* moveAboveNode)
{
	// Change both the order of the representation tree nodes and the layers themselves.
	BaseParticleEditorNode* parentNode = movedItemNode->GetParentNode();
	if (!parentNode)
	{
		return false;
	}

	parentNode->MoveChildNode(movedItemNode, moveAboveNode);
	
	ParticleLayer* layerToMove = movedItemNode->GetLayer();
	ParticleLayer* layerToMoveAbove = moveAboveNode->GetLayer();

	ParticleEmitter* parentEmitter = movedItemNode->GetParticleEmitter();
	parentEmitter->MoveLayer(layerToMove, layerToMoveAbove);
	
	return true;
}

bool ParticlesEditorController::ChangeLayersOrderInDifferentEmitters(LayerParticleEditorNode* moveItemNode, LayerParticleEditorNode* moveAboveNode)
{
	// Sanity check.
	EmitterParticleEditorNode* oldParentNode = dynamic_cast<EmitterParticleEditorNode*>(moveItemNode->GetParentNode());
	EmitterParticleEditorNode* newParentNode = dynamic_cast<EmitterParticleEditorNode*>(moveAboveNode->GetParentNode());
	if (!oldParentNode || !newParentNode)
	{
		return false;
	}

	return PerformMoveBetweenEmitters(oldParentNode, newParentNode, moveItemNode, moveAboveNode);
}

bool ParticlesEditorController::MoveLayerToEmitter(LayerParticleEditorNode* moveItemNode, EmitterParticleEditorNode* newEmitterNode)
{
	// Sanity check.
	// Sanity check.
	EmitterParticleEditorNode* oldParentNode = dynamic_cast<EmitterParticleEditorNode*>(moveItemNode->GetParentNode());
	if (!oldParentNode)
	{
		return false;
	}

	return PerformMoveBetweenEmitters(oldParentNode, newEmitterNode, moveItemNode, NULL);
}

bool ParticlesEditorController::PerformMoveBetweenEmitters(EmitterParticleEditorNode* oldEmitterNode,
														   EmitterParticleEditorNode* newEmitterNode,
														   LayerParticleEditorNode* layerNodeToMove,
														   LayerParticleEditorNode* layerNodeToInsertAbove)
{
	ParticleEmitter* oldParentEmitter = oldEmitterNode->GetParticleEmitter();
	ParticleEmitter* newParentEmitter = newEmitterNode->GetParticleEmitter();
	if (!oldParentEmitter || !newParentEmitter)
	{
		return false;
	}

	// Move the Editor node. layerNodeToInsertAbove is allowed to be NULL.
	newEmitterNode->AddChildNodeAbove(layerNodeToMove, layerNodeToInsertAbove);
	oldEmitterNode->RemoveChildNode(layerNodeToMove, false);
	
	// Move the Particle Layers themselves.
	ParticleLayer* layerToMove = layerNodeToMove->GetLayer();
	ParticleLayer* layerToInsertAbove = NULL;
	if (layerNodeToInsertAbove)
	{
		layerToInsertAbove = layerNodeToInsertAbove->GetLayer();
	}

	SafeRetain(layerToMove);
	oldParentEmitter->RemoveLayer(layerToMove);
	
	newParentEmitter->AddLayer(layerToMove, layerToInsertAbove);
	SafeRelease(layerToMove);
	
	// Update the emitter.
	layerNodeToMove->UpdateEmitterEditorNode(newEmitterNode);

	return true;
}

void ParticlesEditorController::RefreshSelectedNode(bool forceRefresh)
{
	if (this->selectedNode)
	{
		EmitSelectedNodeChanged(forceRefresh);
	}
}

void ParticlesEditorController::CleanupSelectedNodeIfDeleting(BaseParticleEditorNode* nodeToBeDeleted)
{
	if (this->selectedNode == nodeToBeDeleted)
	{
		this->selectedNode = NULL;
	}
}
