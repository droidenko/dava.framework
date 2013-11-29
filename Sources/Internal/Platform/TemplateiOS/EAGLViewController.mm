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
#include "Core/Core.h"
#if defined(__DAVAENGINE_IPHONE__)

#import "Platform/TemplateiOS/EAGLViewController.h"

@implementation BackgroundView

- (void)drawRect:(CGRect)rect
{
    // Do nothing to reduce fill usage.
}

@end

@implementation EAGLViewController

@synthesize backgroundView;

/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        // Custom initialization
    }
    return self;
}
*/
- (id) init
{
    if (self = [super init])
    {
        glView = nil;
        backgroundView = nil;
        [self createGLView];
    }
    return self;
}

- (void) createGLView
{
    if (!glView)
    {
       glView = [[EAGLView alloc] initWithFrame:[[::UIScreen mainScreen] bounds]];
    }    
}

// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView 
{
	// To Hottych: Here should be proper initialization code ??? I mean, size of this view for iPhone / iPhone 4 / iPad
	// Check please what should be here
	[self createGLView];

    // Add the background view needed to place native iOS components on it.
    backgroundView = [[BackgroundView alloc] initWithFrame:[glView frame]];
    [backgroundView setBackgroundColor:[UIColor clearColor]];
    [backgroundView setAutoresizingMask:UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight];
    [backgroundView setMultipleTouchEnabled:YES]; // to pass touches to framework, see please DF-2796.

    [glView addSubview:backgroundView];
    [backgroundView release];

    self.view = glView;
//	[glView release];
//   glView = nil;
}

-(UIView*) getBackgroundView
{
    return backgroundView;
}

/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/

// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation 
{
    DAVA::Core::eScreenOrientation orientation = DAVA::Core::Instance()->GetScreenOrientation();
    switch (orientation) {
        case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT:
            return interfaceOrientation == UIInterfaceOrientationLandscapeLeft;
            break;
        case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT:
            return interfaceOrientation == UIInterfaceOrientationLandscapeRight;
            break;
        case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE:
            return (interfaceOrientation == UIInterfaceOrientationLandscapeLeft)||(interfaceOrientation == UIInterfaceOrientationLandscapeRight);
            break;
        case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT:
            return interfaceOrientation == UIInterfaceOrientationPortrait;
            break;
        case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN:
            return interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown;
            break;
        case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_AUTOROTATE:
            return (interfaceOrientation == UIInterfaceOrientationPortrait)||(interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown);
            break;
        default:
            return FALSE;
            break;
    }
    
}

-(BOOL)shouldAutorotate
{
    /*return (DAVA::Core::Instance()->GetScreenOrientation()==DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE)||(DAVA::Core::Instance()->GetScreenOrientation()==DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_AUTOROTATE);*/
    return TRUE;
}
-(NSUInteger)supportedInterfaceOrientations
{
    DAVA::Core::eScreenOrientation orientation = DAVA::Core::Instance()->GetScreenOrientation();
    switch (orientation) {
        case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT:
            return UIInterfaceOrientationMaskLandscapeLeft;
            break;
        case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT:
            return UIInterfaceOrientationMaskLandscapeRight;
            break;
        case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE:
            return UIInterfaceOrientationMaskLandscape;
            break;
        case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT:
            return UIInterfaceOrientationMaskPortrait;
            break;
        case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN:
            return UIInterfaceOrientationMaskPortraitUpsideDown;
            break;
        case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_AUTOROTATE:
            return UIInterfaceOrientationMaskPortrait;
            break;
        default:
            return UIInterfaceOrientationMaskPortrait;
            break;
    }

}



- (void)didReceiveMemoryWarning 
{
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload 
{
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}

- (void)dealloc 
{
    [glView release];
    glView = nil;
    [super dealloc];
}

- (void) viewWillAppear: (BOOL)animating
{
	NSLog(@"EAGLViewController viewWillAppear (startAnimation)");
	[glView setCurrentContext];
	[glView startAnimation];
}

- (void) viewDidDisappear: (BOOL)animating
{
	NSLog(@"EAGLViewController viewDidDisappear (stopAnimation)");
	[glView stopAnimation];
}


@end
#endif // 