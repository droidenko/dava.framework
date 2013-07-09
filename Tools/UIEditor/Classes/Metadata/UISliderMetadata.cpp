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

#include "UISliderMetadata.h"
#include "StringUtils.h"
#include "StringConstants.h"

using namespace DAVA;

UISliderMetadata::UISliderMetadata(QObject* parent) :
    UIControlMetadata(parent)
{
}

UISlider* UISliderMetadata::GetActiveUISlider() const
{
    return dynamic_cast<UISlider*>(GetActiveUIControl());
}

// Initialize the control(s) attached.
void UISliderMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
    BaseMetadata::InitializeControl(controlName, position);

    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
		// Initialize UISlider with default parameters
        UISlider* slider = dynamic_cast<UISlider*>(this->treeNodeParams[i].GetUIControl());
		if (slider)
		{
			slider->SetMinMaxValue(0.0f, 100.0f);
		}
    }
}

float UISliderMetadata::GetSliderValue() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
	return GetActiveUISlider()->GetValue();
}

void UISliderMetadata::SetSliderValue(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	GetActiveUISlider()->SetValue(value);
}

float UISliderMetadata::GetSliderMinValue() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }

	return GetActiveUISlider()->GetMinValue();
}

void UISliderMetadata::SetSliderMinValue(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	GetActiveUISlider()->SetMinValue(value);
}

float UISliderMetadata::GetSliderMaxValue() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
	return GetActiveUISlider()->GetMaxValue();
}

void UISliderMetadata::SetSliderMaxValue(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	GetActiveUISlider()->SetMaxValue(value);
}

QString UISliderMetadata::GetSliderThumbSprite() const
{
    if (!VerifyActiveParamID())
    {
        return QString();
    }

	UIControl* thumbButton = GetActiveUISlider()->GetThumb();		
	if (thumbButton == NULL)
	{
		return StringConstants::NO_SPRITE_IS_SET;
	}

    Sprite* thumbSprite = thumbButton->GetBackground()->GetSprite();
    if (thumbSprite == NULL)
    {
        return StringConstants::NO_SPRITE_IS_SET;
    }
    
    return QString::fromStdString(thumbSprite->GetRelativePathname().GetAbsolutePathname());
}

void UISliderMetadata::SetSliderThumbSprite(QString value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    //If empty string value is used - remove sprite
    if (value.isEmpty())
    {
        GetActiveUISlider()->SetThumbSprite(NULL, 0); 
    }
    else
    {
        Sprite* thumbSprite = Sprite::Create(TruncateTxtFileExtension(value).toStdString());
        if (thumbSprite)
        {
            GetActiveUISlider()->SetThumbSprite(thumbSprite, 0);
            SafeRelease(thumbSprite);
        }
    }
}

int UISliderMetadata::GetSliderThumbSpriteFrame() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }
    
   	UIControl* thumbButton = GetActiveUISlider()->GetThumb();		
	if (thumbButton == NULL)
	{
		return 0;
	}
    
    return thumbButton->GetBackground()->GetFrame();
}

void UISliderMetadata::SetSliderThumbSpriteFrame(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	UIControl* thumbButton = GetActiveUISlider()->GetThumb();
	if (thumbButton == NULL)
	{
		return;
	}	
	
	Sprite* thumbSprite = thumbButton->GetBackground()->GetSprite();
	if (thumbSprite && thumbSprite->GetFrameCount() > value)
	{
		GetActiveUISlider()->SetThumbSprite(thumbSprite, value);
	}
}

QString UISliderMetadata::GetSliderMinSprite() const
{
    if (!VerifyActiveParamID())
    {
        return QString();
    }
	
	UIControlBackground* bgMin = GetActiveUISlider()->GetBgMin();
	if (bgMin == NULL)
	{
		return StringConstants::NO_SPRITE_IS_SET;
	}

    Sprite* minSprite = bgMin->GetSprite();
    if (minSprite == NULL)
    {
        return StringConstants::NO_SPRITE_IS_SET;
    }
    
    return QString::fromStdString(minSprite->GetRelativePathname().GetAbsolutePathname());
}

