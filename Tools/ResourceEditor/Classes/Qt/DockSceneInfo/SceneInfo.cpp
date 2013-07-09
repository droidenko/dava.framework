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

#include "DAVAEngine.h"
#include "DockSceneInfo/SceneInfo.h"
#include "Scene/SceneDataManager.h"
#include "../../SceneEditor/EditorSettings.h"
#include "../../EditorScene.h"

#include "../../CommandLine/CommandLineManager.h"


#include "Main/QtUtils.h"

#include "Render/TextureDescriptor.h"

#include "../../ImageTools/ImageTools.h"


#include <QHeaderView>
#include <QTimer>
#include <QPalette>

using namespace DAVA;

SceneInfo::SceneInfo(QWidget *parent /* = 0 */)
	: QtPropertyEditor(parent)
    , scene(NULL)
    , treeStateHelper(this, this->curModel)
{
	// global scene manager signals
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneActivated(SceneData *)), this, SLOT(sceneActivated(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneChanged(SceneData *)), this, SLOT(sceneChanged(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneReleased(SceneData *)), this, SLOT(sceneReleased(SceneData *)));

	// MainWindow actions
	posSaver.Attach(this, "DockSceneInfo");
	
	VariantType v = posSaver.LoadValue("splitPos");
	if(v.GetType() == VariantType::TYPE_INT32) header()->resizeSection(0, v.AsInt32());

    ClearData();
    
    InitializeInfo();
    
    viewport()->setBackgroundRole(QPalette::Window);
}

SceneInfo::~SceneInfo()
{
	VariantType v(header()->sectionSize(0));
	posSaver.SaveValue("splitPos", v);
}

void SceneInfo::sceneChanged(SceneData *sceneData)
{
    if(NULL == sceneData) return;
    
	RefreshAllData(sceneData);
}

void SceneInfo::sceneActivated(SceneData *sceneData)
{
    if(NULL == sceneData) return;
    
	RefreshAllData(sceneData);
}

void SceneInfo::sceneReleased(SceneData *sceneData)
{ }



void SceneInfo::InitializeInfo()
{
    RemovePropertyAll();
    
    InitializeGeneralSection();
    Initialize3DDrawSection();
    InitializeMaterialsSection();
    InitializeLODSection();
    InitializeParticlesSection();
}

void SceneInfo::InitializeGeneralSection()
{
    QtPropertyData* header = CreateInfoHeader("General Scene Info");
  
    AddChild("Entities Count", header);
    AddChild("DataNodes Count", header);
    AddChild("All Textures Size", header);
}

void SceneInfo::RefreshSceneGeneralInfo()
{
    QtPropertyData* header = GetInfoHeader("General Scene Info");

    SetChild("Entities Count", (uint32)nodesAtScene.size(), header);
    SetChild("DataNodes Count", (uint32)dataNodesAtScene.size(), header);
    SetChild("All Textures Size", QString::fromStdString(SizeInBytesToString((float32)(sceneTexturesSize + particleTexturesSize))), header);
}

void SceneInfo::Initialize3DDrawSection()
{
    QtPropertyData* header = CreateInfoHeader("DrawInfo");

    AddChild("ArraysCalls", header);
    AddChild("ElementsCalls",  header);
    AddChild("PointsList", header);
    AddChild("LineList", header);
    AddChild("LineStrip", header);
    AddChild("TriangleList", header);
    AddChild("TriangleStrip", header);
    AddChild("TriangleFan", header);
}

void SceneInfo::Refresh3DDrawInfo()
{
    QtPropertyData* header = GetInfoHeader("DrawInfo");
    
    SceneData *sceneData = SceneDataManager::Instance()->SceneGetActive();
    EditorScene *scene = sceneData->GetScene();

    const RenderManager::Stats & renderStats = scene->GetRenderStats();
    
    SetChild("ArraysCalls", renderStats.drawArraysCalls, header);
    SetChild("ElementsCalls", renderStats.drawElementsCalls, header);
    SetChild("PointsList", renderStats.primitiveCount[PRIMITIVETYPE_POINTLIST], header);
    SetChild("LineList", renderStats.primitiveCount[PRIMITIVETYPE_LINELIST], header);
    SetChild("LineStrip", renderStats.primitiveCount[PRIMITIVETYPE_LINESTRIP], header);
    SetChild("TriangleList", renderStats.primitiveCount[PRIMITIVETYPE_TRIANGLELIST], header);
    SetChild("TriangleStrip", renderStats.primitiveCount[PRIMITIVETYPE_TRIANGLESTRIP], header);
    SetChild("TriangleFan", renderStats.primitiveCount[PRIMITIVETYPE_TRIANGLEFAN], header);
}

void SceneInfo::InitializeMaterialsSection()
{
    QtPropertyData* header = CreateInfoHeader("Materials");
    
    AddChild("Materials Count", header);
    AddChild("Textures Count", header);
    AddChild("Textures Size", header);
}

