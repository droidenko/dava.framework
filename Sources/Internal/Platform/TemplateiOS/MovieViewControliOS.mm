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


#include "MovieViewControliOS.h"

#import <MediaPlayer/MediaPlayer.h>
#import <HelperAppDelegate.h>

namespace DAVA {

//Use unqualified UIWebView and UIScreen from global namespace, i.e. from UIKit
using ::UIScreen;

MovieViewControl::MovieViewControl()
{
	moviePlayerController = [[MPMoviePlayerController alloc] init];

	MPMoviePlayerController* player = (MPMoviePlayerController*)moviePlayerController;
	[player setShouldAutoplay:FALSE];
	[player.view setUserInteractionEnabled:NO];

	if ([player respondsToSelector:@selector(loadState)])
	{
		[player setControlStyle:MPMovieControlStyleNone];
		player.scalingMode = MPMovieScalingModeFill;
	}

	HelperAppDelegate* appDelegate = [[UIApplication sharedApplication] delegate];
	[[appDelegate glController].backgroundView addSubview:player.view];
}

MovieViewControl::~MovieViewControl()
{
	MPMoviePlayerController* player = (MPMoviePlayerController*)moviePlayerController;
	
	[player.view removeFromSuperview];
	[player release];
	moviePlayerController = nil;
}

void MovieViewControl::Initialize(const Rect& rect)
{
	SetRect(rect);
}

void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
	NSURL* movieURL = [NSURL fileURLWithPath:[NSString stringWithCString:moviePath.GetAbsolutePathname().c_str() encoding:NSASCIIStringEncoding]];

	MPMoviePlayerController* player = (MPMoviePlayerController*)moviePlayerController;
	[player setScalingMode: (MPMovieScalingMode)ConvertScalingModeToPlatform(params.scalingMode)];
	[player setContentURL: movieURL];
}

void MovieViewControl::SetRect(const Rect& rect)
{
	MPMoviePlayerController* player = (MPMoviePlayerController*)moviePlayerController;

	CGRect playerViewRect = player.view.frame;
	
    playerViewRect.origin.x = rect.x * DAVA::Core::GetVirtualToPhysicalFactor();
    playerViewRect.origin.y = rect.y * DAVA::Core::GetVirtualToPhysicalFactor();
			
    playerViewRect.size.width = rect.dx * DAVA::Core::GetVirtualToPhysicalFactor();
    playerViewRect.size.height = rect.dy * DAVA::Core::GetVirtualToPhysicalFactor();
			
    playerViewRect.origin.x += Core::Instance()->GetPhysicalDrawOffset().x;
    playerViewRect.origin.y += Core::Instance()->GetPhysicalDrawOffset().y;
			
		
	
	// Apply the Retina scale divider, if any.
	float scaleDivider = GetScaleDivider();
	playerViewRect.origin.x /= scaleDivider;
	playerViewRect.origin.y /= scaleDivider;
	playerViewRect.size.height /= scaleDivider;
	playerViewRect.size.width /= scaleDivider;
	
	[player.view setFrame: playerViewRect];
}

void MovieViewControl::SetVisible(bool isVisible)
{
	MPMoviePlayerController* player = (MPMoviePlayerController*)moviePlayerController;
	[player.view setHidden:!isVisible];
}

void MovieViewControl::Play()
{
	[(MPMoviePlayerController*)moviePlayerController play];
}

void MovieViewControl::Stop()
{
	[(MPMoviePlayerController*)moviePlayerController stop];
}

void MovieViewControl::Pause()
{
	[(MPMoviePlayerController*)moviePlayerController pause];
}

void MovieViewControl::Resume()
{
	[(MPMoviePlayerController*)moviePlayerController play];
}

bool MovieViewControl::IsPlaying()
{
	MPMoviePlayerController* player = (MPMoviePlayerController*)moviePlayerController;
	return (player.playbackState == MPMoviePlaybackStatePlaying);
}

float MovieViewControl::GetScaleDivider()
{
	float scaleDivider = 1.f;
	if (DAVA::Core::IsAutodetectContentScaleFactor())
	{
		if ([UIScreen instancesRespondToSelector: @selector(scale) ]
			&& [UIView instancesRespondToSelector: @selector(contentScaleFactor) ])
		{
			scaleDivider = [[UIScreen mainScreen] scale];
		}
	}
		
	return scaleDivider;
}

int MovieViewControl::ConvertScalingModeToPlatform(eMovieScalingMode scalingMode)
{
	switch (scalingMode)
	{
		case scalingModeFill:
		{
			return MPMovieScalingModeFill;
		}

		case scalingModeAspectFill:
		{
			return MPMovieScalingModeAspectFill;
		}

		case scalingModeAspectFit:
		{
			return MPMovieScalingModeAspectFit;
		}
		
		case scalingModeNone:
		default:
		{
			return MPMovieScalingModeNone;
		}
	}
}

}