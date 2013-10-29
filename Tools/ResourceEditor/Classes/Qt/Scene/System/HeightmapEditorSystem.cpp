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



#include "HeightmapEditorSystem.h"
#include "CollisionSystem.h"
#include "SelectionSystem.h"
#include "ModifSystem.h"
#include "LandscapeEditorDrawSystem.h"
#include "../SceneEditor2.h"
#include "../SceneSignals.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "../../../Commands2/HeightmapEditorCommands2.h"
#include "../../../Commands2/TilemaskEditorCommands.h"
#include "../../Main/QtUtils.h"
#include "../../../SceneEditor/EditorSettings.h"
#include "HoodSystem.h"

HeightmapEditorSystem::HeightmapEditorSystem(Scene* scene)
:	SceneSystem(scene)
,	enabled(false)
,	editingIsEnabled(false)
,	curToolSize(0)
,	cursorSize(30)
,	originalHeightmap(NULL)
,	toolImage(NULL)
,	strength(15)
,	averageStrength(0.5f)
,	inverseDrawingEnabled(false)
,	toolImagePath("")
,	drawingType(HEIGHTMAP_DRAW_ABSOLUTE)
,	copyPasteFrom(-1.f, -1.f)
,	copyPasteTo(-1.f, -1.f)
,	prevCursorPosition(-1.f, -1.f)
,	tilemaskImage(NULL)
,	tilemaskCopyPasteTool(NULL)
,   squareTexture(NULL)
,	toolImageIndex(0)
,	copyPasteHeightmap(false)
,	copyPasteTilemask(false)
,	curHeight(0.f)
,	activeDrawingType(drawingType)
,	activeCopyPasteHeightmap(copyPasteHeightmap)
,	activeCopyPasteTilemask(copyPasteTilemask)
{
	cursorTexture = Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/cursor.tex");
	cursorTexture->SetWrapMode(Texture::WRAP_CLAMP_TO_EDGE, Texture::WRAP_CLAMP_TO_EDGE);

	collisionSystem = ((SceneEditor2 *) GetScene())->collisionSystem;
	selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
	modifSystem = ((SceneEditor2 *) GetScene())->modifSystem;
	drawSystem = ((SceneEditor2 *) GetScene())->landscapeEditorDrawSystem;
	hoodSystem = ((SceneEditor2 *) GetScene())->hoodSystem;
}

HeightmapEditorSystem::~HeightmapEditorSystem()
{
	SafeRelease(cursorTexture);
	SafeRelease(squareTexture);
}

bool HeightmapEditorSystem::IsLandscapeEditingEnabled() const
{
	return enabled;
}

bool HeightmapEditorSystem::IsCanBeEnabled()
{
	return drawSystem->VerifyLandscape();
}

bool HeightmapEditorSystem::EnableLandscapeEditing()
{
	if (enabled)
	{
		return true;
	}

	if (!IsCanBeEnabled())
	{
		return false;
	}

	if (!drawSystem->EnableCustomDraw())
	{
		return false;
	}

	selectionSystem->SetLocked(true);
	selectionSystem->LockSelection(true);
	modifSystem->SetLocked(true);
	hoodSystem->Show(false);

	landscapeSize = drawSystem->GetHeightmapProxy()->Size();
	copyPasteFrom = Vector2(-1.f, -1.f);

	drawSystem->EnableCursor(landscapeSize);
	drawSystem->SetCursorTexture(cursorTexture);
	drawSystem->SetCursorSize(cursorSize);

	drawSystem->GetLandscapeProxy()->InitTilemaskImageCopy();
	drawSystem->GetLandscapeProxy()->InitTilemaskSprites();

	enabled = true;
	return enabled;
}

bool HeightmapEditorSystem::DisableLandscapeEdititing()
{
	if (!enabled)
	{
		return true;
	}

	FinishEditing();

	selectionSystem->LockSelection(false);
	selectionSystem->SetLocked(false);
	modifSystem->SetLocked(false);

	if(selectionSystem->GetSelectionCount() > 0)
	{
		hoodSystem->Show(true);
	}
	
	drawSystem->DisableCursor();
	drawSystem->DisableCustomDraw();
	
	enabled = false;
	return !enabled;
}

