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


#include <QSet>

#include "MaterialItem.h"
#include "MaterialModel.h"

MaterialItem::MaterialItem(DAVA::NMaterial * _material)
    : QStandardItem()
    , material(_material)
{
	DVASSERT(material);

	static QImage materialIcon(QString::fromUtf8(":/QtIcons/sphere.png"));
	static QImage instanceIcon(QString::fromUtf8(":/QtIcons/3d.png"));
	
	setEditable(false);
    setData(QVariant::fromValue<DAVA::NMaterial *>(material));
    
	switch(material->GetMaterialType())
	{
		case DAVA::NMaterial::MATERIALTYPE_MATERIAL:
			setData( materialIcon, Qt::DecorationRole );
			setDragEnabled(true);
			setDropEnabled(true);
			break;

		case DAVA::NMaterial::MATERIALTYPE_INSTANCE:
			setData( instanceIcon, Qt::DecorationRole );
			setDragEnabled(true);
			setDropEnabled(false);
			break;

		default:
			setDragEnabled(false);
			setDropEnabled(false);
			break;
	}
}

MaterialItem::~MaterialItem()
{ }

QVariant MaterialItem::data(int role) const
{

	QVariant ret;

	switch(role)
	{
		case Qt::DisplayRole:
			ret = QString(material->GetMaterialName().c_str());
			break;
		default:
			ret = QStandardItem::data(role);
			break;
	}

    return ret;
}

DAVA::NMaterial * MaterialItem::GetMaterial() const
{
	return material;
}

void MaterialItem::SetFlag(MaterialFlag flag, bool set)
{
	if((set && !(curFlag & flag)) || (!set && (curFlag & flag)))
	{
		bool ok = true;

		switch(flag)
		{
			case IS_MARK_FOR_DELETE:
				set ? setBackground(QBrush(QColor(255, 0, 0, 15))) : setBackground(QBrush());
				break;

			case IS_PART_OF_SELECTION:
				if(set)
				{
					QFont curFont = font();
					curFont.setBold(true);
					setFont(curFont); 
				}
				else
				{
					setFont(QFont());
				}
				break;

			default:
				ok = false;
				break;
		}

		if(ok)
		{
			if(set)
			{
				curFlag |= (int) flag;
			}
			else
			{
				curFlag &= ~ (int) flag;
			}
		}
	}
}

bool MaterialItem::GetFlag(MaterialFlag flag) const
{
	return (bool) (curFlag & flag);
}
