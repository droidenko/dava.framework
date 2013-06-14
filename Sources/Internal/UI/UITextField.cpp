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
#include "UI/UITextField.h"
#include "Base/ObjectFactory.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"
#include "Input/KeyboardDevice.h"
#include "UI/UIYamlLoader.h"
#include "UI/UIControlSystem.h"
#include "Render/2D/FontManager.h"
#ifdef __DAVAENGINE_ANDROID__
#include "UITextFieldAndroid.h"
#endif

extern void CreateTextField(DAVA::UITextField *);
extern void ReleaseTextField();
extern void OpenKeyboard();
extern void CloseKeyboard();

namespace DAVA 
{

REGISTER_CLASS(UITextField);

void UITextFieldDelegate::TextFieldShouldReturn(UITextField * /*textField*/)
{
}

void UITextFieldDelegate::TextFieldShouldCancel(UITextField * /*textField*/)
{
};
    
void UITextFieldDelegate::TextFieldLostFocus(UITextField * /*textField*/)
{
};

bool UITextFieldDelegate::TextFieldKeyPressed(UITextField * /*textField*/, int32 /*replacementLocation*/, int32 /*replacementLength*/, const WideString & /*replacementString*/)
{
	return true;
}
    
bool UITextFieldDelegate::IsTextFieldShouldSetFocusedOnAppear(UITextField * /*textField*/)
{
	return false;
}
	
bool UITextFieldDelegate::IsTextFieldCanLostFocus(UITextField * textField)
{
	return true;
}
    
UITextField::UITextField(const Rect &rect, bool rectInAbsoluteCoordinates/*= false*/)
:	UIControl(rect, rectInAbsoluteCoordinates)
,	text()
,	delegate(0)
,	cursorBlinkingTime(0.0f),
        textFont(NULL),
        staticText(NULL)
{
#ifdef __DAVAENGINE_ANDROID__
	textFieldAndroid = new UITextFieldAndroid(this);
#endif

#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone = new UITextFieldiPhone(this);
#else
    staticText = new UIStaticText(Rect(0,0,GetRect().dx, GetRect().dy));
    AddControl(staticText);
    
    staticText->SetSpriteAlign(ALIGN_LEFT | ALIGN_BOTTOM);
#endif
    
    cursorTime = 0;
    showCursor = true;    
}

UITextField::UITextField() : delegate(NULL), cursorBlinkingTime(0.f),
        textFont(NULL), staticText(NULL)
{
#ifdef __DAVAENGINE_ANDROID__
	textFieldAndroid = new UITextFieldAndroid(this);
#endif

#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone = new UITextFieldiPhone(this);
#else
    staticText = new UIStaticText(Rect(0,0,GetRect().dx, GetRect().dy));
    AddControl(staticText);
    
    staticText->SetSpriteAlign(ALIGN_LEFT | ALIGN_BOTTOM);
#endif
    
    cursorTime = 0;
    showCursor = true;
}

//void UITextField::InitAfterYaml()
//{
//#ifdef __DAVAENGINE_IPHONE__
//	textFieldiPhone = new UITextFieldiPhone(this);
//#else
//    
//    staticText = new UIStaticText(Rect(0,0,GetRect().dx, GetRect().dy));
//    staticText->SetFont(textFont);
//    AddControl(staticText);
//
//    staticText->GetBackground()->SetAlign(ALIGN_LEFT | ALIGN_BOTTOM);
//#endif
//
//    cursorTime = 0;
//    showCursor = true;
//}
	
UITextField::~UITextField()
{
    SafeRelease(textFont);
#ifdef __DAVAENGINE_ANDROID__
	SafeDelete(textFieldAndroid);
#endif
#ifdef __DAVAENGINE_IPHONE__
	SafeDelete(textFieldiPhone);
#else
    RemoveAllControls();
    SafeRelease(staticText);
#endif
}

void UITextField::OpenKeyboard()
{
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->OpenKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
	textFieldAndroid->ShowField();
#endif
}

void UITextField::CloseKeyboard()
{
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->CloseKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
	textFieldAndroid->HideField();
#endif
}
	
void UITextField::SetReturnKey(int32 returnType)
{
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetReturnKey(returnType);
#endif
}

void UITextField::Update(float32 timeElapsed)
{
#ifdef __DAVAENGINE_IPHONE__
	Rect rect = GetGeometricData().GetUnrotatedRect();//GetRect(true);
	textFieldiPhone->UpdateRect(rect);
#else

    cursorTime += timeElapsed;

    if (cursorTime >= 0.5f)
    {
        cursorTime = 0;
        showCursor = !showCursor;
        needRedraw = true;
    }

	if(this == UIControlSystem::Instance()->GetFocusedControl())
	{
		if(needRedraw)
		{
			WideString txt = text;

			if (showCursor)
				txt += L"_";
			else
				txt += L" ";

			staticText->SetText(txt);
			needRedraw = false;
		}
	}
	else
	{
		staticText->SetText(text);
	}

#endif
}


void UITextField::WillAppear()
{
    if (delegate && delegate->IsTextFieldShouldSetFocusedOnAppear(this)) 
    {
        UIControlSystem::Instance()->SetFocusedControl(this, false);
    }
}

void UITextField::DidAppear()
{
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->ShowField();
#elif defined(__DAVAENGINE_ANDROID__)
//	textFieldAndroid->ShowField();
#endif
}

void UITextField::WillDisappear()
{
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->HideField();
#elif defined(__DAVAENGINE_ANDROID__)
	textFieldAndroid->HideField();
#endif
}
    
void UITextField::OnFocused()
{
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->OpenKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
	textFieldAndroid->ShowField();
#endif
}
    
void UITextField::OnFocusLost(UIControl *newFocus)
{
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->CloseKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
	textFieldAndroid->HideField();
#endif
    if (delegate) 
    {
        delegate->TextFieldLostFocus(this);
    }
}

bool UITextField::IsLostFocusAllowed(UIControl *newFocus)
{
    if (delegate)
    {
        return delegate->IsTextFieldCanLostFocus(this);
    }
    return true;
}

void UITextField::ReleaseFocus()
{
	if(this == UIControlSystem::Instance()->GetFocusedControl())
	{
		UIControlSystem::Instance()->SetFocusedControl(NULL, true);
	}
}
    
void UITextField::SetFont(Font * font)
{
#ifndef __DAVAENGINE_IPHONE__
    SafeRelease(textFont);
    textFont = SafeRetain(font);
    staticText->SetFont(textFont);
#endif
}

void UITextField::SetTextColor(const Color& fontColor)
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetTextColor(fontColor);
#else
    staticText->SetTextColor(fontColor);
#endif
}

