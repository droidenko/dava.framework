#include "ResultScreen.h"
#include "SettingsManager.h"
#include "GameCore.h"
#include "Config.h"
#include "DeviceInfo.h"

using namespace DAVA;

ResultScreen::ResultScreen(const LandscapeTestData& testData, const FilePath& filename, Texture* landscapeTexture)
:	isFinished(false),
	state(RESULT_STATE_NORMAL),
	testData(testData)
{
	this->filename = filename;
	
	texture = SafeRetain(landscapeTexture);
	textureSprite = NULL;
	resultSprite = NULL;
}

ResultScreen::~ResultScreen()
{
    SafeRelease(fileNameText);
    SafeRelease(statText[0]);
    SafeRelease(statText[1]);
    SafeRelease(statText[2]);
    SafeRelease(tapToContinue);
    SafeRelease(screenshotText);
	SafeRelease(textureSprite);
	SafeRelease(resultSprite);
	SafeRelease(texture);
}

void ResultScreen::LoadResources()
{
	Vector2 spriteSize((float32)texture->GetWidth(), (float32)texture->GetHeight());
	textureSprite = Sprite::CreateFromTexture(texture, 0, 0, spriteSize.x, spriteSize.y);
	resultSprite = Sprite::CreateAsRenderTarget(spriteSize.x * RESULT_TEXTURE_SCALE,
                                                spriteSize.y * RESULT_TEXTURE_SCALE,
                                                FORMAT_RGBA8888, true);
}

void ResultScreen::UnloadResources()
{
	RemoveAllControls();
}

void ResultScreen::WillAppear()
{
    PrepareSprite();
}

void ResultScreen::WillDisappear()
{
}

void ResultScreen::SaveResults()
{
    Core *core=DAVA::Core::Instance();
    Vector2 screenSize(core->GetVirtualScreenWidth(), core->GetVirtualScreenHeight());
    
    Image* image = resultSprite->GetTexture()->CreateImageFromMemory();
    FilePath saveFileName = FileSystem::Instance()->GetUserDocumentsPath();
    saveFileName += filename.GetFilename() + ".png";
    ImageLoader::Save(image, saveFileName);
    
    Map<String, String> results;
	Map<String, Texture *> textures;
	results["TextureMemorySize"] = Format("%.2f Mb", testData.GetTextureMemorySize()/(1024.f * 1024.f));
	results["TextureFilesSize"] = Format("%.2f Mb", testData.GetTexturesFilesSize()/(1024.f * 1024.f));

	String filePath = testData.GetSceneFilePath().GetAbsolutePathname();
    results["SceneFilePath"] = filePath.substr(filePath.find("Maps"));
    
	FilePath folderPathname("~doc:/PerformanceTestResult/");
    FileSystem::Instance()->CreateDirectory(folderPathname);
    FilePath statFileName = folderPathname + FilePath::CreateWithNewExtension(filename, ".txt").GetFilename();
    File* file = File::Create(statFileName, File::CREATE | File::WRITE);
    if (file)
    {
        Map<String, String>::const_iterator it = results.begin();
        for(; it != results.end(); it++)
        {
            // "Format" sometimes doesn't work correct with strings on some platforms
            file->WriteLine(((*it).first + ": " + (*it).second).c_str());
        }
        
        SafeRelease(file);
    }
    
    FilePath levelName = FilePath::CreateWithNewExtension(filename, "").GetFilename();

    if(!GameCore::Instance()->FlushToDB(levelName, results, saveFileName))
        Logger::Debug("Error sending data to DB (connection is lost) !!!");
    
    state = RESULT_STATE_FINISHED;
}

void ResultScreen::Input(UIEvent * event)
{
	if(event->phase == UIEvent::PHASE_BEGAN)
    {
		if(!isFinished && state == RESULT_STATE_NORMAL)
        {
            if(resultSprite != 0)
            {
                SaveResults();
            }
		}
	}
}

void ResultScreen::Update(float32 timeElapsed)
{
	UIScreen::Update(timeElapsed);
	
    if(!isFinished && state == RESULT_STATE_NORMAL && resultSprite != 0)
    {
        SaveResults();
    }
    
	switch (state)
    {
		case RESULT_STATE_MAKING_SCREEN_SHOT:
            break;
			
		case RESULT_STATE_FINISHED:
			isFinished=true;
			break;
			
		default:
			break;
	}
}

void ResultScreen::Draw(const UIGeometricData &geometricData)
{
	Core *core=DAVA::Core::Instance();

	Vector2 screenSize(core->GetVirtualScreenWidth(), core->GetVirtualScreenHeight());
	
	float32 drawSize = Min(screenSize.x, screenSize.y);
	float32 scale = Min(drawSize / resultSprite->GetWidth(), drawSize / resultSprite->GetHeight());
	
	resultSprite->SetPosition(0, 0);
	resultSprite->SetScale(scale, scale);
	
	resultSprite->Draw();

    UIScreen::Draw(geometricData);
}

void ResultScreen::PrepareSprite()
{
	Rect r(0, 0, resultSprite->GetWidth(), resultSprite->GetHeight());

	RenderManager::Instance()->SetRenderTarget(resultSprite);
    textureSprite->SetScale(RESULT_TEXTURE_SCALE, RESULT_TEXTURE_SCALE);
	textureSprite->Draw();
	DrawStatImage(r);
	RenderManager::Instance()->RestoreRenderTarget();
}

void ResultScreen::DrawStatImage(Rect rect)
{
	RenderHelper *helper = RenderHelper::Instance();
	RenderManager *manager = RenderManager::Instance();

	for(uint32 i = 0; i < testData.GetItemCount(); ++i)
	{
		FpsStatItem item = testData.GetItem(i);
		Rect curRect = testData.TranslateRect(item.rect, rect);
		for(uint32 j = 0; j < SECTORS_COUNT; j++)
		{
			manager->SetColor(SettingsManager::Instance()->GetColorByFps(item.avFps[j]));
			Polygon2 curSector;
			curSector.AddPoint(curRect.GetCenter());
			curSector.AddPoint(GetVecInRect(curRect, DegToRad((SECTORS_COUNT - j) * 45.f - 22.5f)));
			curSector.AddPoint(GetVecInRect(curRect, DegToRad((SECTORS_COUNT - j) * 45.f)));
			curSector.AddPoint(GetVecInRect(curRect, DegToRad((SECTORS_COUNT - j) * 45.f + 22.5f)));
			helper->FillPolygon(curSector);
			manager->SetColor(Color::Black());
			helper->DrawPolygon(curSector, true);
		}
	}
}

Vector2 ResultScreen::GetVecInRect(const Rect & rect, float32 angleInRad)
{
	Vector2 retVec;
	Matrix2 m;
	m.BuildRotation(angleInRad);
	angleInRad += DAVA::PI_05;
	while(angleInRad > DAVA::PI_05)
		angleInRad -= DAVA::PI_05;
	if(angleInRad > DAVA::PI_05 / 2)
		angleInRad = DAVA::PI_05 - angleInRad;
	Vector2 v = Vector2((Point2f(rect.GetSize().x / 2, 0) * m).data) / Abs(cosf(angleInRad));
    
	retVec = v + rect.GetCenter();
	return retVec;
}
