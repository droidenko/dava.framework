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

#include "TextureListDelegate.h"
#include "TextureListModel.h"
#include "TextureCache.h"
#include "TextureBrowser.h"
#include <QPainter>
#include <QFileInfo>

#define TEXTURE_PREVIEW_SIZE 100
#define TEXTURE_PREVIEW_SIZE_SMALL 24
#define BORDER_MARGIN 1
#define BORDER_COLOR QColor(0, 0, 0, 25)
#define SELECTION_BORDER_COLOR QColor(0, 0, 0, 50)
#define SELECTION_COLOR_ALPHA 100
#define INFO_TEXT_COLOR QColor(0, 0, 0, 100)
#define FORMAT_INFO_WIDTH 4
#define FORMAT_INFO_SPACING 2

TextureListDelegate::TextureListDelegate(QObject *parent /* = 0 */)
	: QAbstractItemDelegate(parent)
	, nameFont("Arial", 10, QFont::Bold)
	, nameFontMetrics(nameFont)
	, drawRule(DRAW_PREVIEW_BIG)
{
	QObject::connect(TextureConvertor::Instance(), SIGNAL(ReadyOriginal(const DAVA::TextureDescriptor *, const QImage &)), this, SLOT(textureReadyOriginal(const DAVA::TextureDescriptor *, const QImage &)));
};

void TextureListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	switch(drawRule)
	{
	case DRAW_PREVIEW_SMALL:
		drawPreviewSmall(painter, option, index);
		break;

	case DRAW_PREVIEW_BIG:
	default:
		drawPreviewBig(painter, option, index);
		break;
	}
}

QSize TextureListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	switch(drawRule)
	{
	case DRAW_PREVIEW_SMALL:
		return QSize(option.rect.x(), TEXTURE_PREVIEW_SIZE_SMALL);
		break;

	case DRAW_PREVIEW_BIG:
	default:
		return QSize(option.rect.x(), TEXTURE_PREVIEW_SIZE);
		break;
	}
}

void TextureListDelegate::textureReadyOriginal(const DAVA::TextureDescriptor *descriptor, const QImage &image)
{
	if(NULL != descriptor)
	{
		TextureCache::Instance()->setOriginal(descriptor, image);
		if(descriptorIndexes.contains(descriptor))
		{
			QModelIndex index = descriptorIndexes[descriptor];
			descriptorIndexes.remove(descriptor);

			// this will force item with given index to redraw
			emit sizeHintChanged(index);
		}
	}
}

void TextureListDelegate::setDrawRule(DrawRure rule)
{
	drawRule = rule;
}

void TextureListDelegate::drawPreviewBig(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	const TextureListModel *curModel = (TextureListModel *) index.model();
	DAVA::TextureDescriptor *curTextureDescriptor = curModel->getDescriptor(index);

	if(NULL != curTextureDescriptor)
	{
		DAVA::Texture *curTexture = curModel->getTexture(index);

		QString texturePath = curTextureDescriptor->GetSourceTexturePathname().GetAbsolutePathname().c_str();
		QString textureName = QFileInfo(texturePath).fileName();
		QSize textureDimension = QSize();
		QVariant textureDataSize = 0;

		if(NULL != curTexture)
		{
			textureDimension = QSize(curTexture->width, curTexture->height);
			textureDataSize = curTexture->GetDataSize();
		}

		painter->save();
		painter->setClipRect(option.rect);

		// draw border
		QRect borderRect = option.rect;
		borderRect.adjust(BORDER_MARGIN, BORDER_MARGIN, -BORDER_MARGIN, -BORDER_MARGIN);
		if(curModel->isHighlited(index))
		{
			// draw highlight
			painter->setBrush(option.palette.toolTipBase());
		}
		painter->setPen(BORDER_COLOR);
		painter->drawRect(borderRect);

		QImage img = TextureCache::Instance()->getOriginal(curTextureDescriptor);

		// draw image preview
		if(!img.isNull())
		{
			QSize imageSize = img.rect().size();
			imageSize.scale(QSize(TEXTURE_PREVIEW_SIZE - option.decorationSize.width(), TEXTURE_PREVIEW_SIZE - option.decorationSize.height()), Qt::KeepAspectRatio);
			int imageX =  option.rect.x() + (TEXTURE_PREVIEW_SIZE - imageSize.width())/2;
			int imageY =  option.rect.y() + (TEXTURE_PREVIEW_SIZE - imageSize.height())/2;
			painter->drawImage(QRect(QPoint(imageX, imageY), imageSize), img);
		}
		else
		{
			// there is no image for this texture in cache
			// so load it async
			TextureConvertor::Instance()->GetOriginal(curTextureDescriptor);
			descriptorIndexes.insert(curTextureDescriptor, index);
		}

		// draw formats info
		drawFormatInfo(painter, borderRect, curTexture, curTextureDescriptor);

		// draw text info
		{
			QRectF textRect = option.rect;
			textRect.adjust(TEXTURE_PREVIEW_SIZE, option.decorationSize.height() / 2, 0, 0);

			QFont origFont = painter->font();
			painter->setPen(option.palette.text().color());
			painter->setFont(nameFont);
			painter->drawText(textRect, textureName);

			painter->setFont(origFont);
			painter->setPen(INFO_TEXT_COLOR);
			textRect.adjust(0, nameFontMetrics.height(), 0, 0);

			QString infoText;
			char dimen[32];

			sprintf(dimen, "%dx%d", textureDimension.width(), textureDimension.height());
			//infoText += "Dimension: ";
			infoText += dimen;
			infoText += "\nData size: ";
			infoText += textureDataSize.toString();

			if(curTextureDescriptor->compression[DAVA::GPU_POWERVR_IOS].format != DAVA::FORMAT_INVALID)
			{
				infoText += "\nPVR: ";
				infoText += QString(DAVA::Texture::GetPixelFormatString((DAVA::PixelFormat) curTextureDescriptor->compression[DAVA::GPU_POWERVR_IOS].format));
			}

			if(curTextureDescriptor->compression[DAVA::GPU_TEGRA].format != DAVA::FORMAT_INVALID)
			{
				infoText += "\nDXT: ";
				infoText += QString(DAVA::Texture::GetPixelFormatString((DAVA::PixelFormat) curTextureDescriptor->compression[DAVA::GPU_TEGRA].format));
			}

			painter->drawText(textRect, infoText);
		}

		// draw selected item
		if(option.state & QStyle::State_Selected)
		{
			QBrush br = option.palette.highlight();
			QColor cl = br.color();
			cl.setAlpha(SELECTION_COLOR_ALPHA);
			br.setColor(cl);
			painter->setBrush(br);
			painter->setPen(SELECTION_BORDER_COLOR);
			painter->drawRect(borderRect);
		}

		painter->restore();
	}
}