void UITextField::SetFontColor(const Color& fontColor)
{
    SetTextColor(fontColor);
}

void UITextField::SetShadowOffset(const DAVA::Vector2 &offset)
{
	staticText->SetShadowOffset(offset);
}
	
void UITextField::SetShadowColor(const Color& color)
{
	staticText->SetShadowColor(color);
   }

void UITextField::SetTextAlign(int32 align)
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetTextAlign(align);
#else
    staticText->SetTextAlign(align);
#endif
	
}

void UITextField::SetFontSize(float size)
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetFontSize(size);
#endif
}

void UITextField::SetDelegate(UITextFieldDelegate * _delegate)
{
	delegate = _delegate;
}

UITextFieldDelegate * UITextField::GetDelegate()
{
	return delegate;
}
	
void UITextField::SetSpriteAlign(int32 align)
{
    UIControl::SetSpriteAlign(align);
}

void UITextField::SetSize(const DAVA::Vector2 &newSize)
{
    UIControl::SetSize(newSize);
#ifdef __DAVAENGINE_IPHONE__
#else
    staticText->SetSize(newSize);
#endif
}
    
void UITextField::SetText(const WideString & _text)
{
	text = _text;
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetText(text);

#else
    //staticText->SetText(text);
#endif

    needRedraw = true;
}