void HeightmapEditorSystem::Update(DAVA::float32 timeElapsed)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}
	
	if (editingIsEnabled && isIntersectsLandscape)
	{
		UpdateToolImage();
		UpdateBrushTool(timeElapsed);
	}
}

void HeightmapEditorSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}
	
	UpdateCursorPosition();
	
	if (event->tid == UIEvent::BUTTON_1)
	{
		Vector3 point;
		
		switch(event->phase)
		{
			case UIEvent::PHASE_BEGAN:
				if (drawingType == HEIGHTMAP_DRAW_ABSOLUTE_DROPPER ||
					drawingType == HEIGHTMAP_DROPPER)
				{
					curHeight = drawSystem->GetHeightAtPoint(cursorPosition);
					
					SceneSignals::Instance()->EmitDropperHeightChanged(dynamic_cast<SceneEditor2*>(GetScene()), curHeight);
				}
				
				if (isIntersectsLandscape)
				{
					if (drawingType == HEIGHTMAP_COPY_PASTE)
					{
						if (IsKeyModificatorPressed(DVKEY_ALT))
						{
							copyPasteFrom = cursorPosition;
							copyPasteTo = Vector2(-1.f, -1.f);
							return;
						}
						else
						{
							copyPasteTo = cursorPosition;
							if (copyPasteTilemask)
							{
								PrepareTilemaskCopyPaste();
							}

							if (copyPasteHeightmap)
							{
								StoreOriginalHeightmap();
							}
						}
						activeCopyPasteHeightmap = copyPasteHeightmap;
						activeCopyPasteTilemask = copyPasteTilemask;
					}
					else
					{
						if (drawingType != HEIGHTMAP_DROPPER)
						{
							StoreOriginalHeightmap();
						}
					}

					UpdateToolImage();
					editingIsEnabled = true;
				}

				activeDrawingType = drawingType;
				break;
				
			case UIEvent::PHASE_DRAG:
				break;
				
			case UIEvent::PHASE_ENDED:
				FinishEditing();
				break;
		}
	}
}

void HeightmapEditorSystem::FinishEditing()
{
	if (editingIsEnabled)
	{
		if (activeDrawingType == HEIGHTMAP_COPY_PASTE)
		{
			CreateCopyPasteUndo();
		}
		else if (activeDrawingType != HEIGHTMAP_DROPPER)
		{
			CreateHeightmapUndo();
		}

		editingIsEnabled = false;
	}
}

void HeightmapEditorSystem::UpdateCursorPosition()
{
	Vector3 landPos;
	isIntersectsLandscape = false;
	if (collisionSystem->LandRayTestFromCamera(landPos))
	{
		isIntersectsLandscape = true;
		Vector2 point(landPos.x, landPos.y);
		
		point.x = (float32)((int32)point.x);
		point.y = (float32)((int32)point.y);
		
		AABBox3 box = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();
		
		cursorPosition.x = (point.x - box.min.x) * (landscapeSize - 1) / (box.max.x - box.min.x);
		cursorPosition.y = (point.y - box.min.y) * (landscapeSize - 1) / (box.max.y - box.min.y);
		cursorPosition.x = (int32)cursorPosition.x;
		cursorPosition.y = (int32)cursorPosition.y;

		drawSystem->SetCursorPosition(cursorPosition);
	}
	else
	{
		// hide cursor
		drawSystem->SetCursorPosition(DAVA::Vector2(-100, -100));
	}
}

void HeightmapEditorSystem::UpdateToolImage(bool force)
{
	if (toolImage)
	{
		if (curToolSize != cursorSize || force)
		{
			SafeRelease(toolImage);
		}
	}
	
	if (!toolImage)
	{
		if (!toolImagePath.IsEmpty())
		{
			toolImage = CreateToolImage(cursorSize, toolImagePath);
			curToolSize = cursorSize;
		}
	}
}

