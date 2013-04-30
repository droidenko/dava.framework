//
//  ParticlesEditorSceneModelHelper.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/26/12.
//
//

#ifndef __ResourceEditorQt__ParticlesEditorSceneModelHelper__
#define __ResourceEditorQt__ParticlesEditorSceneModelHelper__

#include "DAVAEngine.h"
#include <QItemSelection>
#include <QModelIndex>
#include <QMenu>

#include "ParticlesEditorQT/Nodes/BaseParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/EffectParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/EmitterParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/LayerParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/ForceParticleEditorNode.h"

#include "Qt/Scene/SceneData.h"
#include "Qt/DockSceneGraph/SceneGraphItem.h"

#include "Commands/Command.h"

namespace DAVA {

// Scene Model Helper for Particles Editor.
class ParticlesEditorSceneModelHelper
{
public:
    // Custom processing the Selection Changed in the Scene Graph model. Returns
    // TRUE if no further processing needed.
    bool ProcessSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    // Custom node selection.
    SceneGraphItem* GetGraphItemToBeSelected(GraphItem* rootItem, Entity* node);
	
	// Reset the selected item.
	void ResetSelection();

    // Preprocess the Scene Node during adding, change its type if needed.
    Entity* PreprocessSceneNode(Entity* rawNode);

    // Add the node and all its children to the Scene Graph.
    bool AddNodeToSceneGraph(SceneGraphItem *graphItem, Entity *node);

    // Whether we need to display "standard" Scene Editor Popup Menu for this node?
    bool NeedDisplaySceneEditorPopupMenuItems(const QModelIndex &index) const;

    // Add Popup Menu items depending on the tree node selected.
    void AddPopupMenuItems(QMenu &menu, const QModelIndex &index) const;
    
	// Move the item to parent functionality.
	// Whether the move should be handled by the Particle Editor?
	bool NeedMoveItemToParent(GraphItem* movedItem, GraphItem* newParentItem);

	// Perform the move itself.
	bool MoveItemToParent(GraphItem* movedItem, GraphItem* newParentItem);

	// Get the Scene Graph item which contains the appropriate Particles Editor objects.
	SceneGraphItem* GetGraphItemForParticlesLayer(GraphItem* rootItem, DAVA::ParticleLayer* layer);
	
	// Get the persistent pointer to the Particle Editor element.
	void* GetPersistentDataForModelIndex(const QModelIndex& modelIndex) const;

	// Some Scene Graph tree nodes might be checkable - check logic.
	// Is the graph item passed checkable?
	bool IsGraphItemCheckable(GraphItem* graphItem) const;
	
	// Get/set the checked state for the Graph Item, if it is checkable.
	bool GetCheckableStateForGraphItem(GraphItem* graphItem) const;
	void SetCheckableStateForGraphItem(GraphItem* graphItem, bool value);
	
protected:
	// Add the action to QT menu.
	void AddActionToMenu(QMenu *menu, const QString &actionTitle, Command *command) const;
	
    // Build the Scene Graph in a recursive way.
    void BuildSceneGraphRecursive(BaseParticleEditorNode* rootNode, SceneGraphItem* rootItem);

    // Determine whether we need to select Editor Tree Node in a recursive way.
    SceneGraphItem* GetGraphItemToBeSelectedRecursive(GraphItem* rootItem, Entity* node);

    // Do the checks needed and return ExtraUserData from model index. or from the item.
    ExtraUserData* GetExtraUserData(const QModelIndex& modelIndex) const;
	ExtraUserData* GetExtraUserData(GraphItem* item) const;
	ExtraUserData* GetExtraUserDataByModelIndex(const QModelIndex& modelIndex) const;
    
    // Synchronization of the whole Particle Editor Tree and different types of Nodes.
    void SynchronizeParticleEditorTree(BaseParticleEditorNode* node);

    void SynchronizeEffectParticleEditorNode(EffectParticleEditorNode* node, Entity* effectRootNode);
    void SynchronizeEmitterParticleEditorNode(EmitterParticleEditorNode* node);
    void SynchronizeLayerParticleEditorNode(LayerParticleEditorNode* node);

	void BuildEntitiesSets(EffectParticleEditorNode* node, Entity* effectRootNode,
						   Set<Entity*>& entitiesInParticleEditor,
						   Set<Entity*>& entitiesInSceneGraph);
	void AddNewNodesToSceneGraph(EffectParticleEditorNode* node, Entity* effectRootNode);
	void RemoveExcessiveNodesFromSceneGraph(EffectParticleEditorNode* node, Entity* effectRootNode);

	// Move item to parent functionality.
	bool IsItemBelongToParticleEditor(GraphItem* item);

	// Whether this move is forbidden?
	bool IsMoveItemToParentForbidden(GraphItem* movedItem, GraphItem* newParentItem);

	Entity* CreateParticleEffectNode();
	
	// Get the particular type of node by the graph item. Return NULL if the graph item
	// belongs to another kind of node.
	LayerParticleEditorNode* GetLayerEditorNodeByGraphItem(GraphItem* graphItem) const;
};
	
}

#endif /* defined(__ResourceEditorQt__ParticlesEditorSceneModelHelper__) */
