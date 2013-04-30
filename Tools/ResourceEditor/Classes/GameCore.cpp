/*
 *  GameCore.cpp
 *  TemplateProjectMacOS
 *
 *  Created by Vitaliy  Borodovsky on 3/19/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "GameCore.h"
#include "AppScreens.h"

#ifdef __DAVAENGINE_BEAST__
#include "BeastProxyImpl.h"
#else
#include "BeastProxy.h"
#endif //__DAVAENGINE_BEAST__

#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/SceneValidator.h"
#include "PVRConverter.h"

#include "SceneEditor/CommandLineTool.h"
#include "SceneEditor/SceneUtilsScreen.h"

#include "ImageSplitter/ImageSplitterScreen.h"

#include "Qt/TextureBrowser/TextureConvertor.h"
#include "Qt/DockParticleEditor/ParticlesEditorController.h"

using namespace DAVA;


GameCore::GameCore()
{ }

GameCore::~GameCore()
{ }

void GameCore::OnAppStarted()
{
	Logger::Instance()->SetLogFilename("ResEditor.txt");
	RenderManager::Instance()->SetFPS(30);

    LocalizationSystem::Instance()->SetCurrentLocale(EditorSettings::Instance()->GetLanguage());
	LocalizationSystem::Instance()->InitWithDirectory("~res:/Strings/");

    
#ifdef __DAVAENGINE_BEAST__
	new BeastProxyImpl();
#else 
    new BeastProxy();
#endif //__DAVAENGINE_BEAST__
	
	new PVRConverter();

#if defined (__DAVAENGINE_MACOS__)
	PVRConverter::Instance()->SetPVRTexTool(String("~res:/PVRTexToolCL"));
#elif defined (__DAVAENGINE_WIN32__)
    PVRConverter::Instance()->SetPVRTexTool(String("~res:/PVRTexToolCL.exe"));
#endif

    sceneUtilsScreen = new SceneUtilsScreen();

	new ParticlesEditorController();
    imageSplitterScreen = new ImageSplitterScreen();

    UIScreenManager::Instance()->RegisterScreen(SCREEN_UTILS_SCENE, sceneUtilsScreen);
    UIScreenManager::Instance()->RegisterScreen(SCREEN_UTILS_IMAGE_SPLITTER, imageSplitterScreen);

    
    if( CommandLineTool::Instance() &&
		(CommandLineTool::Instance()->CommandIsFound(String("-sceneexporter")) ||
		 CommandLineTool::Instance()->CommandIsFound(String("-scenesaver"))))
    {
        UIScreenManager::Instance()->SetFirst(SCREEN_UTILS_SCENE);
    }
    else if(CommandLineTool::Instance() && CommandLineTool::Instance()->CommandIsFound(String("-imagesplitter")))
    {
        UIScreenManager::Instance()->SetFirst(SCREEN_UTILS_IMAGE_SPLITTER);
    }
    else
    {
        Texture::SetDefaultFileFormat((ImageFileFormat)EditorSettings::Instance()->GetTextureViewFileFormat());
    }
	
	// Yuri Coder, 2013/01/23. The call below is needed for Win32 linker to notify it we are using
	// ParticleEffectNode and thus REGISTER_CLASS(ParticleEffectNode) must not be removed during optimization.
	ParticleEffectNode* effectNode = new ParticleEffectNode();
	delete effectNode;
}

void GameCore::OnAppFinished()
{
	PVRConverter::Instance()->Release();
    SceneValidator::Instance()->Release();

	BeastProxy::Instance()->Release();

    SafeRelease(sceneUtilsScreen);
    SafeRelease(imageSplitterScreen);
}

void GameCore::OnSuspend()
{
	//prevent going to suspend
    //ApplicationCore::OnSuspend();
}

void GameCore::OnResume()
{
    ApplicationCore::OnResume();
}

void GameCore::OnBackground()
{
	//ApplicationCore::OnBackground();
}

void GameCore::BeginFrame()
{
	ApplicationCore::BeginFrame();
	RenderManager::Instance()->ClearWithColor(0, 0, 0, 0);
}

void GameCore::Update(float32 timeElapsed)
{
	ApplicationCore::Update(timeElapsed);
}

void GameCore::Draw()
{
	ApplicationCore::Draw();
}


