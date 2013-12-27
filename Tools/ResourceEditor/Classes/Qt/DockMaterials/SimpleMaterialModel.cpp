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



#include "SimpleMaterialModel.h"
#include "SimpleMaterialItem.h"

#include "Scene/SceneEditor2.h"
#include "Scene/EntityGroup.h"
#include "Tools/MimeData/MimeDataHelper2.h"

#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Material/MaterialSystem.h"

SimpleMaterialModel::SimpleMaterialModel(QObject * parent)
    : QStandardItemModel(parent)
{ 
}

SimpleMaterialModel::~SimpleMaterialModel()
{ }

void SimpleMaterialModel::SetScene(SceneEditor2 *scene)
{
	removeRows(0, rowCount());
    selectedMaterials.clear();
    
	if(NULL != scene)
	{
		QStandardItem *root = invisibleRootItem();
		DAVA::MaterialSystem *matSys = scene->renderSystem->GetMaterialSystem();

		DAVA::Vector<DAVA::NMaterial *> materials;
		matSys->BuildMaterialList(NULL, materials);

		for(DAVA::uint32 i = 0; i < (DAVA::uint32)materials.size(); ++i)
		{
			if(materials[i]->IsSwitchable() && materials[i]->IsConfigMaterial())
			{
                if(IsMaterialValidForModel(materials[i]) == false)
                    continue;
                
                for(DAVA::int32 m = 0; m < materials[i]->GetChildrenCount(); ++m)
                {
                    SimpleMaterialItem *item = new SimpleMaterialItem(materials[i]->GetChild(m));
                    root->appendRow(item);
                }
			}
		}
        
        SetSelection(scene->selectionSystem->GetSelection());
	}
}

DAVA::NMaterial * SimpleMaterialModel::GetMaterial(const QModelIndex & index) const
{
    if(!index.isValid()) return NULL;
    
	SimpleMaterialItem *item = (SimpleMaterialItem *) itemFromIndex(index);
    return item->GetMaterial();
}

QMimeData * SimpleMaterialModel::mimeData(const QModelIndexList & indexes) const
{
	if(indexes.size() > 0)
	{
        QVector<DAVA::NMaterial*> data;
        foreach(QModelIndex index, indexes)
        {
            data.push_back(GetMaterial(index));
        }
        
        return MimeDataHelper2<DAVA::NMaterial>::EncodeMimeData(data);
    }
    
	return NULL;
}

QStringList SimpleMaterialModel::mimeTypes() const
{
	QStringList types;
    
	types << MimeDataHelper2<DAVA::NMaterial>::GetMimeType();
    
	return types;
}

bool SimpleMaterialModel::IsMaterialValidForModel(const DAVA::NMaterial * material) const
{
    const DAVA::FastName & name = material->GetMaterialName();
    
    bool invalid =
        IsFastNameContains(name, "ShadowVolume")   ||
        IsFastNameContains(name, "Silhouette");
    
    return !invalid;
}


bool SimpleMaterialModel::IsFastNameContains(const DAVA::FastName & name, const DAVA::String & partOfName) const
{
    const DAVA::String materialName = name.c_str();
    return (materialName.find(partOfName) != DAVA::String::npos);
}

void SimpleMaterialModel::SetSelection(const EntityGroup & selection)
{
    selectedMaterials.clear();
    
    DAVA::Vector<DAVA::Entity *> entitiesWithRenderComponent;

    size_t count = selection.Size();
    for(size_t i = 0; i < count; ++i)
    {
        DAVA::Entity *entity = selection.GetEntity(i);
        
        entity->GetChildEntitiesWithComponent(entitiesWithRenderComponent, DAVA::Component::RENDER_COMPONENT);
        if(GetRenderObject(entity))
        {
            entitiesWithRenderComponent.push_back(entity);
        }
    }
    
    count = entitiesWithRenderComponent.size();
    for(size_t i = 0; i < count; ++i)
    {
        RenderObject *ro = GetRenderObject(entitiesWithRenderComponent[i]);
        if(!ro) continue;
        
        DAVA::uint32 rbCount = ro->GetRenderBatchCount();
        for(DAVA::uint32 r = 0; r < rbCount; ++r)
        {
            DAVA::NMaterial * mat = ro->GetRenderBatch(r)->GetMaterial();
            if(mat)
            {
                selectedMaterials.insert(mat->GetParent());
            }
        }
    }
}

bool SimpleMaterialModel::IsMaterialSelected(const DAVA::NMaterial *material) const
{
    return (selectedMaterials.find(material) != selectedMaterials.end());
}

