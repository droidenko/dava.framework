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

#include "SceneSaver.h"
#include "SceneEditor/SceneValidator.h"

#include "Qt/Scene/SceneDataManager.h"
#include "../StringConstants.h"

using namespace DAVA;

SceneSaver::SceneSaver()
{
}

SceneSaver::~SceneSaver()
{
    ReleaseTextures();
}


void SceneSaver::SetInFolder(const FilePath &folderPathname)
{
    sceneUtils.SetInFolder(folderPathname);
}

void SceneSaver::SetOutFolder(const FilePath &folderPathname)
{
    sceneUtils.SetOutFolder(folderPathname);
}


void SceneSaver::SaveFile(const String &fileName, Set<String> &errorLog)
{
    Logger::Info("[SceneSaver::SaveFile] %s", fileName.c_str());
    
    FilePath filePath = sceneUtils.dataSourceFolder + fileName;

    //Load scene with *.sc2
    Scene *scene = new Scene();
    Entity *rootNode = scene->GetRootNode(filePath);
    if(rootNode)
    {
        int32 count = rootNode->GetChildrenCount();
		Vector<Entity*> tempV;
		tempV.reserve((count));
        for(int32 i = 0; i < count; ++i)
        {
			tempV.push_back(rootNode->GetChild(i));
        }
		for(int32 i = 0; i < count; ++i)
		{
			scene->AddNode(tempV[i]);
		}
		
		SaveScene(scene, filePath, errorLog);
    }
	else
	{
		errorLog.insert(Format("[SceneSaver::SaveFile] Can't open file %s", fileName.c_str()));
	}

    SafeRelease(scene);
}

void SceneSaver::ResaveFile(const String &fileName, Set<String> &errorLog)
{
	Logger::Info("[SceneSaver::ResaveFile] %s", fileName.c_str());

	FilePath sc2Filename = sceneUtils.dataSourceFolder + fileName;

	//Load scene with *.sc2
	Scene *scene = new Scene();
	Entity *rootNode = scene->GetRootNode(sc2Filename);
	if(rootNode)
	{
		int32 count = rootNode->GetChildrenCount();

		Vector<Entity*> tempV;
		tempV.reserve((count));
		for(int32 i = 0; i < count; ++i)
		{
			tempV.push_back(rootNode->GetChild(i));
		}
		for(int32 i = 0; i < count; ++i)
		{
			scene->AddNode(tempV[i]);
		}

		scene->Update(0.f);

		SceneFileV2 * outFile = new SceneFileV2();
		outFile->EnableDebugLog(false);
		outFile->SaveScene(sc2Filename, scene);
		SafeRelease(outFile);
	}
	else
	{
		errorLog.insert(Format("[SceneSaver::ResaveFile] Can't open file %s", fileName.c_str()));
	}

	SafeRelease(scene);
}

void SceneSaver::SaveScene(Scene *scene, const FilePath &fileName, Set<String> &errorLog)
{
    DVASSERT(0 == texturesForSave.size())
    
    String relativeFilename = fileName.GetRelativePathname(sceneUtils.dataSourceFolder);
    sceneUtils.workingFolder = fileName.GetDirectory().GetRelativePathname(sceneUtils.dataSourceFolder);
    
    FileSystem::Instance()->CreateDirectory(sceneUtils.dataFolder + sceneUtils.workingFolder, true);

    scene->Update(0.1f);

    FilePath oldPath = SceneValidator::Instance()->SetPathForChecking(sceneUtils.dataSourceFolder);
    SceneValidator::Instance()->ValidateScene(scene, errorLog);

    texturesForSave.clear();
    SceneDataManager::EnumerateTextures(scene, texturesForSave);

    CopyTextures(scene, errorLog);
	ReleaseTextures();

	Landscape *landscape = EditorScene::GetLandscape(scene);
    if (landscape)
    {
        sceneUtils.CopyFile(landscape->GetHeightmapPathname(), errorLog);
    }

	CopyReferencedObject(scene, errorLog);

    //save scene to new place
    FilePath tempSceneName = sceneUtils.dataSourceFolder + relativeFilename;
    tempSceneName.ReplaceExtension(".saved.sc2");
    
    SceneFileV2 * outFile = new SceneFileV2();
    outFile->EnableSaveForGame(true);
    outFile->EnableDebugLog(false);
    
    outFile->SaveScene(tempSceneName, scene);
    SafeRelease(outFile);

    bool moved = FileSystem::Instance()->MoveFile(tempSceneName, sceneUtils.dataFolder + relativeFilename, true);
	if(!moved)
	{
		errorLog.insert(Format("Can't move file %s", fileName.GetAbsolutePathname().c_str()));
	}
    
    SceneValidator::Instance()->SetPathForChecking(oldPath);
}


void SceneSaver::CopyTextures(DAVA::Scene *scene, Set<String> &errorLog)
{
    Map<String, Texture *>::const_iterator endIt = texturesForSave.end();
    Map<String, Texture *>::iterator it = texturesForSave.begin();
    for( ; it != endIt; ++it)
    {
        CopyTexture(it->first, errorLog);
    }
}

void SceneSaver::ReleaseTextures()
{
    texturesForSave.clear();
}

void SceneSaver::CopyTexture(const FilePath &texturePathname, Set<String> &errorLog)
{
    FilePath descriptorPathname = TextureDescriptor::GetDescriptorPathname(texturePathname);
    FilePath pngPathname = GPUFamilyDescriptor::CreatePathnameForGPU(texturePathname, GPU_UNKNOWN, FORMAT_RGBA8888);

    sceneUtils.CopyFile(descriptorPathname, errorLog);
    sceneUtils.CopyFile(pngPathname, errorLog);
}

void SceneSaver::CopyReferencedObject( Entity *node, Set<String> &errorLog )
{
	KeyedArchive *customProperties = node->GetCustomProperties();
	if(customProperties && customProperties->IsKeyExists(ResourceEditor::EDITOR_REFERENCE_TO_OWNER))
	{
		String path = customProperties->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
		sceneUtils.CopyFile(path, errorLog);
	}
	for (int i = 0; i < node->GetChildrenCount(); i++)
	{
		CopyReferencedObject(node->GetChild(i), errorLog);
	}

}


