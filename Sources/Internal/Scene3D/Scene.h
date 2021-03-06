/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_SCENE_H__
#define __DAVAENGINE_SCENE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Scene3D/Entity.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"
#include "Scene3D/SceneFileV2.h"

namespace DAVA
{

/**
    \defgroup scene3d 3D Engine
  */  
    
class Texture;
class Material;
class StaticMesh;
class AnimatedMesh;
class SceneNodeAnimationList;
class DataNode;
class SceneFileV2;
class ShadowVolumeNode;
class ProxyNode;
class Light;
class ShadowRect;
class QuadTree;
class MeshInstanceNode;
class ImposterManager;
class ImposterNode;
class Component;
class SceneSystem;
class RenderSystem;
class RenderUpdateSystem;
class TransformSystem;
class LodSystem;
class DebugRenderSystem;
class EventSystem;
class ParticleEmitterSystem;
class ParticleEffectSystem;
class UpdateSystem;
class LightUpdateSystem;
class SwitchSystem;
class SoundUpdateSystem;
class ActionUpdateSystem;
class SkyboxSystem;
    
/**
    \ingroup scene3d
    \brief This class is a code of our 3D Engine scene graph. 
    To visualize any 3d scene you'll need to create Scene object. 
    Scene have visible hierarchy and invisible root nodes. You can add as many root nodes as you want, and do not visualize them.
    For example you can have multiple scenes, load them to one scene, and show each scene when it will be required. 
 
 
 */
class Scene : public Entity
{
protected:
	virtual ~Scene();
public:	
	Scene();
	
    /**
        \brief Function to register node in scene. This function is called when you add node to the node that already in the scene. 
     */
    virtual void    RegisterNode(Entity * entity);
    virtual void    UnregisterNode(Entity * entity);
    
    virtual void    AddComponent(Entity * entity, Component * component);
    virtual void    RemoveComponent(Entity * entity, Component * component);
    
    virtual void    AddSystem(SceneSystem * sceneSystem, uint32 componentFlags);
    virtual void    RemoveSystem(SceneSystem * sceneSystem);
    
	virtual void ImmediateEvent(Entity * entity, uint32 componentType, uint32 event);

    Vector<SceneSystem*> systems;
    TransformSystem * transformSystem;
    RenderUpdateSystem * renderUpdateSystem;
	LodSystem * lodSystem;
    DebugRenderSystem * debugRenderSystem;
	EventSystem * eventSystem;
	ParticleEffectSystem * particleEffectSystem;
	UpdateSystem * updatableSystem;
    LightUpdateSystem * lightUpdateSystem;
	SwitchSystem * switchSystem;
	RenderSystem * renderSystem;
	SoundUpdateSystem * soundSystem;
	ActionUpdateSystem* actionSystem;
	SkyboxSystem* skyboxSystem;
	
    /**
        \brief Overloaded GetScene returns this, instead of normal functionality.
     */
    virtual Scene * GetScene();
    
	void AddAnimatedMesh(AnimatedMesh * mesh);
	void RemoveAnimatedMesh(AnimatedMesh * mesh);
	AnimatedMesh * GetAnimatedMesh(int32 index);
	inline int32	GetAnimatedMeshCount();
	
	void AddAnimation(SceneNodeAnimationList * animation);
	SceneNodeAnimationList * GetAnimation(int32 index);
	SceneNodeAnimationList * GetAnimation(const String & name);
	inline int32 GetAnimationCount();
    
    
    /**
        \brief Function to add root node.
        \param[in] node node you want to add
        \param[in] rootNodePath path of this root node
     */

    void AddRootNode(Entity *node, const FilePath &rootNodePath);

	/**
        \brief Get root node by path.
        This function can be used when you want to get a node and add it to real scene.  
        \code
        Entity * node = scene->GetRootNode("~res:/Scenes/level0.sce");
        scene->AddNode(node);
        \endcode
     */
    
    Entity *GetRootNode(const FilePath &rootNodePath);
    
    /**
        \brief Release root node by name.
        \param[in] rootNodePath root node path you want to release.
     */
    void ReleaseRootNode(const FilePath &rootNodePath);
    
    /**
        \brief Release root node by pointer to this node.
        \param[in] nodeToRelease root node pointer you want to release.
     */
    void ReleaseRootNode(Entity *nodeToRelease);

	
	virtual void StopAllAnimations(bool recursive = true);
	
	virtual void	Update(float timeElapsed);
	virtual void	Draw();
	
	
	virtual void	SetupTestLighting();
	
	Camera * GetCamera(int32 n);
	void AddCamera(Camera * c);
	inline int32	GetCameraCount();
    
    void SetCurrentCamera(Camera * camera);
    Camera * GetCurrentCamera() const;
    
    /* 
        This camera is used for clipping only. If you do not call this function GetClipCamera returns currentCamera. 
        You can use SetClipCamera function if you want to test frustum clipping, and view the scene from different angles.
     */
    void SetClipCamera(Camera * clipCamera);
    Camera * GetClipCamera() const;

	void AddDrawTimeShadowVolume(ShadowVolumeNode * shadowVolume);
    
    Set<Light*> & GetLights();
	Light * GetNearestDynamicLight(Light::eType type, Vector3 position);

	void RegisterImposter(ImposterNode * imposter);
	void UnregisterImposter(ImposterNode * imposter);

	void CreateComponents();
	void CreateSystems();

	EventSystem * GetEventSystem();
	RenderSystem * GetRenderSystem() const;
    
    virtual SceneFileV2::eError Save(const DAVA::FilePath & pathname, bool saveForGame = false);

    
protected:	
    
    void UpdateLights();
    
    uint64 drawTime;
    uint32 nodeCounter;

	Vector<AnimatedMesh*> animatedMeshes;
	Vector<Camera*> cameras;
	Vector<SceneNodeAnimationList*> animations;
    
    Map<String, ProxyNode*> rootNodes;

    Camera * currentCamera;
    Camera * clipCamera;

	Vector<ShadowVolumeNode*> shadowVolumes;
    Set<Light*> lights;

	ImposterManager * imposterManager;
    
    friend class Entity;
};

	
int32 Scene::GetAnimationCount()
{
    return (int32)animations.size();
}

int32 Scene::GetAnimatedMeshCount()
{
    return (int32)animatedMeshes.size();
}

int32 Scene::GetCameraCount()
{
    return (int32)cameras.size();
}  

};




#endif // __DAVAENGINE_SCENE_H__


