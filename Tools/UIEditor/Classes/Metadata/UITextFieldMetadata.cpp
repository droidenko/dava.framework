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

#include "UITextFieldMetadata.h"
#include "EditorFontManager.h"

#include "StringUtils.h"

using namespace DAVA;

UITextFieldMetadata::UITextFieldMetadata(QObject* parent) :
    UITextControlMetadata(parent)
{
}

UITextField* UITextFieldMetadata::GetActiveUITextField() const
{
    return dynamic_cast<UITextField*>(GetActiveUIControl());
}

QString UITextFieldMetadata::GetText() const
{
    if (!VerifyActiveParamID())
    {
        return QString();
    }
    return WideString2QStrint(GetActiveUITextField()->GetText());
}


void UITextFieldMetadata::SetText(const QString& text)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUITextField()->SetText(QStrint2WideString(text));
}

Font * UITextFieldMetadata::GetFont()
{
    if (VerifyActiveParamID())
    {
        return GetActiveUITextField()->GetFont();
    }
    return EditorFontManager::Instance()->GetDefaultFont();
}

void UITextFieldMetadata::SetFont(Font * font)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    if (font)
    {
        font->SetSize(GetFontSize());
        GetActiveUITextField()->SetFont(font);
    }
}

float UITextFieldMetadata::GetFontSize() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }    
   
    Font* font = GetActiveUITextField()->GetFont();
    if (!font)
    {
        return 0.0f;
    }
    
    return font->GetSize();
}

void UITextFieldMetadata::SetFontSize(float fontSize)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    Font* font = GetActiveUITextField()->GetFont();
    if (font)
    {
        font->SetSize(fontSize);
        GetActiveUITextField()->SetFont(font);
    }
}

QColor UITextFieldMetadata::GetTextColor() const
{
    if (!VerifyActiveParamID())
    {
        return QColor();
    }
	
	return DAVAColorToQTColor(GetActiveUITextField()->GetTextColor());
}

void UITextFieldMetadata::SetTextColor(const QColor &color)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
    GetActiveUITextField()->SetTextColor(QTColorToDAVAColor(color));
}

QColor UITextFieldMetadata::GetFontColor() const
{
	return GetTextColor();
}

void UITextFieldMetadata::SetFontColor(const QColor& value)
{
	SetTextColor(value);
}

float UITextFieldMetadata::GetShadowOffsetX() const
{
	if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
	
	return GetActiveUITextField()->GetShadowOffset().x;	
}

void UITextFieldMetadata::SetShadowOffsetX(float offset)
{
	if (!VerifyActiveParamID())
    {
        return;
    }
	
	Vector2 shadowOffset = GetOffsetX(GetActiveUITextField()->GetShadowOffset(), offset);
	GetActiveUITextField()->SetShadowOffset(shadowOffset);
}

float UITextFieldMetadata::GetShadowOffsetY() const
{
	if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
	
	return GetActiveUITextField()->GetShadowOffset().y;
}

void UITextFieldMetadata::SetShadowOffsetY(float offset)
{
	if (!VerifyActiveParamID())
    {
        return;
    }
	
	Vector2 shadowOffset = GetOffsetY(GetActiveUITextField()->GetShadowOffset(), offset);
	GetActiveUITextField()->SetShadowOffset(shadowOffset);
}

QColor UITextFieldMetadata::GetShadowColor() const
{
    if (!VerifyActiveParamID())
    {
        return QColor();
    }
	
	return DAVAColorToQTColor(GetActiveUITextField()->GetShadowColor());
}

void UITextFieldMetadata::SetShadowColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUITextField()->SetShadowColor(QTColorToDAVAColor(value));
}

int UITextFieldMetadata::GetTextAlign()
{
	if (!VerifyActiveParamID())
	{
		return ALIGN_HCENTER|ALIGN_VCENTER;
	}
	
	return GetActiveUITextField()->GetTextAlign();
}

void UITextFieldMetadata::SetTextAlign(int32 align)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUITextField()->SetTextAlign(align);
}

// Initialize the control(s) attached.
void UITextFieldMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
    BaseMetadata::InitializeControl(controlName, position);
    
    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        UITextField* textField = dynamic_cast<UITextField*>(this->treeNodeParams[i].GetUIControl());
        
        textField->SetFont(EditorFontManager::Instance()->GetDefaultFont());
        textField->GetBackground()->SetDrawType(UIControlBackground::DRAW_ALIGNED);
        
        // Initialize both control text and localization key.
        WideString controlText = StringToWString(textField->GetName());
        
        HierarchyTreeNode* activeNode = GetTreeNode(i);
        textField->SetText(controlText);
        
        activeNode->GetExtraData().SetLocalizationKey(controlText, this->GetReferenceState());
    }
}