void UISliderMetadata::SetSliderMinSprite(QString value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    //If empty string value is used - remove sprite
    if (value.isEmpty())
    {
        GetActiveUISlider()->SetMinSprite(NULL, 0);
    }
    else
    {
        Sprite* sprite = Sprite::Create(TruncateTxtFileExtension(value).toStdString());
        if (sprite)
        {
            GetActiveUISlider()->SetMinSprite(sprite, 0);
            SafeRelease(sprite);
        }
    }
}

int UISliderMetadata::GetSliderMinSpriteFrame() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }
	
	UIControlBackground* bgMin = GetActiveUISlider()->GetBgMin();
	if (bgMin == NULL)
	{
		return 0;
	}

    return bgMin->GetFrame();
}

void UISliderMetadata::SetSliderMinSpriteFrame(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	UIControlBackground* bgMin = GetActiveUISlider()->GetBgMin();
	if (bgMin == NULL)
	{
		return;
	}
		
	Sprite* minSprite = bgMin->GetSprite();
	if (minSprite && minSprite->GetFrameCount() > value)
	{
		GetActiveUISlider()->SetMinSprite(minSprite, value);
	}
}

int UISliderMetadata::GetSliderMinDrawType() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }
	
	UIControlBackground* bgMin = GetActiveUISlider()->GetBgMin();
	if (bgMin == NULL)
	{
		return 0;
	}
	return bgMin->GetDrawType();
}

void UISliderMetadata::SetSliderMinDrawType(int value)
{
	GetActiveUISlider()->SetMinDrawType((UIControlBackground::eDrawType)value);
}

QString UISliderMetadata::GetSliderMaxSprite() const
{
    if (!VerifyActiveParamID())
    {
        return QString();
    }
	
	UIControlBackground* bgMax = GetActiveUISlider()->GetBgMax();
	if (bgMax == NULL)
	{
		return StringConstants::NO_SPRITE_IS_SET;
	}

    Sprite* maxSprite = bgMax->GetSprite();
    if (maxSprite == NULL)
    {
        return StringConstants::NO_SPRITE_IS_SET;
    }
    
    return QString::fromStdString(maxSprite->GetRelativePathname().GetAbsolutePathname());
}

void UISliderMetadata::SetSliderMaxSprite(QString value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    //If empty string value is used - remove sprite
    if (value.isEmpty())
    {
        GetActiveUISlider()->SetMaxSprite(NULL, 0); 
    }
    else
    {
        Sprite* maxSprite = Sprite::Create(TruncateTxtFileExtension(value).toStdString());
        if (maxSprite)
        {
            GetActiveUISlider()->SetMaxSprite(maxSprite, 0);
            SafeRelease(maxSprite);
        }
    }
}

int UISliderMetadata::GetSliderMaxSpriteFrame() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }
	
	UIControlBackground* bgMax = GetActiveUISlider()->GetBgMax();
	if (bgMax == NULL)
	{
		return 0;
	}

    return bgMax->GetFrame();
}

void UISliderMetadata::SetSliderMaxSpriteFrame(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	UIControlBackground* bgMax = GetActiveUISlider()->GetBgMax();
	if (bgMax == NULL)
	{
		return;
	}
	
	Sprite* maxSprite = bgMax->GetSprite();
	if (maxSprite && maxSprite->GetFrameCount() > value)
	{
		GetActiveUISlider()->SetMaxSprite(maxSprite, value);
	}
}

int UISliderMetadata::GetSliderMaxDrawType() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }
	
	UIControlBackground* bgMax = GetActiveUISlider()->GetBgMax();
	if (bgMax == NULL)
	{
		return 0;
	}
	return bgMax->GetDrawType();
}

void UISliderMetadata::SetSliderMaxDrawType(int value)
{
	GetActiveUISlider()->SetMaxDrawType((UIControlBackground::eDrawType)value);
}

void UISliderMetadata::ApplyResize(const Rect& /*originalRect*/, const Rect& newRect)
{
    if (!VerifyActiveParamID())
    {
        return;
    }	
	
    GetActiveUISlider()->SetRect(newRect);
}
