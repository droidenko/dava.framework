#ifndef __SCENE_DATA_H__
#define __SCENE_DATA_H__

#include "DAVAEngine.h"
#include "SceneEditor/CameraController.h"
#include "ParticlesEditorQT/Helpers/ParticlesEditorSceneDataHelper.h"
#include <QObject>

class EditorScene;
class LandscapesController;
class EditorLandscape;

class SceneData: public QObject
{
    friend class SceneDataManager;
    
    Q_OBJECT
    
    
public:
    SceneData();
    virtual ~SceneData();

	// Rebuild the scene graph for particular node.
	void RebuildSceneGraphNode(DAVA::Entity* node);
	
	// Rebuild the whole scene graph.
    void RebuildSceneGraph();

    void SetScene(EditorScene *newScene);
    EditorScene * GetScene();
    
    void AddSceneNode(DAVA::Entity *node);
    void RemoveSceneNode(DAVA::Entity *node);

    void SelectNode(DAVA::Entity *node);
    DAVA::Entity * GetSelectedNode();
    
    void LockAtSelectedNode();
    
    DAVA::CameraController *GetCameraController();
    
    void CreateScene(bool createEditorCameras);

    void SetScenePathname(const DAVA::FilePath &newPathname);
    const DAVA::FilePath & GetScenePathname() const;

    void ToggleNotPassableLandscape();
    
    bool CanSaveScene();
    
    LandscapesController *GetLandscapesController();
	void SetLandscapesControllerScene(EditorScene* scene);
    
	void ResetLandsacpeSelection();

	void RestoreTexture(const DAVA::FilePath &descriptorPathname, DAVA::Texture *texture);

	// Emit the SceneChanged singal.
	void EmitSceneChanged();

	void GetAllParticleEffects(DAVA::List<DAVA::Entity*> & particleEffects);

signals:
	void SceneChanged(EditorScene *scene);
	void SceneNodeSelected(DAVA::Entity *node);
	
	// Signals are specific for Scene Graph Model.
	void SceneGraphModelNeedsRebuildNode(DAVA::Entity *node);
	void SceneGraphModelNeedsRebuild();
	
	void SceneGraphModelNeedSetScene(EditorScene* scene);
	void SceneGraphModelNeedsSelectNode(DAVA::Entity* node);

protected:
    
    void BakeNode(DAVA::Entity *node);
    void FindIdentityNodes(DAVA::Entity *node);
    void RemoveIdentityNodes(DAVA::Entity *node);
    
    void ReloadNode(DAVA::Entity *node, const DAVA::FilePath &nodePathname);

    void ReleaseScene();

	void FindAllParticleEffectsRecursive(DAVA::Entity *node , DAVA::List<DAVA::Entity*> & particleEffects);

protected slots:
    void SceneNodeSelectedInGraph(DAVA::Entity *node);

protected:
    EditorScene *scene;

	// Node currently selected.
	DAVA::Entity* selectedNode;
	
	// Controllers related to SceneData.
    DAVA::WASDCameraController *cameraController;
    LandscapesController *landscapesController;
	
    DAVA::FilePath sceneFilePathname;
    
	// Particles Editor Scene Data Helper.
	DAVA::ParticlesEditorSceneDataHelper particlesEditorSceneDataHelper;
};

#endif // __SCENE_DATA_H__
