//
//  UIWebView.cpp
//  Framework
//
//  Created by Yuri Coder on 2/15/13.
//
//

#include "UIWebView.h"

#if defined(__DAVAENGINE_MACOS__)
#include "../Platform/TemplateMacOS/WebViewControlMacOS.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "../Platform/TemplateIOS/WebViewControliOS.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "../Platform/TemplateWin32/WebViewControlWin32.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "../Platform/TemplateAndroid/WebViewControl.h"
#else
#pragma error UIWEbView control is not implemented for this platform yet!
#endif

using namespace DAVA;

UIWebView::UIWebView()
{
	webViewControl = new WebViewControl();
}

UIWebView::~UIWebView()
{
	SafeDelete(webViewControl);
};

UIWebView::UIWebView(const Rect &rect, bool rectInAbsoluteCoordinates) :
	webViewControl(new WebViewControl()),
	UIControl(rect, rectInAbsoluteCoordinates)
{
	this->webViewControl->Initialize(rect);
}

void UIWebView::SetDelegate(IUIWebViewDelegate* delegate)
{
	webViewControl->SetDelegate(delegate, this);
}

void UIWebView::OpenURL(const String& urlToOpen)
{
	this->webViewControl->OpenURL(urlToOpen);
}

void UIWebView::SetPosition(const Vector2 &position, bool positionInAbsoluteCoordinates)
{
	UIControl::SetPosition(position, positionInAbsoluteCoordinates);

	// Update the inner control.
	Rect newRect = GetRect();
	this->webViewControl->SetRect(newRect);
}

void UIWebView::SetSize(const Vector2 &newSize)
{
	UIControl::SetSize(newSize);

	// Update the inner control.
	Rect newRect = GetRect();
	this->webViewControl->SetRect(newRect);
}

void UIWebView::SetVisible(bool isVisible, bool hierarchic)
{
	UIControl::SetVisible(isVisible, hierarchic);
	this->webViewControl->SetVisible(isVisible, hierarchic);
}

void UIWebView::SetBackgroundTransparency(bool enabled)
{
	this->webViewControl->SetBackgroundTransparency(enabled);
}
