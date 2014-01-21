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



#ifndef __MATERIAL_DELEGATE_H__
#define __MATERIAL_DELEGATE_H__

#include "Render/Material/NMaterial.h"
#include "Render/TextureDescriptor.h"

#include "TextureBrowser/TextureInfo.h"

#include <QAbstractItemDelegate>

class QSortFilterProxyModel;
class QPainter;
class MaterialDelegate: public QAbstractItemDelegate
{
	Q_OBJECT
    
    static const int PREVIEW_HEIGHT = 32;
    static const int TEXT_HEIGHT = 24;
    static const int BORDER_MARGIN = 1;

public:

	enum DrawRule
	{
		DRAW_PREVIEW,
		DRAW_TEXT
	};

	MaterialDelegate(QSortFilterProxyModel * model, QObject *parent = 0);

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

	void SetDrawRule(DrawRule rule);

private slots:

    void ThumbnailLoaded(const DAVA::TextureDescriptor *descriptor, const TextureInfo & image);

private:
    QRect GetBackgroundRect(const QStyleOptionViewItem & option) const;
    
    void DrawBackground(QPainter *painter, const QRect &rect, DAVA::NMaterial * material) const;
	void DrawText(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect,  const DAVA::NMaterial * material) const;
    void DrawSelection(QPainter *painter, const QStyleOptionViewItem &option, const DAVA::NMaterial * material) const;
    
    DAVA::NMaterial * GetMaterial(const QModelIndex &index) const;
    bool HasPreview(const QModelIndex &index) const;
    QImage GetPreview(const QStyleOptionViewItem & option, const DAVA::NMaterial * material) const;
    
    QModelIndex FindItemIndex(const DAVA::TextureDescriptor *descriptor) const;
    QModelIndex FindItemIndex(const QModelIndex &parent, const DAVA::TextureDescriptor *descriptor) const;
    
private:
    
	DrawRule drawRule;
    
    QSortFilterProxyModel *proxyModel;
};

#endif // __MATERIAL_DELEGATE_H__