Image* HeightmapEditorSystem::CreateToolImage(int32 sideSize, const FilePath& filePath)
{
	RenderManager::Instance()->LockNonMain();
	
	Sprite *dstSprite = Sprite::CreateAsRenderTarget((float32)sideSize, (float32)sideSize, FORMAT_RGBA8888);
	Texture *srcTex = Texture::CreateFromFile(filePath);
	Sprite *srcSprite = Sprite::CreateFromTexture(srcTex, 0, 0, (float32)srcTex->GetWidth(), (float32)srcTex->GetHeight());
	
	RenderManager::Instance()->SetRenderTarget(dstSprite);
	
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
	
	RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
	RenderManager::Instance()->SetColor(Color::White());
	
	srcSprite->SetScaleSize((float32)sideSize, (float32)sideSize);
	srcSprite->SetPosition(Vector2((dstSprite->GetTexture()->GetWidth() - sideSize)/2.0f,
								   (dstSprite->GetTexture()->GetHeight() - sideSize)/2.0f));
	srcSprite->Draw();
	RenderManager::Instance()->RestoreRenderTarget();
	
	Image *retImage = dstSprite->GetTexture()->CreateImageFromMemory();
	
	SafeRelease(srcSprite);
	SafeRelease(srcTex);
	SafeRelease(dstSprite);
	
	RenderManager::Instance()->UnlockNonMain();
	
	return retImage;
}

void HeightmapEditorSystem::UpdateBrushTool(float32 timeElapsed)
{
	if (!toolImage)
	{
		DAVA::Logger::Error("Tool image is empty!");
		return;
	}
	
	EditorHeightmap* editorHeightmap = drawSystem->GetHeightmapProxy();
	
	int32 scaleSize = toolImage->GetWidth();
	Vector2 pos = cursorPosition - Vector2((float32)scaleSize, (float32)scaleSize) / 2.0f;
	{
		switch (activeDrawingType)
		{
			case HEIGHTMAP_DRAW_ABSOLUTE:
			{
				float32 koef = (strength * timeElapsed);
				if(inverseDrawingEnabled)
				{
					koef = -koef;
				}

				if (IsKeyModificatorPressed(DVKEY_ALT))
				{
					koef = -koef;
				}

				editorHeightmap->DrawRelativeRGBA(toolImage, (int32)pos.x, (int32)pos.y, scaleSize, scaleSize, koef);
				break;
			}
				
			case HEIGHTMAP_DRAW_AVERAGE:
			{
				float32 koef = (averageStrength * timeElapsed) * 2.0f;
				editorHeightmap->DrawAverageRGBA(toolImage, (int32)pos.x, (int32)pos.y, scaleSize, scaleSize, koef);
				break;
			}

			case HEIGHTMAP_DRAW_RELATIVE:
			case HEIGHTMAP_DRAW_ABSOLUTE_DROPPER:
			{
				float32 maxHeight = drawSystem->GetLandscapeMaxHeight();
				float32 height = curHeight / maxHeight * Heightmap::MAX_VALUE;
				
				float32 koef = (averageStrength * timeElapsed) * 2.0f;
				editorHeightmap->DrawAbsoluteRGBA(toolImage, (int32)pos.x, (int32)pos.y, scaleSize, scaleSize, koef, height);
				break;
			}

			case HEIGHTMAP_DROPPER:
			{
				float32 curHeight = drawSystem->GetHeightAtPoint(cursorPosition);
				SceneSignals::Instance()->EmitDropperHeightChanged(dynamic_cast<SceneEditor2*>(GetScene()), curHeight);
				return;
			}

			case HEIGHTMAP_COPY_PASTE:
			{
				if (copyPasteFrom == Vector2(-1.f, -1.f) || copyPasteTo == Vector2(-1.f, -1.f))
				{
					return;
				}

				if (activeCopyPasteHeightmap)
				{
					Vector2 posTo = pos;
					
					Vector2 deltaPos = cursorPosition - copyPasteTo;
					Vector2 posFrom = copyPasteFrom + deltaPos - Vector2((float32)scaleSize, (float32)scaleSize)/2.f;
					
					float32 koef = (averageStrength * timeElapsed) * 2.0f;

					editorHeightmap->DrawCopypasteRGBA(toolImage, posFrom, posTo, scaleSize, scaleSize, koef);
				}

				if (activeCopyPasteTilemask && prevCursorPosition != cursorPosition)
				{
					int32 tilemaskSize = drawSystem->GetTextureSize();
					int32 heightmapSize = drawSystem->GetHeightmapProxy()->Size();
					float32 multiplier = (float32)tilemaskSize / (float32)heightmapSize;
					int32 tilemaskCursorSize = (float32)((int32)(cursorSize * multiplier));

					Vector2 posTo = cursorPosition * multiplier - Vector2((float32)tilemaskCursorSize,
																		  (float32)tilemaskCursorSize) / 2.f;
					posTo.x = (float32)((int32)posTo.x);
					posTo.y = (float32)((int32)posTo.y);

					Vector2 deltaPos = cursorPosition - copyPasteTo;
					Vector2 posFrom = (copyPasteFrom + deltaPos) * multiplier - Vector2((float32)tilemaskCursorSize,
																						(float32)tilemaskCursorSize) / 2.f;
					posFrom.x = (float32)((int32)posFrom.x);
					posFrom.y = (float32)((int32)posFrom.y);

					LandscapeProxy* landscapeProxy = drawSystem->GetLandscapeProxy();
					Texture* texture = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK);

					EditorHeightmap::DrawCopypasteRGBA(tilemaskImage, tilemaskImage, tilemaskCopyPasteTool,
													   posFrom, posTo, tilemaskCursorSize, tilemaskCursorSize);
					if(texture)
					{
						texture->TexImage(0, tilemaskImage->GetWidth(), tilemaskImage->GetHeight(),
										  tilemaskImage->GetData(), 0, Texture::CUBE_FACE_INVALID);
						//TODO: is code useful?
						texture->GenerateMipmaps();
						texture->SetWrapMode(Texture::WRAP_REPEAT, Texture::WRAP_REPEAT);
						//ENDOFTODO

						landscapeProxy->UpdateFullTiledTexture();
					}

					prevCursorPosition = cursorPosition;
					Rect r = Rect(posTo, Vector2(tilemaskCursorSize, tilemaskCursorSize));
					AddRectToAccumulator(tilemaskUpdatedRect, r);
				}

				break;
			}
				
			default:
				DAVA::Logger::Error("Invalid drawing type!");
				return;
		}
		
		Rect rect(pos.x, pos.y, (float32)scaleSize, (float32)scaleSize);
		drawSystem->GetHeightmapProxy()->UpdateRect(rect);
		AddRectToAccumulator(heightmapUpdatedRect, rect);
	}
}

