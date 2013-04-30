#include "SceneUtils.h"

using namespace DAVA;

SceneUtils::SceneUtils()
{
}

SceneUtils::~SceneUtils()
{
}

void SceneUtils::CleanFolder(const FilePath &folderPathname, Set<String> &errorLog)
{
    bool ret = FileSystem::Instance()->DeleteDirectory(folderPathname);
    if(!ret)
    {
        bool folderExists = FileSystem::Instance()->IsDirectory(folderPathname);
        if(folderExists)
        {
            errorLog.insert(String(Format("[CleanFolder] ret = %d, folder = %s", ret, folderPathname.GetAbsolutePathname().c_str())));
        }
    }
}

void SceneUtils::SetInFolder(const FilePath &folderPathname)
{
    DVASSERT(folderPathname.IsDirectoryPathname());
    dataSourceFolder = folderPathname;
}

void SceneUtils::SetOutFolder(const FilePath &folderPathname)
{
    DVASSERT(folderPathname.IsDirectoryPathname());
    dataFolder = folderPathname;
}


bool SceneUtils::CopyFile(const FilePath &filePathname, Set<String> &errorLog)
{
    String workingPathname = RemoveFolderFromPath(filePathname, dataSourceFolder);
    PrepareFolderForCopyFile(workingFolder, errorLog);
    
    bool retCopy = FileSystem::Instance()->CopyFile(dataSourceFolder + workingPathname, dataFolder + workingPathname);
    if(!retCopy)
    {
        errorLog.insert(String(Format("Can't copy %s from %s to %s",
                                      workingPathname.c_str(),
                                      dataSourceFolder.GetAbsolutePathname().c_str(),
                                      dataFolder.GetAbsolutePathname().c_str())));
    }
    
    return retCopy;
}

void SceneUtils::PrepareFolderForCopyFile(const String &filename, Set<String> &errorLog)
{
    FilePath newFolderPath = (dataFolder + filename).GetDirectory();
    
    if(!FileSystem::Instance()->IsDirectory(newFolderPath))
    {
        FileSystem::eCreateDirectoryResult retCreate = FileSystem::Instance()->CreateDirectory(newFolderPath, true);
        if(FileSystem::DIRECTORY_CANT_CREATE == retCreate)
        {
            errorLog.insert(String(Format("Can't create folder %s", newFolderPath.GetAbsolutePathname().c_str())));
        }
    }
    
    FileSystem::Instance()->DeleteFile(dataFolder + filename);
}

String SceneUtils::RemoveFolderFromPath(const FilePath &pathname, const FilePath &folderPathname)
{
    DVASSERT(folderPathname.IsDirectoryPathname());

    String workingPathname = pathname.GetAbsolutePathname();
    String::size_type pos = workingPathname.find(folderPathname.GetAbsolutePathname());
    if(String::npos != pos)
    {
        workingPathname = workingPathname.replace(pos, folderPathname.GetAbsolutePathname().length(), "");
    }

    return workingPathname;
}

