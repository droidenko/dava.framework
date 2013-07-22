//
//  UIWebView.h
//  Framework
//
//  Created by Yuri Coder on 2/15/13.
//
//

#ifndef __DAVAENGINE_UIWEBVIEW_H__
#define __DAVAENGINE_UIWEBVIEW_H__

#include "UIControl.h"
#include "IWebViewControl.h"

namespace DAVA {
	
// The purpose of UIWebView class is displaying embedded Web Page Controls.
class UIWebView : public UIControl
{
public:
	UIWebView();
	UIWebView(const Rect &rect, bool rectInAbsoluteCoordinates = false);
	virtual ~UIWebView();
		
	// Open the URL.
	void OpenURL(const String& urlToOpen);

	// Overloaded virtual methods.
	virtual void SetPosition(const Vector2 &position, bool positionInAbsoluteCoordinates = false);
	virtual void SetSize(const Vector2 &newSize);
	virtual void SetVisible(bool isVisible, bool hierarchic = true);

	void SetDelegate(IUIWebViewDelegate* delegate);
	void SetBackgroundTransparency(bool enabled);

protected:

	// Platform-specific implementation of the Web View Control.
	IWebViewControl* webViewControl;
};
};

#endif /* defined(__DAVAENGINE_UIWEBVIEW_H__) */
