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

#ifndef __SCENE_INFO_H__
#define __SCENE_INFO_H__

#include "Tools/QtPosSaver/QtPosSaver.h"
#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "DockProperties/PropertyEditorStateHelper.h"

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
