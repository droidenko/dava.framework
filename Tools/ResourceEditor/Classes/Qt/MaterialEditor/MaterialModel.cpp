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



#include "MaterialModel.h"
#include "MaterialItem.h"

#include "Scene/SceneEditor2.h"
#include "Tools/MimeData/MimeDataHelper2.h"

#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Material/MaterialSystem.h"

MaterialModel::MaterialModel(QObject * parent)
    : QStandardItemModel(parent)
{ 
	QStringList headerLabels;
	headerLabels.append("Materials hierarchy");
	setHorizontalHeaderLabels(headerLabels);
}

MaterialModel::~MaterialModel()
{ }

void MaterialModel::SetScene(SceneEditor2 *scene)
{
	removeRows(0, rowCount());

	if(NULL != scene)
	{
		QStandardItem *root = invisibleRootItem();
		DAVA::MaterialSystem *matSys = scene->renderSystem->GetMaterialSystem();

		DAVA::Vector<DAVA::NMaterial *> materials;
		matSys->BuildMaterialList(NULL, materials);

		for(DAVA::uint32 i = 0; i < (DAVA::uint32)materials.size(); ++i)
		{
			if(GetLevel(materials[i]) == 1)
			{
				MaterialItem *item = new MaterialItem(materials[i], 1);
				item->setDragEnabled(false);
				root->appendRow(item);
			}
		}
	}
}

DAVA::NMaterial * MaterialModel::GetMaterial(const QModelIndex & index) const
{
    if(!index.isValid()) return NULL;
    
	MaterialItem *item = (MaterialItem *) itemFromIndex(index);
    return item->GetMaterial();
}

QModelIndex MaterialModel::GetIndex(DAVA::NMaterial *material, const QModelIndex &parent) const
{
	QModelIndex ret = QModelIndex();

	MaterialItem* item = (MaterialItem*)itemFromIndex(parent);
	if(NULL != item && item->GetMaterial() == material)
	{
		ret = parent;
	}
	else
	{
		QStandardItem *sItem = (NULL != item) ? item : invisibleRootItem();
		if(NULL != sItem)
		{
			for(int i = 0; i < sItem->rowCount(); ++i)
			{
				ret = GetIndex(material, index(i, 0, parent));
				if(ret.isValid())
				{
					break;
				}
			}
		}
	}

	return ret;
}

QMimeData * MaterialModel::mimeData(const QModelIndexList & indexes) const
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

QStringList MaterialModel::mimeTypes() const
{
	QStringList types;
    
	types << MimeDataHelper2<DAVA::NMaterial>::GetMimeType();
    
	return types;
}

bool MaterialModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	bool ret = false;
	QVector<DAVA::NMaterial *> materials = MimeDataHelper2<DAVA::NMaterial>::DecodeMimeData(data);

	if(materials.size() > 0)
	{
		if(dropCanBeAccepted(data, action, row, column, parent))
		{
			DAVA::NMaterial *targetMaterial = GetMaterial(parent);
			MaterialItem *targetMaterialItem = (MaterialItem *) itemFromIndex(parent);

			for(int i = 0; i < materials.size(); ++i)
			{
				MaterialItem *sourceMaterialItem = (MaterialItem *) itemFromIndex(GetIndex(materials[i]));
				if(NULL != sourceMaterialItem)
				{
					materials[i]->SetParent(targetMaterial);
					((MaterialItem *) sourceMaterialItem->parent())->Sync();
				}
			}

			targetMaterialItem->Sync();
		}
	}

	return ret;
}

bool MaterialModel::dropCanBeAccepted(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	bool ret = false;
	QVector<DAVA::NMaterial *> materials = MimeDataHelper2<DAVA::NMaterial>::DecodeMimeData(data);

	if(materials.size() > 0)
	{
		// allow only direct drop to parent
		if(row == -1 && column == -1)
		{
			bool foundInacceptable = false;
			DAVA::NMaterial *targetMaterial = GetMaterial(parent);
			MaterialItem *targetMaterialItem = (MaterialItem *) itemFromIndex(parent);

			switch(targetMaterialItem->GetLevel())
			{
			case 1:
				{
					// dropping into level 1 material
					// accept only level 2 materials
					for(int i = 0; i < materials.size(); ++i)
					{
						if(GetLevel(materials[i]) != 2)
						{
							foundInacceptable = true;
							break;
						}
					}
				}
				break;

			case 2:
				{
					// dropping into level 2 material
					// accept only level 3 materials
					for(int i = 0; i < materials.size(); ++i)
					{
						if(GetLevel(materials[i]) != 3)
						{
							foundInacceptable = true;
							break;
						}
					}
				}
				break;

			case 0:
			default:
				foundInacceptable = true;
				break;
			}

			ret = !foundInacceptable;
		}
	}

	return ret;
}

int MaterialModel::GetLevel(DAVA::NMaterial *material) const
{
	int ret = 0;

	if(NULL != material)
	{
		// determine material level
		if(material->IsConfigMaterial() && material->IsSwitchable())
		{
			ret = 1;
		}
		else if(material->IsSwitchable())
		{
			if(NULL != material->GetParent() &&
				material->GetParent()->IsConfigMaterial())
			{
				ret = 2;
			}
			else
			{
				ret = 3;
			}
		}
	}

	return ret;
}

MaterialFilteringModel::MaterialFilteringModel(MaterialModel *_materialModel, QObject *parent /*= NULL*/)
: QSortFilterProxyModel(parent)
, materialModel(_materialModel)
{ 
	setSourceModel(materialModel);
}

bool MaterialFilteringModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	bool ret = false;

	if(NULL != materialModel)
	{
		DAVA::NMaterial *materialLeft = materialModel->GetMaterial(left);
		DAVA::NMaterial *materialRight = materialModel->GetMaterial(right);

		if( materialModel->GetLevel(materialLeft) == 1 &&
			materialModel->GetLevel(materialRight) == 1)
		{
			if( materialLeft->GetChildrenCount() > 0 && materialRight->GetChildrenCount() > 0 ||
				materialLeft->GetChildrenCount() == 0 && materialRight->GetChildrenCount() == 0)
			{
				ret = (strcmp(materialLeft->GetMaterialName().c_str(), materialRight->GetMaterialName().c_str()) < 0);
			}
			else if(materialLeft->GetChildrenCount() > 0 && materialRight->GetChildrenCount() == 0)
			{
				ret = true;
			}
		}
	}

	return ret;
}