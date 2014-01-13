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
#include "Base/BaseTypes.h"
#include "Render/Cursor.h"
#include "FileSystem/FileSystem.h"

#if defined(__DAVAENGINE_MACOS__) 

#if defined(__DAVAENGINE_NPAPI__)
#include "NPAPIOpenGLLayerMacOS.h"
#endif

#if defined(Q_OS_MAC)
#include "Platform/Qt/MacOS/CorePlatformMacOS.h"
#else //#if defined(Q_OS_MAC)
#include "Platform/TemplateMacOS/CorePlatformMacOS.h"
#endif //#if defined(Q_OS_MAC)
#include <Cocoa/Cocoa.h>

namespace DAVA
{
Cursor * Cursor::Create(const FilePath & cursorPathname, const Vector2 & hotSpot)
{
	const String realPath = cursorPathname.GetAbsolutePathname();
	NSImage * image = [[NSImage alloc] initByReferencingFile:[NSString stringWithFormat:@"%s", realPath.c_str()]];
	if (!image)
	{
		Logger::Error("Can't open cursor image");
		return 0;
	}
	
	/*if ((image.size.width != 32) && (image.size.height != 32))
	{
		Logger::Error("You are trying to create cursor image with size different from 32x32 pixels");
		return 0;
	}*/
	
	NSCursor * macOSXCursor = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(hotSpot.x, hotSpot.y)];
	[macOSXCursor setOnMouseEntered: YES];
	
	[image release];
	image = 0;
	
	if (macOSXCursor)
	{
		Cursor * cursor = new Cursor();
		cursor->macOSXCursor = macOSXCursor;
		return cursor;
	}
	return 0;
}

Cursor::Cursor()
{
    
}

Cursor::~Cursor()
{
	NSCursor * macOSXCursorX = (NSCursor *) this->macOSXCursor;
	[macOSXCursorX release];
	this->macOSXCursor = 0;
}

void * Cursor::GetMacOSXCursor()
{
	return macOSXCursor;
}

void Cursor::HardwareSet()
{
	// Do nothing here in MacOS version. Everything is in OpenGLView 
}
    
DAVA::Vector2 Cursor::GetPosition()
{
    return static_cast<CoreMacOSPlatform *>(CoreMacOSPlatform::Instance())->GetMousePosition();
}
    
void Cursor::MoveToCenterOfWindow()
{
#ifdef __DAVAENGINE_NPAPI__
	// Just position to the center of the main screen.
    NSScreen *mainScreen = [NSScreen mainScreen];
	NSRect wndRect = [mainScreen visibleFrame];
#else
    NSRect wndRect =[[[NSApplication sharedApplication] mainWindow] frame];
#endif

    CGPoint center = {wndRect.origin.x + wndRect.size.width / 2, wndRect.origin.y + wndRect.size.height / 2};
    CGWarpMouseCursorPosition(center);
}
    
void Cursor::ShowSystemCursor(bool show)
{
#ifdef __DAVAENGINE_NPAPI__
	CGDirectDisplayID displayID = 0; //this parameter is ignored on MacOS.
	if (show)
	{
		CGDisplayShowCursor(displayID);
	}
	else
	{
		CGDisplayHideCursor(displayID);
	}
#else
    if(show)
        [NSCursor unhide];
    else
        [NSCursor hide];
#endif
}
    
void Cursor::Show(bool _show)
{
    show = _show;
    
    ShowSystemCursor(show);
}
    
bool Cursor::IsShow()
{
    return show;
}
    
/*void Cursor::MacOSX_Set()
{
	NSCursor * macOSXCursorX = (NSCursor *)this->macOSXCursor;
	[macOSXCursorX set];
}*/
	
};


#endif // __DAVAENGINE_MACOS__
