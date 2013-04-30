#include "LandscapeTool.h"
#include "Qt/Main/QtUtils.h"

LandscapeTool::LandscapeTool(int32 _ID, eToolType _type, const FilePath & _imageName)
{
    image = NULL;
    
    toolID = _ID;
    type = _type;
    
    imageName = _imageName;
    image = CreateTopLevelImage(imageName);
    
    RenderManager::Instance()->LockNonMain();
    
    float32 sideSize = (float32)image->width;
    sprite = Sprite::CreateAsRenderTarget(sideSize, sideSize, FORMAT_RGBA8888);
    
    Texture *srcTex = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(),
                                              image->GetWidth(), image->GetHeight(), false);
    
    Sprite *srcSprite = Sprite::CreateFromTexture(srcTex, 0, 0, (float32)image->GetWidth(), (float32)image->GetHeight());
    
    RenderManager::Instance()->SetRenderTarget(sprite);
    
    RenderManager::Instance()->SetColor(Color::Black());
    RenderHelper::Instance()->FillRect(Rect(0, 0, sideSize, sideSize));
    
    RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
    RenderManager::Instance()->SetColor(Color::White());
    
    srcSprite->SetScaleSize(sideSize, sideSize);
    srcSprite->SetPosition(Vector2(0, 0));
    srcSprite->Draw();
    RenderManager::Instance()->RestoreRenderTarget();
    
    
    SafeRelease(srcSprite);
    SafeRelease(srcTex);
    
    RenderManager::Instance()->UnlockNonMain();
    
    maxSize = 0.f;
    size = 0.f;
    maxStrength = 0.f;
    strength = 0.f;
    height = 0.0f;
    averageStrength = 0.f;

    
    relativeDrawing = true;
    averageDrawing = false;
    absoluteDropperDrawing = false;
    
    copyHeightmap = false;
    copyTilemask = false;
}

LandscapeTool::~LandscapeTool()
{
    SafeRelease(image);
    SafeRelease(sprite);
}

float32 LandscapeTool::SizeColorMin()
{
    return 0.2f;
}

float32 LandscapeTool::SizeColorMax()
{
    return 2.0f;
}

float32 LandscapeTool::StrengthColorMin()
{
    return 0.0f;
}

float32 LandscapeTool::StrengthColorMax()
{
    return 0.50f;
}

float32 LandscapeTool::StrengthHeightMax()
{
    return 30;
}

float32 LandscapeTool::SizeHeightMax()
{
    return 60.0f;
}

float32 LandscapeTool::DefaultStrengthHeight()
{
    return 15.f;
}

float32 LandscapeTool::DefaultSizeHeight()
{
    return SizeHeightMax() / 2.0f;
}