void HeightmapEditorSystem::ResetAccumulatorRect(Rect& accumulator)
{
	float32 inf = std::numeric_limits<float32>::infinity();
	accumulator = Rect(inf, inf, -inf, -inf);
}

void HeightmapEditorSystem::AddRectToAccumulator(Rect& accumulator, const Rect& rect)
{
	accumulator = accumulator.Combine(rect);
}

Rect HeightmapEditorSystem::GetHeightmapUpdatedRect()
{
	Rect r = heightmapUpdatedRect;
	drawSystem->ClampToHeightmap(r);
	return r;
}

Rect HeightmapEditorSystem::GetTilemaskUpdatedRect()
{
	Rect r = tilemaskUpdatedRect;
	drawSystem->ClampToTexture(r);
	return r;
}

void HeightmapEditorSystem::StoreOriginalHeightmap()
{
	EditorHeightmap* editorHeightmap = drawSystem->GetHeightmapProxy();
	
	DVASSERT(originalHeightmap == NULL);
	originalHeightmap = editorHeightmap->Clone(NULL);
	ResetAccumulatorRect(heightmapUpdatedRect);
}

void HeightmapEditorSystem::PrepareTilemaskCopyPaste()
{
	tilemaskImage = CreateTilemaskImage();
	CreateTilemaskCopyPasteTool();
	ResetAccumulatorRect(tilemaskUpdatedRect);
}

void HeightmapEditorSystem::CreateHeightmapUndo()
{
	SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
	DVASSERT(scene);
	scene->Exec(new ModifyHeightmapCommand(drawSystem->GetHeightmapProxy(),
										   originalHeightmap,
										   GetHeightmapUpdatedRect()));

	SafeRelease(originalHeightmap);
}

