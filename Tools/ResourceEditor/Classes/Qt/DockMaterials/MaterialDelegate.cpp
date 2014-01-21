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


#include "MaterialDelegate.h"
#include "SimpleMaterialModel.h"

#include "Math/Color.h"

#include "Main/QtUtils.h"
#include "TextureBrowser/TextureCache.h"
#include "TextureBrowser/TextureConvertor.h"


#include <QSortFilterProxyModel>
#include <QPainter>
#include <QFileInfo>
#include <QImage>
#include <QRectF>


#define BORDER_COLOR QColor(0, 0, 0, 25)
#define HIGHLIGHTED_COLOR QColor(255, 255, 200)

#define SELECTION_BORDER_COLOR QColor(0, 0, 0, 50)
#define SELECTION_COLOR_ALPHA 100
#define INFO_TEXT_COLOR QColor(0, 0, 0, 100)


MaterialDelegate::MaterialDelegate(QSortFilterProxyModel * model, QObject *parent /* = 0 */)
	: QAbstractItemDelegate(parent)
	, drawRule(DRAW_TEXT)
    , proxyModel(model)
{
	QObject::connect(TextureCache::Instance(), SIGNAL(ThumbnailLoaded(const DAVA::TextureDescriptor *, const TextureInfo &)), this, SLOT(ThumbnailLoaded(const DAVA::TextureDescriptor *, const TextureInfo &)));
};

QSize MaterialDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if((drawRule == DRAW_PREVIEW) && (HasPreview(index)))
    {
		return QSize(option.rect.x(), PREVIEW_HEIGHT);
    }
    
    return QSize(option.rect.x(), TEXT_HEIGHT);
}


void MaterialDelegate::SetDrawRule(DrawRule rule)
{
	drawRule = rule;
}

void MaterialDelegate::ThumbnailLoaded(const DAVA::TextureDescriptor *descriptor, const TextureInfo & image)
{
	if(NULL != descriptor)
	{
        QModelIndex index = FindItemIndex(descriptor);
        emit sizeHintChanged(index);
	}
}


void MaterialDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    DAVA::NMaterial * material = GetMaterial(index);
    if(!material) return;
    
    painter->save();
    painter->setClipRect(option.rect);
    
    QRect backgroundRect = GetBackgroundRect(option);

    DrawBackground(painter, backgroundRect, material);

    int textXOffset = 0;
    if(DRAW_PREVIEW == drawRule)
    {
        const QImage img = GetPreview(option, material);
        if(img.isNull() == false)
        {
            painter->drawImage(QRect(backgroundRect.topLeft(), QSize(PREVIEW_HEIGHT, PREVIEW_HEIGHT)), img);
            textXOffset = PREVIEW_HEIGHT;
        }
    }
    
    QRect textRect(backgroundRect);
    textRect.adjust(textXOffset + 5, 0, 0, 0);
    DrawText(painter, option, textRect, material);
    
    // draw selected item
    if(option.state & QStyle::State_Selected)
    {
        DrawSelection(painter, option, material);
    }
    
    painter->restore();
}

void MaterialDelegate::DrawBackground(QPainter *painter, const QRect &rect, DAVA::NMaterial * material) const
{
    const SimpleMaterialModel *curModel = (SimpleMaterialModel *) proxyModel->sourceModel();
    if(curModel->IsMaterialSelected(material))
    {
        painter->setBrush(QBrush(HIGHLIGHTED_COLOR));
    }
    painter->setPen(BORDER_COLOR);
    painter->drawRect(rect);
}

void MaterialDelegate::DrawText(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const DAVA::NMaterial * material) const
{
    // draw text
    QRectF textRect(rect);
    textRect.adjust(0, (option.rect.height() - option.fontMetrics.height())/2, 0, 0);
    
    painter->setPen(option.palette.text().color());
    painter->drawText(textRect, material->GetMaterialName().c_str());
}

void MaterialDelegate::DrawSelection(QPainter *painter, const QStyleOptionViewItem &option, const DAVA::NMaterial *material) const
{
    QRect backgroundRect = GetBackgroundRect(option);
    QBrush br = option.palette.highlight();
    QColor cl = br.color();
    cl.setAlpha(SELECTION_COLOR_ALPHA);
    br.setColor(cl);
    painter->setBrush(br);
    painter->setPen(SELECTION_BORDER_COLOR);
    painter->drawRect(backgroundRect);
}

DAVA::NMaterial * MaterialDelegate::GetMaterial(const QModelIndex &index) const
{
    if(index.isValid() == false) return NULL;
    
    const SimpleMaterialModel *curModel = (SimpleMaterialModel *) proxyModel->sourceModel();
    return curModel->GetMaterial(proxyModel->mapToSource(index));
}

QModelIndex MaterialDelegate::FindItemIndex(const DAVA::TextureDescriptor *descriptor) const
{
    int rowCount = proxyModel->rowCount();
    for(int i = 0; i < rowCount; ++i)
    {
        const QModelIndex index = proxyModel->index(i, 0);

        const QModelIndex found = FindItemIndex(index, descriptor);
        if(found.isValid())
            return found;
    }
    
    return QModelIndex();
}

QModelIndex MaterialDelegate::FindItemIndex(const QModelIndex &parent, const DAVA::TextureDescriptor *descriptor) const
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
    
    int rowCount = proxyModel->rowCount(parent);
    for(int i = 0; i < rowCount; ++i)
    {
        const QModelIndex index = proxyModel->index(i, 0, parent);
        
        const QModelIndex found = FindItemIndex(index, descriptor);
        if(found.isValid())
            return found;
    }
    
    return QModelIndex();
}


bool MaterialDelegate::HasPreview(const QModelIndex &index) const
{
    const DAVA::NMaterial *material = GetMaterial(index);
    if(!material) return false;
    
    DAVA::Texture *t = material->GetTexture(DAVA::NMaterial::TEXTURE_ALBEDO);
    if(t)
    {
        const DAVA::Vector<QImage>& images = TextureCache::Instance()->getThumbnail(t->GetDescriptor());
        if((images.size() > 0) && (images[0].isNull() == false))
        {
            return true;
        }
        else
        {
            TextureConvertor::Instance()->GetThumbnail(t->GetDescriptor());
        }
    }
    else if(material->GetFlagValue(DAVA::NMaterial::FLAG_FLATCOLOR) == DAVA::NMaterial::FlagOn)
    {
        const DAVA::NMaterialProperty *prop = material->GetMaterialProperty(DAVA::NMaterial::PARAM_FLAT_COLOR);
        return (NULL != prop);
    }
    
    return false;
}

QImage MaterialDelegate::GetPreview(const QStyleOptionViewItem & option, const DAVA::NMaterial * material) const
{
    DAVA::Texture *t = material->GetTexture(DAVA::NMaterial::TEXTURE_ALBEDO);
    if(t)
    {
        const DAVA::Vector<QImage>& images = TextureCache::Instance()->getThumbnail(t->GetDescriptor());
        if((images.size() > 0) && (images[0].isNull() == false))
        {
            return images[0];
        }
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


QRect MaterialDelegate::GetBackgroundRect(const QStyleOptionViewItem & option) const
{
    QRect backgroundRect = option.rect;
    backgroundRect.adjust(BORDER_MARGIN, BORDER_MARGIN, -BORDER_MARGIN, -BORDER_MARGIN);

    return backgroundRect;
}


