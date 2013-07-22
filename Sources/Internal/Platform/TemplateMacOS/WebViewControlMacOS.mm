//
//  WebViewControlMacOS.cpp
//  Framework
//
//  Created by Yuri Coder on 2/15/13.
//
//

#include "WebViewControlMacOS.h"
#include "MainWindowController.h"

#import <WebKit/WebKit.h>
#import <AppKit/NSWorkspace.h>

using namespace DAVA;

// A delegate is needed to block the context menu. Note - this delegate
// is informal, so no inheritance from WebUIDelegate needed.
@interface WebViewControlUIDelegate : NSObject
{
}

// Intercept the right-mouse-clicks/
- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems;

@end

@implementation WebViewControlUIDelegate

- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems
{
	// No menu items needed.
	return nil;
}

@end


@interface WebViewPolicyDelegate : NSObject
{
	IUIWebViewDelegate* delegate;
	UIWebView* webView;
}

- (id)init;

- (void)webView:(WebView *)webView decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id<WebPolicyDecisionListener>)listener;

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame;

- (void)setDelegate:(IUIWebViewDelegate*)d andWebView:(UIWebView*)w;

@end

@implementation WebViewPolicyDelegate

- (id)init
{
	self = [super init];
	if (self)
	{
		delegate = NULL;
		webView = NULL;
	}
	return self;
}

- (void)webView:(WebView *)webView decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id<WebPolicyDecisionListener>)listener
{
	BOOL process = YES;
	
	if (delegate && self->webView)
	{
		NSString* url = [[request URL] absoluteString];
		
		if (url)
		{
            NSInteger navigationTypeKey = [[actionInformation objectForKey:@"WebActionNavigationTypeKey"] integerValue];
            bool isRedirecteByMouseClick = navigationTypeKey == WebNavigationTypeLinkClicked;
			IUIWebViewDelegate::eAction action = delegate->URLChanged(self->webView, [url UTF8String], isRedirecteByMouseClick);
			
			switch (action)
			{
				case IUIWebViewDelegate::PROCESS_IN_WEBVIEW:
					Logger::Debug("PROCESS_IN_WEBVIEW");
					break;
					
				case IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER:
					Logger::Debug("PROCESS_IN_SYSTEM_BROWSER");
					process = NO;
					[[NSWorkspace sharedWorkspace] openURL:[request URL]];
					break;
					
				case IUIWebViewDelegate::NO_PROCESS:
					Logger::Debug("NO_PROCESS");
					
				default:
					process = NO;
					break;
			}
		}
	}
	
	if (process)
	{
		[listener use];
	}
	else
	{
		[listener ignore];
	}
}

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
    if (delegate && self->webView)
	{
        delegate->PageLoaded(self->webView);
    }
}

- (void)setDelegate:(DAVA::IUIWebViewDelegate *)d andWebView:(DAVA::UIWebView *)w
{
	if (d && w)
	{
		delegate = d;
		webView = w;
	}
}

@end


WebViewControl::WebViewControl()
{
	NSRect emptyRect = NSMakeRect(0.0f, 0.0f, 0.0f, 0.0f);
	webViewPtr = [[WebView alloc] initWithFrame:emptyRect frameName:nil groupName:nil];

	WebView* localWebView = (WebView*)webViewPtr;
	[localWebView setWantsLayer:YES];
	
	webViewDelegatePtr = [[WebViewControlUIDelegate alloc] init];
	[localWebView setUIDelegate:(WebViewControlUIDelegate*)webViewDelegatePtr];

	webViewPolicyDelegatePtr = [[WebViewPolicyDelegate alloc] init];
	[localWebView setPolicyDelegate:(WebViewPolicyDelegate*)webViewPolicyDelegatePtr];
    
    [localWebView setFrameLoadDelegate:(WebViewPolicyDelegate*)webViewPolicyDelegatePtr];

	NSView* openGLView = (NSView*)Core::Instance()->GetOpenGLView();
	[openGLView addSubview:localWebView];
}

WebViewControl::~WebViewControl()
{
	WebView* innerWebView = (WebView*)webViewPtr;

	[innerWebView setUIDelegate:nil];

	[innerWebView removeFromSuperview];
	[innerWebView close];
	[innerWebView release];
	webViewPtr = nil;

	WebViewPolicyDelegate* w = (WebViewPolicyDelegate*)webViewPolicyDelegatePtr;
	[w release];
	webViewPolicyDelegatePtr = nil;
}

void WebViewControl::SetDelegate(DAVA::IUIWebViewDelegate *delegate, DAVA::UIWebView* webView)
{
	WebViewPolicyDelegate* w = (WebViewPolicyDelegate*)webViewPolicyDelegatePtr;
	[w setDelegate:delegate andWebView:webView];
}

void WebViewControl::Initialize(const Rect& rect)
{
	SetRect(rect);
}

// Open the URL requested.
void WebViewControl::OpenURL(const String& urlToOpen)
{
	NSString* nsURLPathToOpen = [NSString stringWithUTF8String:urlToOpen.c_str()];
	[(WebView*)webViewPtr setMainFrameURL:nsURLPathToOpen];
}

void WebViewControl::SetRect(const Rect& rect)
{
	NSRect webViewRect = [(WebView*)webViewPtr frame];

	webViewRect.size.width = rect.dx * Core::GetVirtualToPhysicalFactor();
	webViewRect.size.height = rect.dy * Core::GetVirtualToPhysicalFactor();
	
	webViewRect.origin.x = rect.x * DAVA::Core::GetVirtualToPhysicalFactor();
	webViewRect.origin.y = (Core::Instance()->GetPhysicalScreenHeight() - rect.y - rect.dy) * DAVA::Core::GetVirtualToPhysicalFactor();
	
	webViewRect.origin.x += Core::Instance()->GetPhysicalDrawOffset().x;
	webViewRect.origin.y += Core::Instance()->GetPhysicalDrawOffset().y;
	
	[(WebView*)webViewPtr setFrame: webViewRect];
}

void WebViewControl::SetVisible(bool isVisible, bool hierarchic)
{
	[(WebView*)webViewPtr setHidden:!isVisible];
}

void WebViewControl::SetBackgroundTransparency(bool enabled)
{
	WebView* webView = (WebView*)webViewPtr;
	[webView setDrawsBackground:(enabled ? NO : YES)];
}
