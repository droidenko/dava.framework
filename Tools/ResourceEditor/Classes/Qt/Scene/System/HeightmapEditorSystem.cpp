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
#include "../../Main/QtUtils.h"

const float32 HeightmapEditorSystem::MAX_STRENGTH = 30.f;

HeightmapEditorSystem::HeightmapEditorSystem(Scene* scene)
:	SceneSystem(scene)
,	enabled(false)
,	editingIsEnabled(false)
,	curToolSize(0)
,	cursorSize(30)
,	originalHeightmap(NULL)
,	toolImage(NULL)
,	strength(0)
,	averageStrength(0)
,	inverseDrawingEnabled(false)
,	toolImagePath("")
,	drawingType(HEIGHTMAP_DRAW_RELATIVE)
,	copyPasteFrom(-1.f, -1.f)
,	copyPasteTo(-1.f, -1.f)
{
	cursorTexture = Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/cursor.png");
	cursorTexture->SetWrapMode(Texture::WRAP_CLAMP_TO_EDGE, Texture::WRAP_CLAMP_TO_EDGE);

	squareTexture = Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/squareCursor.png");
	squareTexture->SetWrapMode(Texture::WRAP_CLAMP_TO_EDGE, Texture::WRAP_CLAMP_TO_EDGE);

	collisionSystem = ((SceneEditor2 *) GetScene())->collisionSystem;
	selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
	modifSystem = ((SceneEditor2 *) GetScene())->modifSystem;
	drawSystem = ((SceneEditor2 *) GetScene())->landscapeEditorDrawSystem;
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

bool HeightmapEditorSystem::EnableLandscapeEditing()
{
	if (enabled)
	{
		return true;
	}
	
	selectionSystem->SetLocked(true);
	modifSystem->SetLocked(true);
	
	drawSystem->EnableCustomDraw();

	landscapeSize = drawSystem->GetHeightmapProxy()->Size();
	copyPasteFrom = Vector2(-1.f, -1.f);

	drawSystem->EnableCursor(landscapeSize);
	drawSystem->SetCursorTexture(cursorTexture);
	drawSystem->SetCursorSize(cursorSize);
	
	enabled = true;
	return enabled;
}

bool HeightmapEditorSystem::DisableLandscapeEdititing()
{
	if (!enabled)
	{
		return true;
	}
	
	selectionSystem->SetLocked(false);
	modifSystem->SetLocked(false);
	
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
					
					SceneSignals::Instance()->EmitUpdateDropperHeight(dynamic_cast<SceneEditor2*>(GetScene()), curHeight);
				}
				
				if (isIntersectsLandscape)
				{
					if (drawingType == HEIGHTMAP_COPY_PASTE)
					{
						if (IsKeyModificatorPressed(DVKEY_ALT)) //(copyPasteFrom == Vector2(-1.f, -1.f))
						{
							copyPasteFrom = cursorPosition;
							copyPasteTo = Vector2(-1.f, -1.f);
						}
						else
						{
							copyPasteTo = cursorPosition;
							StoreOriginalHeightmap();
						}
					}
					else
					{
						StoreOriginalHeightmap();
					}

					UpdateToolImage();
					editingIsEnabled = true;
				}
				break;
				
			case UIEvent::PHASE_DRAG:
				break;
				
			case UIEvent::PHASE_ENDED:
				if (editingIsEnabled)
				{
					CreateUndoPoint();
					
					editingIsEnabled = false;
				}
				break;
		}
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
	Vector2 pos = cursorPosition - Vector2((float32)scaleSize, (float32)scaleSize)/2.0f;
	{
		switch (drawingType)
		{
			case HEIGHTMAP_DRAW_RELATIVE:
			{
				float32 koef = (strength * timeElapsed);
				if(inverseDrawingEnabled)
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
				float32 height = drawSystem->GetHeightAtPoint(cursorPosition);
				SceneSignals::Instance()->EmitUpdateDropperHeight(dynamic_cast<SceneEditor2*>(GetScene()), height);
				return;
			}

			case HEIGHTMAP_COPY_PASTE:
			{
				if (copyPasteFrom == Vector2(-1.f, -1.f) || copyPasteTo == Vector2(-1.f, -1.f))
				{
					return;
				}

				int32 scaleSize = toolImage->GetWidth();
				Vector2 posTo = pos;

				Vector2 deltaPos = cursorPosition - copyPasteTo;
				Vector2 posFrom = copyPasteFrom + deltaPos - Vector2((float32)scaleSize, (float32)scaleSize)/2.f;

				float32 koef = (averageStrength * timeElapsed) * 2.0f;

				editorHeightmap->DrawCopypasteRGBA(toolImage, posFrom, posTo, scaleSize, scaleSize, koef);

				break;
			}
				
			default:
				DAVA::Logger::Error("Invalid drawing type!");
				return;
		}
		
		Rect rect(pos.x, pos.y, (float32)scaleSize, (float32)scaleSize);
		drawSystem->GetHeightmapProxy()->UpdateRect(rect);
		AddRectToAccumulator(rect);
	}
}

