//
//  WebViewControlMacOS.h
//  Framework
//
//  Created by Yuri Coder on 2/15/13.
//
//

#ifndef __DAVAENGINE_WEBVIEWCONTROL_MACOS_H__
#define __DAVAENGINE_WEBVIEWCONTROL_MACOS_H__

#include "../../UI/IWebViewControl.h"

namespace DAVA {

// Web View Control - MacOS version.
class WebViewControl : public IWebViewControl
{
public:
	WebViewControl();
	virtual ~WebViewControl();
	
	// Initialize the control.
	virtual void Initialize(const Rect& rect);
	
	// Open the URL requested.
	virtual void OpenURL(const String& urlToOpen);

	// Size/pos/visibility changes.
	virtual void SetRect(const Rect& rect);
	virtual void SetVisible(bool isVisible, bool hierarchic);

	virtual void SetDelegate(DAVA::IUIWebViewDelegate *delegate, DAVA::UIWebView* webView);
	virtual void SetBackgroundTransparency(bool enabled);

protected:
	//A pointer to MacOS WebView.
	void* webViewPtr;
	
	// A pointer to the WebView delegate.
	void* webViewDelegatePtr;

	void* webViewPolicyDelegatePtr;
};

};

#endif /* defined(__DAVAENGINE_WEBVIEWCONTROL_MACOS_H__) */
