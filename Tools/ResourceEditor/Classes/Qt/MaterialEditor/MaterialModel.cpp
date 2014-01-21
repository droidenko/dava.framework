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
#include "Scene/EntityGroup.h"

#include "Main/QtUtils.h"
#include "Tools/MimeData/MimeDataHelper2.h"
#include "Commands2/MaterialSwitchParentCommand.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Systems/MaterialSystem.h"

#include "TextureBrowser/TextureCache.h"
#include "TextureBrowser/TextureConvertor.h"
#include "TextureBrowser/TextureInfo.h"

#include <QDebug>


namespace
{
    const int PREVIEW_HEIGHT = 24;
}


MaterialModel::MaterialModel(QObject * parent)
    : QStandardItemModel(parent)
	, curScene(NULL)
{ 
	QStringList headerLabels;
	headerLabels.append("Materials hierarchy");
	setHorizontalHeaderLabels(headerLabels);

    connect( TextureCache::Instance(), SIGNAL(ThumbnailLoaded(const DAVA::TextureDescriptor *, const TextureInfo &)), SLOT(ThumbnailLoaded(const DAVA::TextureDescriptor *, const TextureInfo &)) );
}

MaterialModel::~MaterialModel()
{ }

void MaterialModel::SetScene(SceneEditor2 *scene)
{
	removeRows(0, rowCount());
	curScene = scene;

	if(NULL != scene)
	{
		Sync();
	}
}

void MaterialModel::SetSelection(const EntityGroup *group)
{
	QStandardItem *root = invisibleRootItem();
	for(int i = 0; i < root->rowCount(); ++i)
	{
		MaterialItem *topItem = (MaterialItem *) root->child(i);

		for(int j = 0; j < topItem->rowCount(); ++j)
		{
			MaterialItem *childItem = (MaterialItem *) topItem->child(j);

			if(NULL != group)
			{
				DAVA::NMaterial *material = childItem->GetMaterial();
				DAVA::Entity *entity = curScene->materialSystem->GetEntity(material);

				entity = curScene->selectionSystem->GetSelectableEntity(entity);
				childItem->SetFlag(MaterialItem::IS_PART_OF_SELECTION, group->HasEntity(entity));
			}
			else
			{
				childItem->SetFlag(MaterialItem::IS_PART_OF_SELECTION, false);
			}
		}
	}
}

void MaterialModel::Sync()
{
	if(NULL != curScene)
	{
		DAVA::Map<DAVA::NMaterial*, DAVA::Set<DAVA::NMaterial *> > materialsTree;
		curScene->materialSystem->BuildMaterialsTree(materialsTree);

		// remove items, that are not in set
		QStandardItem *root = invisibleRootItem();
		for(int i = 0; i < root->rowCount(); ++i)
		{
			MaterialItem *item = (MaterialItem *) root->child(i);
			if(0 == materialsTree.count(item->GetMaterial()))
			{
				root->removeRow(i--);
			}
			else
			{
				// there is same material, so we should check it childs
				// and remove those that are not in tree

				const DAVA::Set<DAVA::NMaterial *> &childsList = materialsTree[item->GetMaterial()];

				for(int j = 0; j < item->rowCount(); ++j)
				{
					MaterialItem *child = (MaterialItem *) item->child(j);
					if(0 == childsList.count(child->GetMaterial()))
					{
						item->removeRow(j--);
					}
				}
			}
		}

		// add items, that are not added yet
		auto it = materialsTree.begin();
		auto end = materialsTree.end();
		for(; it != end; ++it)
		{
			DAVA::NMaterial *toAdd = it->first;
			QModelIndex index = GetIndex(toAdd);
            MaterialItem *item = NULL;

			// still no such material in model?
			if(!index.isValid())
			{
				item = new MaterialItem(toAdd);

				// and it childs
				auto cit = it->second.begin();
				auto cend = it->second.end();
				for(; cit != cend; ++cit)
				{
					item->appendRow(new MaterialItem(*cit));
				}

				// add created item
				root->appendRow(item);
			}
			else
			{
				// there is already such material in model
				// we should sync it childs

				item = (MaterialItem *) itemFromIndex(index);
				auto cit = it->second.begin();
				auto cend = it->second.end();

				for(; cit != cend; ++cit)
				{
					DAVA::NMaterial *childMaterial = *cit;

					// no such item?
					if(!GetIndex(childMaterial, index).isValid())
					{
						item->appendRow(new MaterialItem(childMaterial));
					}
				}
			}

            setPreview( item, toAdd );
		}

		// mark materials that can be deleted
		for(int i = 0; i < root->rowCount(); ++i)
		{
			MaterialItem *item = (MaterialItem *) root->child(i);
			item->SetFlag(MaterialItem::IS_MARK_FOR_DELETE, item->rowCount() == 0);
		}
	}

	emit dataChanged(QModelIndex(), QModelIndex());
}