void HeightmapEditorSystem::ResetAccumulatorRect()
{
	float32 inf = std::numeric_limits<float32>::infinity();
	updatedRectAccumulator = Rect(inf, inf, -inf, -inf);
}

void HeightmapEditorSystem::AddRectToAccumulator(const Rect &rect)
{
	updatedRectAccumulator = updatedRectAccumulator.Combine(rect);
}

Rect HeightmapEditorSystem::GetUpdatedRect()
{
	EditorHeightmap* editorHeightmap = drawSystem->GetHeightmapProxy();
	
	int32 heightmapSize = editorHeightmap->Size() - 1;
	Rect r = updatedRectAccumulator;

	r.x = (float32)Clamp((int32)updatedRectAccumulator.x, 0, heightmapSize - 1);
	r.y = (float32)Clamp((int32)updatedRectAccumulator.y, 0, heightmapSize - 1);
	r.dx = Clamp((updatedRectAccumulator.x + updatedRectAccumulator.dx),
				 0.f, (float32)heightmapSize - 1.f) - updatedRectAccumulator.x;
	r.dy = Clamp((updatedRectAccumulator.y + updatedRectAccumulator.dy),
				 0.f, (float32)heightmapSize - 1.f) - updatedRectAccumulator.y;

	return r;
}

void HeightmapEditorSystem::StoreOriginalHeightmap()
{
	EditorHeightmap* editorHeightmap = drawSystem->GetHeightmapProxy();
	
	DVASSERT(originalHeightmap == NULL);
	originalHeightmap = editorHeightmap->Clone(NULL);
	ResetAccumulatorRect();
}

void HeightmapEditorSystem::CreateUndoPoint()
{
	SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
	DVASSERT(scene);
	scene->Exec(new ModifyHeightmapCommand(drawSystem->GetHeightmapProxy(),
										   originalHeightmap,
										   GetUpdatedRect()));

	SafeRelease(originalHeightmap);
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
	if (s <= MAX_STRENGTH)
	{
		this->strength = s;
		
		inverseDrawingEnabled = false;
		if (strength < 0.f)
		{
			inverseDrawingEnabled = true;
		}
	}
}

void HeightmapEditorSystem::SetAverageStrength(float32 averageStrength)
{
	if (averageStrength >= 0)
	{
		this->averageStrength = averageStrength;
	}
}

void HeightmapEditorSystem::SetToolImage(const FilePath& toolImagePath)
{
	this->toolImagePath = toolImagePath;
	UpdateToolImage(true);
}

void HeightmapEditorSystem::SetDrawingType(eHeightmapDrawType type)
{
	copyPasteFrom = Vector2(-1.f, -1.f);

	if (type == HEIGHTMAP_COPY_PASTE)
	{
		drawSystem->SetCursorTexture(squareTexture);
	}
	else if (drawingType == HEIGHTMAP_COPY_PASTE)
	{
		drawSystem->SetCursorTexture(cursorTexture);
	}

	drawingType = type;
}