void HeightmapEditorSystem::CreateCopyPasteUndo()
{
	if (!activeCopyPasteHeightmap && !activeCopyPasteTilemask)
	{
		return;
	}

	SceneEditor2* scene = (SceneEditor2*)GetScene();

	scene->BeginBatch("Height Map Copy/Paste");
	if (activeCopyPasteHeightmap)
	{
		Rect heightmapRect = GetHeightmapUpdatedRect();
		ModifyHeightmapCommand* cmd = new ModifyHeightmapCommand(drawSystem->GetHeightmapProxy(), originalHeightmap, heightmapRect);
		scene->Exec(cmd);
		SafeRelease(originalHeightmap);
	}

	if (activeCopyPasteTilemask)
	{
		Rect tilemaskRect = GetTilemaskUpdatedRect();
		ModifyTilemaskCommand* cmd = new ModifyTilemaskCommand(drawSystem->GetLandscapeProxy(), tilemaskRect);
		scene->Exec(cmd);
	}
	scene->EndBatch();
}

void HeightmapEditorSystem::SetBrushSize(int32 brushSize)
{
	if (brushSize > 0)
	{
		cursorSize = (uint32)brushSize;
		drawSystem->SetCursorSize(cursorSize);
	}
}

void HeightmapEditorSystem::SetStrength(float32 strength)
{
	float32 s = abs(strength);
	this->strength = s;
		
	inverseDrawingEnabled = false;
	if (strength < 0.f)
	{
		inverseDrawingEnabled = true;
	}
}

void HeightmapEditorSystem::SetAverageStrength(float32 averageStrength)
{
	if (averageStrength >= 0)
	{
		this->averageStrength = averageStrength;
	}
}

void HeightmapEditorSystem::SetToolImage(const FilePath& toolImagePath, int32 index)
{
	this->toolImagePath = toolImagePath;
	this->toolImageIndex = index;
	UpdateToolImage(true);
}

void HeightmapEditorSystem::SetDrawingType(eHeightmapDrawType type)
{
	copyPasteFrom = Vector2(-1.f, -1.f);
	drawingType = type;
}

void HeightmapEditorSystem::SetCopyPasteHeightmap(bool active)
{
	copyPasteHeightmap = active;
}

void HeightmapEditorSystem::SetCopyPasteTilemask(bool active)
{
	copyPasteTilemask = active;
}

Image* HeightmapEditorSystem::CreateTilemaskImage()
{
	Texture* texture = drawSystem->GetLandscapeProxy()->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK);

	eBlendMode src = RenderManager::Instance()->GetSrcBlend();
	eBlendMode dst = RenderManager::Instance()->GetDestBlend();
	RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
	Image* img = texture->CreateImageFromMemory();
	RenderManager::Instance()->SetBlendMode(src, dst);

	return img;
}

void HeightmapEditorSystem::CreateTilemaskCopyPasteTool()
{
	SafeRelease(tilemaskCopyPasteTool);
	tilemaskCopyPasteTool = CreateToolImage(128, toolImagePath);
}

int32 HeightmapEditorSystem::GetBrushSize()
{
	return cursorSize;
}

float32 HeightmapEditorSystem::GetStrength()
{
	float32 s = strength;
	if (inverseDrawingEnabled)
	{
		s = -s;
	}

	return s;
}

float32 HeightmapEditorSystem::GetAverageStrength()
{
	return averageStrength;
}

int32 HeightmapEditorSystem::GetToolImage()
{
	return toolImageIndex;
}

HeightmapEditorSystem::eHeightmapDrawType HeightmapEditorSystem::GetDrawingType()
{
	return drawingType;
}

bool HeightmapEditorSystem::GetCopyPasteHeightmap()
{
	return copyPasteHeightmap;
}

bool HeightmapEditorSystem::GetCopyPasteTilemask()
{
	return copyPasteTilemask;
}

void HeightmapEditorSystem::SetDropperHeight(float32 height)
{
	float32 maxHeight = drawSystem->GetLandscapeMaxHeight();

	if (height >= 0 && height <= maxHeight)
	{
		curHeight = height;
		SceneSignals::Instance()->EmitDropperHeightChanged(dynamic_cast<SceneEditor2*>(GetScene()), curHeight);
	}
}

float32 HeightmapEditorSystem::GetDropperHeight()
{
	return curHeight;
}