const WideString & UITextField::GetText()
{
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->GetText(text);
#endif
	
	return text;
}
    
    Font* UITextField::GetFont()
    {
        return textFont;
    }
	
	Color UITextField::GetTextColor()
	{
		return staticText ? staticText->GetTextColor() : Color(1,1,1,1);
	}
	
	Vector2 UITextField::GetShadowOffset()
	{
		return staticText ? staticText->GetShadowOffset() : Vector2(0,0);
	}
	
	Color UITextField::GetShadowColor()
	{
		return staticText ? staticText->GetShadowColor() : Color(1,1,1,1);
	}

	int32 UITextField::GetTextAlign()
	{
#ifdef __DAVAENGINE_IPHONE__
        return textFieldiPhone ? textFieldiPhone->GetTextAlign() : ALIGN_HCENTER|ALIGN_VCENTER;
#else
        return staticText ? staticText->GetTextAlign() : ALIGN_HCENTER|ALIGN_VCENTER;
#endif
		
	}

void UITextField::Input(UIEvent *currentInput)
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    // nothing to do
#else

//	if(currentInput->phase == UIEvent::PHASE_BEGAN)
//	{
//        UIControlSystem::Instance()->SetFocusedControl(this, true);
//	}

	if(this != UIControlSystem::Instance()->GetFocusedControl())
		return;


    if (currentInput->phase == UIEvent::PHASE_KEYCHAR)
    {	
        /// macos
        
        if (currentInput->tid == DVKEY_BACKSPACE)
        {
            //TODO: act the same way on iPhone
            WideString str = L"";
            if(delegate->TextFieldKeyPressed(this, (int32)GetText().length(), -1, str))
			{
                SetText(GetAppliedChanges((int32)GetText().length(),  -1, str));
			}
        }
		else if (currentInput->tid == DVKEY_ENTER)
		{
			delegate->TextFieldShouldReturn(this);
		}
		else if (currentInput->tid == DVKEY_ESCAPE)
		{
			delegate->TextFieldShouldCancel(this);
		}
        else if(currentInput->keyChar != 0)
        {
			WideString str;
			str += currentInput->keyChar;
			if(delegate->TextFieldKeyPressed(this, (int32)GetText().length(), 1, str))
			{
                SetText(GetAppliedChanges((int32)GetText().length(),  1, str));
			}
        }
    }

#endif
}
    
WideString UITextField::GetAppliedChanges(int32 replacementLocation, int32 replacementLength, const WideString & replacementString)
{//TODO: fix this for copy/paste
    WideString txt = GetText();
    if (replacementLength < 0) 
    {
        if (txt.size() > 0)
        {
            txt.erase(txt.end() + replacementLength);
        }
    }
    else 
    {
        txt = GetText() + replacementString;
    }
    return txt;
}
    

void UITextField::RenderText()
{
#ifdef __DAVAENGINE_IPHONE__
    // nothing to do
#else

#if 0
    SafeRelease(textSprite);

    int32 w = GetRect().dx;
    int32 h = GetRect().dy;

    EnsurePowerOf2(w);
    EnsurePowerOf2(h);

    int16 * buf;
	buf = new int16[w * h];
	memset(buf, 0, w * h * sizeof(int16));

    Size2i ds = textFont->DrawString(text, buf, w, h, 0, 0);
    String addInfo = WStringToString(Format(L"Text texture: %S", text.c_str()));
    Texture *tex = Texture::CreateTextFromData(FORMAT_RGBA4444, (uint8*)buf, w, h, addInfo.c_str());
    delete [] buf;

    textSprite = Sprite::CreateFromTexture(tex, 0, 0, GetRect().dx, GetRect().dy);
    SafeRelease(tex);

    textSprite->SetDefaultPivotPoint(0.0f, (float32)(textFont->GetBaseline() - h));
    
    SetSprite(textSprite, 0);

#endif

#endif
}