void TextureListDelegate::drawPreviewSmall(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	const TextureListModel *curModel = (TextureListModel *) index.model();
	DAVA::TextureDescriptor *curTextureDescriptor = curModel->getDescriptor(index);
	DAVA::Texture *curTexture = curModel->getTexture(index);

	if(NULL != curTextureDescriptor)
	{
		painter->save();
		painter->setClipRect(option.rect);



		// draw border
		QRect borderRect = option.rect;
		borderRect.adjust(BORDER_MARGIN, BORDER_MARGIN, -BORDER_MARGIN, -BORDER_MARGIN);
		if(curModel->isHighlited(index))
		{
			// draw highlight
			painter->setBrush(QBrush(QColor(255, 255, 200)));
		}
		painter->setPen(BORDER_COLOR);
		painter->drawRect(borderRect);

		// draw preview
		QRect previewRect = borderRect;
		previewRect.adjust(BORDER_MARGIN, BORDER_MARGIN, -BORDER_MARGIN*2, -BORDER_MARGIN*2);
		previewRect.setWidth(previewRect.height());
		painter->setBrush(QBrush(QColor(100, 100, 100)));
		painter->drawRect(previewRect);

		// draw formats info
		drawFormatInfo(painter, borderRect, curTexture, curTextureDescriptor);

		// draw text
		QRectF textRect = option.rect;
		textRect.adjust(TEXTURE_PREVIEW_SIZE_SMALL, (option.rect.height() - option.fontMetrics.height())/2, 0, 0);
		painter->setPen(option.palette.text().color());
		painter->drawText(textRect, curModel->data(index).toString());

		// draw selected item
		if(option.state & QStyle::State_Selected)
		{
			QBrush br = option.palette.highlight();
			QColor cl = br.color();
			cl.setAlpha(SELECTION_COLOR_ALPHA);
			br.setColor(cl);
			painter->setBrush(br);
			painter->setPen(SELECTION_BORDER_COLOR);
			painter->drawRect(borderRect);
		}

		painter->restore();
	}
}

void TextureListDelegate::drawFormatInfo(QPainter *painter, QRect rect, const DAVA::Texture *texture, const DAVA::TextureDescriptor *descriptor) const
{
	if(NULL != descriptor && NULL != texture)
	{
		rect.adjust(FORMAT_INFO_SPACING, FORMAT_INFO_SPACING, -FORMAT_INFO_SPACING, -FORMAT_INFO_SPACING);
		rect.setX(rect.x() + rect.width() - FORMAT_INFO_WIDTH);
		rect.setWidth(FORMAT_INFO_WIDTH);

		QColor gpuInfoColors[DAVA::GPU_FAMILY_COUNT];
		gpuInfoColors[DAVA::GPU_POWERVR_IOS] = TextureBrowser::gpuColor_PVR_ISO;
		gpuInfoColors[DAVA::GPU_POWERVR_ANDROID] = TextureBrowser::gpuColor_PVR_Android;
		gpuInfoColors[DAVA::GPU_TEGRA] = TextureBrowser::gpuColor_Tegra;
		gpuInfoColors[DAVA::GPU_MALI] = TextureBrowser::gpuColor_MALI;
		gpuInfoColors[DAVA::GPU_ADRENO] = TextureBrowser::gpuColor_Adreno;

		for(int i = 0; i < DAVA::GPU_FAMILY_COUNT; ++i)
		{
			if(descriptor->compression[i].format != DAVA::FORMAT_INVALID)
			{
				QColor c = gpuInfoColors[i];
				c.setAlpha(200);

				painter->setPen(c);
				painter->setBrush(c);
				painter->drawRect(rect);
			}

			rect.adjust(-FORMAT_INFO_WIDTH - FORMAT_INFO_SPACING, 0, -FORMAT_INFO_WIDTH - FORMAT_INFO_SPACING, 0);
		}

		if(texture->width != texture->height)
		{
			QRect r = rect;
			r.setHeight(FORMAT_INFO_WIDTH * 2);

			painter->setPen(TextureBrowser::errorColor);
			painter->setBrush(QBrush(TextureBrowser::errorColor));
			painter->drawRect(r);

			rect.adjust(-FORMAT_INFO_WIDTH - FORMAT_INFO_SPACING, 0, -FORMAT_INFO_WIDTH - FORMAT_INFO_SPACING, 0);
		}
	}
}
