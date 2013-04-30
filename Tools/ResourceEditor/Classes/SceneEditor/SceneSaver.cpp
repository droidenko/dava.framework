#include "SceneSaver.h"
#include "SceneValidator.h"

#include "Qt/Scene/SceneDataManager.h"

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


void SceneSaver::SaveFile(const FilePath &fileName, Set<String> &errorLog)
{
    Logger::Info("[SceneSaver::SaveFile] %s", fileName.GetAbsolutePathname().c_str());
    
    //Load scene with *.sc2
    Scene *scene = new Scene();
    Entity *rootNode = scene->GetRootNode(sceneUtils.dataSourceFolder + fileName);
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
		
		SaveScene(scene, fileName, errorLog);
    }
	else
	{
		errorLog.insert(Format("[SceneSaver::SaveFile] Can't open file %s", fileName.GetAbsolutePathname().c_str()));
	}

    SafeRelease(scene);
}

void SceneSaver::ResaveFile(const FilePath &fileName, Set<String> &errorLog)
{
	Logger::Info("[SceneSaver::ResaveFile] %s", fileName.GetAbsolutePathname().c_str());

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
		errorLog.insert(Format("[SceneSaver::ResaveFile] Can't open file %s", fileName.GetAbsolutePathname().c_str()));
	}

	SafeRelease(scene);
}

void SceneSaver::SaveScene(Scene *scene, const FilePath &fileName, Set<String> &errorLog)
{
    DVASSERT(0 == texturesForSave.size())
    
    //Create destination folder

    sceneUtils.workingFolder = FilePath(fileName.GetDirectory());
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
    FilePath sceneName(fileName.GetFilename());
    sceneName.ReplaceExtension(".saved.sc2");

    FilePath tempSceneName = sceneUtils.workingFolder + sceneName;
    
    SceneFileV2 * outFile = new SceneFileV2();
    outFile->EnableSaveForGame(true);
    outFile->EnableDebugLog(false);
    
    outFile->SaveScene(sceneUtils.dataSourceFolder + tempSceneName, scene);
    SafeRelease(outFile);

    bool moved = FileSystem::Instance()->MoveFile(sceneUtils.dataSourceFolder + tempSceneName, sceneUtils.dataFolder + fileName, true);
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
    FilePath pngPathname = TextureDescriptor::GetPathnameForFormat(texturePathname, PNG_FILE);

    sceneUtils.CopyFile(descriptorPathname, errorLog);
    sceneUtils.CopyFile(pngPathname, errorLog);
}

void SceneSaver::CopyReferencedObject( Entity *node, Set<String> &errorLog )
{
	KeyedArchive *customProperties = node->GetCustomProperties();
	if(customProperties && customProperties->IsKeyExists("editor.referenceToOwner"))
	{
		String path = customProperties->GetString("editor.referenceToOwner");
		sceneUtils.CopyFile(path, errorLog);
	}
	for (int i = 0; i < node->GetChildrenCount(); i++)
	{
		CopyReferencedObject(node->GetChild(i), errorLog);
	}

}