void UITextField::LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader)
{
	UIControl::LoadFromYamlNode(node, loader);

    YamlNode * textNode = node->Get("text");
	if (textNode)
    {
        SetText(textNode->AsWString());
    }

    YamlNode * fontNode = node->Get("font");
    if (fontNode)
    {
        Font * font = loader->GetFontByName(fontNode->AsString());
        if (font)
            SetFont(font);
    }

    if(staticText)
    {
        staticText->SetRect(Rect(0,0,GetRect().dx, GetRect().dy));
		
		YamlNode * textColorNode = node->Get("textcolor");
		YamlNode * shadowColorNode = node->Get("shadowcolor");
		YamlNode * shadowOffsetNode = node->Get("shadowoffset");
		YamlNode * textAlignNode = node->Get("textalign");
		
		if(textColorNode)
		{
			Vector4 c = textColorNode->AsVector4();
			SetTextColor(Color(c.x, c.y, c.z, c.w));
		}

		if(shadowColorNode)
		{
			Vector4 c = shadowColorNode->AsVector4();
			SetShadowColor(Color(c.x, c.y, c.z, c.w));
		}

		if(shadowOffsetNode)
		{
			SetShadowOffset(shadowOffsetNode->AsVector2());
		}

		if(textAlignNode)
		{
			SetTextAlign(loader->GetAlignFromYamlNode(textAlignNode));
		}
    }
    //InitAfterYaml();

#if 0
	YamlNode * orientNode = node->Get("orientation");
	if (orientNode)
	{
		if (orientNode->AsString() == "Vertical")
			orientation = ORIENTATION_VERTICAL;
		else if (orientNode->AsString() == "Horizontal")
			orientation = ORIENTATION_HORIZONTAL;
	}



		// TODO
	InitAfterYaml();
#endif
}

YamlNode * UITextField::SaveToYamlNode(UIYamlLoader * loader)
{
    YamlNode *node = UIControl::SaveToYamlNode(loader);

    //Temp variable
    VariantType *nodeValue = new VariantType();

    //Control Type
	SetPreferredNodeType(node, "UITextField");

    //Text
    nodeValue->SetWideString(this->GetText());
    node->Set("text", nodeValue);

    //Font
    //Get font name and put it here
    nodeValue->SetString(FontManager::Instance()->GetFontName(this->GetFont()));
    node->Set("font", nodeValue);
	
	//TextColor
	Color textColor = GetTextColor();
	nodeValue->SetVector4(Vector4(textColor.r, textColor.g, textColor.b, textColor.a));
	node->Set("textcolor", nodeValue);

	// ShadowColor
	Color shadowColor = GetShadowColor();
	nodeValue->SetVector4(Vector4(shadowColor.r, shadowColor.g, shadowColor.b, shadowColor.a));
	node->Set("shadowcolor", nodeValue);

	// ShadowOffset
	nodeValue->SetVector2(GetShadowOffset());
	node->Set("shadowoffset", nodeValue);

	// Text align
	node->SetNodeToMap("textalign", loader->GetAlignNodeValue(this->GetTextAlign()));

	// Draw Type must be overwritten fot UITextField.
	UIControlBackground::eDrawType drawType =  this->GetBackground()->GetDrawType();
	node->Set("drawType", loader->GetDrawTypeNodeValue(drawType));

    SafeDelete(nodeValue);
    
    return node;
}

List<UIControl* >& UITextField::GetRealChildren()
{
	List<UIControl* >& realChildren = UIControl::GetRealChildren();
	realChildren.remove(staticText);
	return realChildren;
}

UIControl* UITextField::Clone()
{
	UITextField *t = new UITextField();
	t->CopyDataFrom(this);
	return t;
}
	
void UITextField::CopyDataFrom(UIControl *srcControl)
{
	UIControl::CopyDataFrom(srcControl);
	UITextField* t = (UITextField*) srcControl;
		
	cursorTime = t->cursorTime;
    showCursor = t->showCursor;
	SetText(t->text);
	SetRect(t->GetRect());
	
	cursorBlinkingTime = t->cursorBlinkingTime;
	if (t->staticText)
	{
		staticText = (UIStaticText*)t->staticText->Clone();
		AddControl(staticText);
	}
	if (t->textFont)
		SetFont(t->textFont);
}
	
}; // namespace


