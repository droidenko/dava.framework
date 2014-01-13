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
#include "Utils/Utils.h"

#ifdef __DAVAENGINE_IPHONE__
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#endif

#if defined(__DAVAENGINE_MACOS__)
#import <Foundation/NSThread.h>
#import <Foundation/NSURL.h>
#import <AppKit/AppKit.h>
#endif //#if defined(__DAVAENGINE_MACOS__)

namespace DAVA
{
	
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)

	
	WideString GetDeviceName()
	{
#if defined(__DAVAENGINE_IPHONE__)
		NSString * string = [UIDevice currentDevice].name;
		WideString ws;
		int len = [string length];
		ws.resize(len);
		for (int k = 0; k < len; ++k)
		{
			ws[k] = [string characterAtIndex: k];
		}
		return ws;
#else
		return L"MacOS";
#endif
	}

	bool IsDrawThread()
	{
		return [NSThread isMainThread];
	}

    void OpenURL(const String& url)
    {
        NSString* urlString = [NSString stringWithCString:url.c_str() encoding:NSASCIIStringEncoding];
        NSURL* urlToOpen = [NSURL URLWithString:urlString];
#if defined(__DAVAENGINE_IPHONE__)
        [[UIApplication sharedApplication] openURL:urlToOpen];
#else
        [[NSWorkspace sharedWorkspace] openURL:urlToOpen];
#endif
    }

#endif
	
#if defined(__DAVAENGINE_IPHONE__)
void DisableSleepTimer()
{
	UIApplication * app = [UIApplication sharedApplication];
	app.idleTimerDisabled = YES;
}

void EnableSleepTimer()
{
	UIApplication * app = [UIApplication sharedApplication];
	app.idleTimerDisabled = NO;
}
	
	uint64 EglGetCurrentContext()
	{
		return (uint64)[EAGLContext currentContext];
	}
	
#endif
    
}; // end of namespace DAVA
