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



#ifndef __DAVAENGINE_IWEBVIEWCONTROL_H__
#define __DAVAENGINE_IWEBVIEWCONTROL_H__

#include "Math/MathConstants.h"
#include "Math/Math2D.h"
#include "Math/Vector.h"
#include "Math/Rect.h"

namespace DAVA {

class UIWebView;
class IUIWebViewDelegate
{
public:
	enum eAction
	{
		PROCESS_IN_WEBVIEW = 0,
		PROCESS_IN_SYSTEM_BROWSER,
		NO_PROCESS,
		ACTIONS_COUNT
	};

	virtual eAction URLChanged(DAVA::UIWebView* webview, const String& newURL, bool isRedirectedByMouseClick) = 0;
	
	virtual void PageLoaded(DAVA::UIWebView* webview) = 0;
};


// Common interface for Web View Controls for different platforms.
class IWebViewControl
{
public:
	virtual ~IWebViewControl() {};
	
	// Initialize the control.
	virtual void Initialize(const Rect& rect) = 0;
	
	// Open the URL requested.
	virtual void OpenURL(const String& urlToOpen) = 0;
	// Load html page from string
	virtual void LoadHtmlString(const WideString& htmlString) = 0;
	// Delete all cookies associated with target URL
	virtual void DeleteApplicationCookies(const String& targetUrl) { };
	
	// Size/pos/visibility changes.
	virtual void SetRect(const Rect& rect) = 0;
	virtual void SetVisible(bool isVisible, bool hierarchic) = 0;
	// Page scale property change
	virtual void SetScalesPageToFit(bool isScalesToFit) { };
	
	virtual void SetDelegate(DAVA::IUIWebViewDelegate *delegate, DAVA::UIWebView* webView) = 0;
	virtual void SetBackgroundTransparency(bool enabled) { };

	// Bounces settings.
	virtual bool GetBounces() const {return false;};
	virtual void SetBounces(bool value) {};
};

};

#endif // __DAVAENGINE_IWEBVIEWCONTROL_H__