void SceneInfo::RefreshMaterialsInfo()
{
    QtPropertyData* header = GetInfoHeader("Materials");
    SetChild("Materials Count", (uint32)materialsAtScene.size(), header);
    SetChild("Textures Count", (uint32)sceneTextures.size(), header);
    SetChild("Textures Size", QString::fromStdString(SizeInBytesToString((float32)sceneTexturesSize)), header);
}

void SceneInfo::InitializeLODSection()
{
    QtPropertyData* header = CreateInfoHeader("LOD");
    
    for(int32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
    {
        AddChild(Format("Objects LOD%d Triangles", i), header);
    }
    
    AddChild("Objects LOD Triangles", header);
    AddChild("Landscape Triangles", header);
    AddChild("All Triangles", header);
}

void SceneInfo::RefreshLODInfo()
{
    QtPropertyData* header = GetInfoHeader("LOD");

    uint32 objectTriangles = 0;
    for(int32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
    {
        SetChild(Format("Objects LOD%d Triangles", i), lodInfo.trianglesOnLod[i], header);
        
        objectTriangles += lodInfo.trianglesOnLod[i];
    }
    
    SetChild("Objects LOD Triangles", objectTriangles, header);
    SetChild("Landscape Triangles", lodInfo.trianglesOnLandscape, header);
    SetChild("All Triangles", lodInfo.trianglesOnLandscape + objectTriangles, header);
}


void SceneInfo::InitializeParticlesSection()
{
    QtPropertyData* header = CreateInfoHeader("Particles");

    AddChild("Emitters Count", header);
    AddChild("Sprites Count", header);
    AddChild("Textures Count", header);
    AddChild("Textures Size", header);
}

void SceneInfo::RefreshParticlesInfo()
{
    QtPropertyData* header = GetInfoHeader("Particles");

    SetChild("Emitters Count", emittersCount, header);
    SetChild("Sprites Count", spritesCount, header);
    SetChild("Textures Count", (uint32)particleTextures.size(), header);
    SetChild("Textures Size", QString::fromStdString(SizeInBytesToString((float32)particleTexturesSize)), header);
}

uint32 SceneInfo::CalculateTextureSize(const Map<String, Texture *> &textures)
{
    KeyedArchive *settings = EditorSettings::Instance()->GetSettings();
    String projectPath = settings->GetString("ProjectPath");

    uint32 textureSize = 0;
    
    Map<String, Texture *>::const_iterator endIt = textures.end();
    for(Map<String, Texture *>::const_iterator it = textures.begin(); it != endIt; ++it)
    {
        FilePath pathname = it->first;
        Texture *tex = it->second;
        DVASSERT(tex);
        
        if(String::npos == pathname.GetAbsolutePathname().find(projectPath))
        {
            Logger::Warning("[SceneInfo::CalculateTextureSize] Path (%s) doesn't belong to project.", pathname.GetAbsolutePathname().c_str());
            continue;
        }
        
        TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(pathname);
        if(!descriptor)
        {
            Logger::Error("[SceneInfo::CalculateTextureSize] Can't create descriptor for texture %s", pathname.GetAbsolutePathname().c_str());
            continue;
        }
        
        textureSize += ImageTools::GetTexturePhysicalSize(descriptor, EditorSettings::Instance()->GetTextureViewGPU());
        
        SafeRelease(descriptor);
    }

    return textureSize;
}



void SceneInfo::CollectSceneData(SceneData *sceneData, bool force)
{
    if(sceneData->GetScene() != scene || force)
    {
        ClearData();
        
        scene = sceneData->GetScene();
        DVASSERT(scene);
 
        scene->GetChildNodes(nodesAtScene);
        scene->GetDataNodes(materialsAtScene);
        scene->GetDataNodes(dataNodesAtScene);
        
        CollectSceneTextures();
        CollectParticlesData();
        
        sceneTexturesSize = CalculateTextureSize(sceneTextures);
        particleTexturesSize = CalculateTextureSize(particleTextures);
    }
}

void SceneInfo::ClearData()
{
    nodesAtScene.clear();
    materialsAtScene.clear();
    dataNodesAtScene.clear();
    sceneTextures.clear();
    particleTextures.clear();
    
    lodInfo.Clear();
    
    sceneTexturesSize = 0;
    particleTexturesSize = 0;
    emittersCount = 0;
    spritesCount = 0;
}

void SceneInfo::CollectSceneTextures()
{
    for(int32 n = 0; n < (int32)nodesAtScene.size(); ++n)
    {
        RenderObject *ro = GetRenderObject(nodesAtScene[n]);
        if(!ro) continue;
        
        uint32 count = ro->GetRenderBatchCount();
        for(uint32 b = 0; b < count; ++b)
        {
            RenderBatch *renderBatch = ro->GetRenderBatch(b);
            
            Material *material = renderBatch->GetMaterial();
            if(material)
            {
                for(int32 t = 0; t < Material::TEXTURE_COUNT; ++t)
                {
                    CollectTexture(sceneTextures, material->GetTextureName((Material::eTextureLevel)t), material->GetTexture((Material::eTextureLevel)t));
                }
            }
            
            InstanceMaterialState *instanceMaterial = renderBatch->GetMaterialInstance();
            if(instanceMaterial)
            {
                CollectTexture(sceneTextures, instanceMaterial->GetLightmapName(), instanceMaterial->GetLightmap());
            }
        }
        
        if(ro->GetType() == RenderObject::TYPE_LANDSCAPE)
        {
            Landscape *land = static_cast<Landscape *>(ro);
            for(int32 t = 0; t < Landscape::TEXTURE_COUNT; ++t)
            {
                CollectTexture(sceneTextures, land->GetTextureName((Landscape::eTextureLevel)t), land->GetTexture((Landscape::eTextureLevel)t));
            }
        }
    }
}

void SceneInfo::CollectParticlesData()
{
    Set<Sprite *>sprites;

    emittersCount = 0;
    for(uint32 n = 0; n < nodesAtScene.size(); ++n)
    {
        ParticleEmitter *emitter = GetEmitter(nodesAtScene[n]);
        if(!emitter) continue;
        
        ++emittersCount;
        
        Vector<ParticleLayer*> &layers = emitter->GetLayers();
        
        for(uint32 lay = 0; lay < layers.size(); ++lay)
        {
            Sprite *spr = layers[lay]->GetSprite();
            if(spr)
            {
                sprites.insert(spr);
                
                for(int32 fr = 0; fr < spr->GetFrameCount(); ++fr)
                {
                    Texture *tex = spr->GetTexture(fr);
                    CollectTexture(particleTextures, tex->GetPathname(), tex);
                }
            }
        }
    }
    
    spritesCount = (uint32)sprites.size();
}

void SceneInfo::CollectLODData()
{
    //TODO: Will be done later
}


void SceneInfo::CollectTexture(Map<String, Texture *> &textures, const FilePath &name, Texture *tex)
{
    if(!name.IsEmpty() && tex)
	{
		textures[name.GetAbsolutePathname()] = tex;
	}
}


QtPropertyData * SceneInfo::CreateInfoHeader(const QString &key)
{
    QtPropertyData* headerData = new QtPropertyData("");
    headerData->SetFlags(QtPropertyData::FLAG_IS_NOT_EDITABLE);

    QPair<QtPropertyItem*, QtPropertyItem*> prop = AppendProperty(key, headerData);
    prop.first->setBackground(QBrush(QColor(Qt::lightGray)));
    prop.second->setBackground(QBrush(QColor(Qt::lightGray)));
    
    return headerData;
}

QtPropertyData * SceneInfo::GetInfoHeader(const QString &key)
{
    QtPropertyData * header = GetPropertyData(key);
    DVASSERT(header);
    
    return header;
}

void SceneInfo::AddChild(const QString & key, QtPropertyData *parent)
{
    QtPropertyData *propData = new QtPropertyData(0);
    propData->SetFlags(QtPropertyData::FLAG_IS_NOT_EDITABLE);
    parent->ChildAdd(key, propData);
}

void SceneInfo::SetChild(const QString & key, const QVariant &value, QtPropertyData *parent)
{
    for (int32 c = 0; c < parent->ChildCount(); ++c)
    {
        QPair<QString, QtPropertyData*> pair = parent->ChildGet(c);
        if(pair.first == key)
        {
            pair.second->SetValue(value);
            return;
        }
    }
}

void SceneInfo::SaveTreeState()
{
    // Store the current Property Editor Tree state before switching to the new node.
	// Do not clear the current states map - we are using one storage to share opened
	// Property Editor nodes between the different Scene Nodes.
	treeStateHelper.SaveTreeViewState(false);
}

void SceneInfo::RestoreTreeState()
{
    // Restore back the tree view state from the shared storage.
	if (!treeStateHelper.IsTreeStateStorageEmpty())
	{
		treeStateHelper.RestoreTreeViewState();
	}
	else
	{
		// Expand the root elements as default value.
		expandToDepth(0);
	}
}

void SceneInfo::showEvent ( QShowEvent * event )
{
    QtPropertyEditor::showEvent(event);
    
    if(isVisible() && !CommandLineManager::Instance()->IsCommandLineModeEnabled())
    {
        QTimer::singleShot(1000, this, SLOT(timerDone()));
    }
}

void SceneInfo::timerDone()
{
    SceneData *sceneData = SceneDataManager::Instance()->SceneGetActive();
    if(sceneData)
        RefreshAllData(sceneData);
    
    if(isVisible())
    {
        QTimer::singleShot(1000, this, SLOT(timerDone()));
    }
}

void SceneInfo::RefreshAllData(SceneData *sceneData)
{
	CollectSceneData(sceneData, true);

	SaveTreeState();

	RefreshSceneGeneralInfo();
	Refresh3DDrawInfo();
	RefreshMaterialsInfo();
	RefreshLODInfo();
	RefreshParticlesInfo();

	RestoreTreeState();
}

