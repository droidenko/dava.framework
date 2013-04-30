//
//  BaseParticleEditorNode.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/26/12.
//
//

#ifndef __ResourceEditorQt__BaseParticleEditorNode__
#define __ResourceEditorQt__BaseParticleEditorNode__

#include "Scene3D/ParticleEffectNode.h"
#include "Qt/DockSceneGraph/ExtraUserData.h"

namespace DAVA {

class ParticleEffectComponent;
	
// Base Particle Editor node - common for all inner nodes.
class BaseParticleEditorNode : public ExtraUserData
{
public:
    BaseParticleEditorNode(Entity* rootNode);
    virtual ~BaseParticleEditorNode();
    
    // Add/remove child node.
    virtual void AddChildNode(BaseParticleEditorNode* childNode);
    virtual void AddChildNodeAbove(BaseParticleEditorNode* childNode, BaseParticleEditorNode* childNodeToMoveAbove);
	
    void RemoveChildNode(BaseParticleEditorNode* childNode, bool needDeleteNode = true);

	// Change the order of the children.
	void MoveChildNode(BaseParticleEditorNode* childNode, BaseParticleEditorNode* childNodeToMoveAbove);
	
    // Access to the root node.
    Entity* GetRootNode() const {return rootNode;};

	// Access to the parent node.
	BaseParticleEditorNode* GetParentNode() const {return parentNode;};

	// Access to the Particle Effect Component.
	ParticleEffectComponent* GetParticleEffectComponent() const;

    // Node name.
    void SetNodeName(const QString& nodeName) {this->nodeName = nodeName;};
    virtual QString GetName() const {return this->nodeName;};

    // Mark for selection logic.
    bool IsMarkedForSelection() const {return this->isMarkedForSelection;};
    void SetMarkedToSelection(bool value) {this->isMarkedForSelection = value;};

    // Access to children.
    typedef List<BaseParticleEditorNode*> PARTICLEEDITORNODESLIST;
    const PARTICLEEDITORNODESLIST& GetChildren() const {return childNodes;};

	KeyedArchive* GetExtraData();
	void ClearExtraData();

protected:
	// Set the parent of the node.
	void SetParentNode(BaseParticleEditorNode* parentNode) {this->parentNode = parentNode; };

    // Cleanup the node's children.
    void Cleanup();

    // Root effect node.
    Entity* rootNode;

	// Our parent.
	BaseParticleEditorNode* parentNode;

    // Current node name.
    QString nodeName;

    // Child nodes.
    PARTICLEEDITORNODESLIST childNodes;
    
    // Whether the node is marked for selection.
    bool isMarkedForSelection;

	KeyedArchive* extraData;
};

}

#endif /* defined(__ResourceEditorQt__BaseParticleEditorNode__) */
