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



#include "ScreenControl.h"
#include <QString>
#include "HierarchyTreeNode.h"

#include "Render/RenderHelper.h"
#include "Render/RenderManager.h"
#include "Grid/GridVisualizer.h"

#include "DefaultScreen.h"

ScreenControl::ScreenControl()
{
    background = new UIControl();
    Sprite* backgroundSprite = Sprite::Create("~res:/Gfx/chequered");
    
    UIControlBackground* bkgControl = background->GetBackground();
    bkgControl->SetSprite(backgroundSprite, 0);
    bkgControl->SetDrawType(UIControlBackground::DRAW_TILED);

    SafeRelease(backgroundSprite);
}

ScreenControl::~ScreenControl()
{
	SafeRelease(background);
}

void ScreenControl::SystemDraw(const UIGeometricData &geometricData)
{
	Rect rect = this->GetRect();
    
    // Draw "transparent" (cheqered) backgound under the control.
    RenderManager::Instance()->PushDrawMatrix();
    RenderManager::Instance()->IdentityDrawMatrix();
  	RenderManager::Instance()->SetDrawScale(Vector2(1.0f, 1.0f));
   
    Vector2 backgroundPos = Vector2(0.0f, 0.0f);
    Vector2 backgroundSize = rect.GetSize();
    
    if (pos.x > 0)
    {
        // The size of the background is less than the size of the screen.
        backgroundPos.x = pos.x * scale.x;
        backgroundSize.x *= scale.x;
    }
    else
    {
        // The size of background is the same as the size of screen.
        backgroundSize.x = DAVA::Core::Instance()->GetVirtualScreenWidth();
    }

    // The same logic for Y coord.
    if (pos.y > 0)
    {
        backgroundPos.y = pos.y * scale.y;
        backgroundSize.y *= scale.y;
    }
    else
    {
        backgroundSize.y = DAVA::Core::Instance()->GetVirtualScreenHeight();
    }

    background->SetPosition(backgroundPos);
    background->SetSize(backgroundSize);
    background->SystemDraw(geometricData);

   	RenderManager::Instance()->PopDrawMatrix();
    
    // Draw the control itself.
	UIControl::SystemDraw(geometricData);
    
    // Draw the grid over the control.
	GridVisualizer::Instance()->DrawGridIfNeeded(rect);
}

bool ScreenControl::IsPointInside(const Vector2& /*point*/, bool/* expandWithFocus*/)
{
	//YZ:
	//input must be handled by screen
	return false;
}

void ScreenControl::SetScale(const Vector2& value)
{
    this->scale = value;
}

void ScreenControl::SetPos(const Vector2& value)
{
    this->pos = value;
}