QImage MaterialModel::GetPreview( const DAVA::NMaterial * material ) const
{
    DAVA::Texture *t = material->GetTexture(DAVA::NMaterial::TEXTURE_ALBEDO);
    if(t)
    {
        const DAVA::Vector<QImage>& images = TextureCache::Instance()->getThumbnail(t->GetDescriptor());
        if((images.size() > 0) && (images[0].isNull() == false))
            return images[0];
        else
            TextureConvertor::Instance()->GetThumbnail(t->GetDescriptor());
    }
    else if(material->GetFlagValue(DAVA::NMaterial::FLAG_FLATCOLOR) == DAVA::NMaterial::FlagOn)
    {
        const DAVA::NMaterialProperty *prop = material->GetMaterialProperty(DAVA::NMaterial::PARAM_FLAT_COLOR);
        if(prop)
        {
            const DAVA::Color color = *(DAVA::Color*)prop->data;
            
            QImage img(QSize(PREVIEW_HEIGHT, PREVIEW_HEIGHT), QImage::Format_ARGB32);
            img.fill(ColorToQColor(color));
            
            return img;
        }
    }

    return QImage();
}

void MaterialModel::setPreview( QStandardItem *item, const DAVA::NMaterial * material )
{
    item->setData( QSize( PREVIEW_HEIGHT, PREVIEW_HEIGHT ), Qt::SizeHintRole );

    const QImage& preview = GetPreview( material );
    if ( !preview.isNull() )
        item->setData( preview.scaled( QSize( PREVIEW_HEIGHT, PREVIEW_HEIGHT ), Qt::KeepAspectRatio, Qt::FastTransformation ), Qt::DecorationRole );
}

void MaterialModel::ThumbnailLoaded(const DAVA::TextureDescriptor *descriptor, const TextureInfo & image)
{
	if(NULL != descriptor)
	{
        QModelIndex index = FindItemIndex(descriptor);
        if ( !index.isValid() )
            return ;

        MaterialItem *item = (MaterialItem *)itemFromIndex(index);
        if ( !item )
            return ;
        DAVA::NMaterial* material = item->GetMaterial();
        const DAVA::Vector<QImage>& images = image.images;
        if ( images.size() > 0 )
            item->setData( images[0], Qt::DecorationRole );
        //setPreview( item, material );
	}
}

QModelIndex MaterialModel::FindItemIndex(const DAVA::TextureDescriptor *descriptor) const
{
    int nRows = rowCount();
    for(int i = 0; i < nRows; ++i)
    {
        const QModelIndex idx = index(i, 0);

        const QModelIndex found = FindItemIndex(idx, descriptor);
        if(found.isValid())
            return found;
    }
    
    return QModelIndex();
}

QModelIndex MaterialModel::FindItemIndex(const QModelIndex &parent, const DAVA::TextureDescriptor *descriptor) const
{
    if(parent.isValid() == false) return QModelIndex();
    
    const DAVA::NMaterial *material = GetMaterial(parent);
    if(material)
    {
        DAVA::Texture *t = material->GetTexture(DAVA::NMaterial::TEXTURE_ALBEDO);
        if(t && t->GetDescriptor() == descriptor)
        {
            return parent;
        }
    }
    
    int nRows = rowCount(parent);
    for(int i = 0; i < nRows; ++i)
    {
        const QModelIndex idx = index(i, 0, parent);
        const QModelIndex found = FindItemIndex(idx, descriptor);
        if(found.isValid())
            return found;
    }
    
    return QModelIndex();
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
			MaterialItem *targetMaterialItem = (MaterialItem *) itemFromIndex(parent);
			DAVA::NMaterial *targetMaterial = targetMaterialItem->GetMaterial();

			if(materials.size() > 1)
			{
				curScene->BeginBatch("Change materials parent");
			}

			// change parent material
			// NOTE: model synchronization will be done in OnCommandExecuted handler
			for(int i = 0; i < materials.size(); ++i)
			{
				MaterialItem *sourceMaterialItem = (MaterialItem *) itemFromIndex(GetIndex(materials[i]));
				if(NULL != sourceMaterialItem)
				{
					curScene->Exec(new MaterialSwitchParentCommand(materials[i], targetMaterial));
				}
			}

			if(materials.size() > 1)
			{
				curScene->EndBatch();
			}
		
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
			DAVA::NMaterial *targetMaterial = GetMaterial(parent);

			// allow drop only into material, but not into instance
			if(NULL != targetMaterial)
			{
				bool foundInacceptable = false;
				
				if(targetMaterial->GetMaterialType() == DAVA::NMaterial::MATERIALTYPE_MATERIAL)
				{
					// only instance type should be in mime data
					for(int i = 0; i < materials.size(); ++i)
					{
						if(materials[i]->GetMaterialType() != DAVA::NMaterial::MATERIALTYPE_INSTANCE)
						{
							foundInacceptable = true;
							break;
						}
					}
				}

				ret = !foundInacceptable;
			}
		}
	}

	return ret;
}
