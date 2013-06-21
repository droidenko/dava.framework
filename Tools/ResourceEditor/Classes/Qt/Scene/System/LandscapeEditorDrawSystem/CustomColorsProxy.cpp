/*==================================================================================
 Copyright (c) 2008, DAVA Consulting, LLC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the DAVA Consulting, LLC nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/

#include "CustomColorsProxy.h"
#include "../SceneEditor/EditorConfig.h"

CustomColorsProxy::CustomColorsProxy(int32 size)
:	changedRect(Rect())
,	spriteChanged(false)
,	size(size)
,	changes(0)
{
	customColorsSprite = Sprite::CreateAsRenderTarget((float32)size, (float32)size, FORMAT_RGBA8888);
	RenderManager::Instance()->SetRenderTarget(customColorsSprite);
	Vector<Color> customColors = EditorConfig::Instance()->GetColorPropertyValues("LandscapeCustomColors");
	if (customColors.size())
	{
		Color color = customColors.front();
		RenderManager::Instance()->ClearWithColor(color.r, color.g, color.b, color.a);
	}
	RenderManager::Instance()->RestoreRenderTarget();
}

CustomColorsProxy::~CustomColorsProxy()
{
	SafeRelease(customColorsSprite);
}

Sprite* CustomColorsProxy::GetSprite()
{
	return customColorsSprite;
}

void CustomColorsProxy::ResetSpriteChanged()
{
	spriteChanged = false;
}

bool CustomColorsProxy::IsSpriteChanged()
{
	return spriteChanged;
}

Rect CustomColorsProxy::GetChangedRect()
{
	if (IsSpriteChanged())
	{
		return changedRect;
	}
	
	return Rect();
}

void CustomColorsProxy::UpdateRect(const DAVA::Rect &rect)
{
	changedRect = rect;
	
	changedRect.x = Max(changedRect.x, 0.f);
	changedRect.y = Max(changedRect.y, 0.f);
	changedRect.dx = Min(changedRect.dx, size - changedRect.x);
	changedRect.dy = Min(changedRect.dy, size - changedRect.y);
	
	spriteChanged = true;
}

int32 CustomColorsProxy::GetChangesCount() const
{
	return changes;
}

void CustomColorsProxy::ResetChanges()
{
	changes = 0;
}

void CustomColorsProxy::IncrementChanges()
{
	++changes;
}

void CustomColorsProxy::DecrementChanges()
{
	--changes;
}