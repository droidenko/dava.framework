#ifndef __SCENE_INFO_H__
#define __SCENE_INFO_H__

#include "Qt/QtPosSaver/QtPosSaver.h"
#include "Qt/QtPropertyEditor/QtPropertyEditor.h"
#include "Qt/DockProperties/PropertyEditorStateHelper.h"

#include <QShowEvent>

class SceneData;
class SceneInfo : public QtPropertyEditor
{
	Q_OBJECT

protected:
    
    struct LODInfo
    {
        DAVA::uint32 trianglesOnLod[DAVA::LodComponent::MAX_LOD_LAYERS];
        DAVA::uint32 trianglesOnLandscape;
        
        void Clear()
        {
            for(DAVA::int32 i = 0; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
            {
                trianglesOnLod[i] = 0;
            }
            trianglesOnLandscape = 0;
        }
    };
    
public:
	SceneInfo(QWidget *parent = 0);
	~SceneInfo();


public slots:
	void sceneActivated(SceneData *scene);
	void sceneChanged(SceneData *scene);
	void sceneReleased(SceneData *scene);

//TODO: add set of slots to different scene changes
//    void nodeDeleted(SceneData *scene);
//    void nodeAdded(SceneData *scene);
//    void TexturesReloaded();
//    void MaterialsChanged();
    
protected slots:
    
    void timerDone();
    
protected:
    
    virtual void showEvent ( QShowEvent * event );
    
protected:

    void InitializeInfo();
    void InitializeGeneralSection();
    void Initialize3DDrawSection();
    void InitializeMaterialsSection();
    void InitializeLODSection();
    void InitializeParticlesSection();

    void RefreshSceneGeneralInfo();
    void Refresh3DDrawInfo();
    void RefreshMaterialsInfo();
    void RefreshLODInfo();
    void RefreshParticlesInfo();

    
	void RefreshAllData(SceneData *sceneData);

    void CollectSceneData(SceneData *sceneData, bool force);
    void ClearData();
    
    void SaveTreeState();
    void RestoreTreeState();
    
    
    QtPropertyData * CreateInfoHeader(const QString &key);
    QtPropertyData * GetInfoHeader(const QString &key);
    
    void AddChild(const QString & key, QtPropertyData *parent);
    void SetChild(const QString & key, const QVariant &value, QtPropertyData *parent);
    
    
    void CollectSceneTextures();
    void CollectParticlesData();
    void CollectLODData();
    void CollectTexture(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, const DAVA::FilePath &pathname, DAVA::Texture *tex);
    
    static DAVA::uint32 CalculateTextureSize(const DAVA::Map<DAVA::String, DAVA::Texture *> &textures);

    bool IsInConsoleMode();
    
    
protected:
    
	QtPosSaver posSaver;
    PropertyEditorStateHelper treeStateHelper;
    
    DAVA::Scene * scene;
    DAVA::Vector<DAVA::Entity *> nodesAtScene;
    
    DAVA::Vector<DAVA::Material *>materialsAtScene;
    DAVA::Vector<DAVA::DataNode *>dataNodesAtScene;

    DAVA::Map<DAVA::String, DAVA::Texture *>sceneTextures;
    DAVA::Map<DAVA::String, DAVA::Texture *>particleTextures;
    
    
    DAVA::uint32 sceneTexturesSize;
    DAVA::uint32 particleTexturesSize;
    
    DAVA::uint32 emittersCount;
    DAVA::uint32 spritesCount;
    
    LODInfo lodInfo;
};

#endif // __SCENE_INFO_H__
