#ifndef __SCENE_UTILS_H__
#define __SCENE_UTILS_H__

#include "DAVAEngine.h"

class SceneUtils
{
public:

	SceneUtils();
    ~SceneUtils();
    
    void CleanFolder(const DAVA::FilePath &folderPathname, DAVA::Set<DAVA::String> &errorLog);
    
    void SetInFolder(const DAVA::FilePath &folderPathname);
    void SetOutFolder(const DAVA::FilePath &folderPathname);
    
    bool CopyFile(const DAVA::FilePath &filePathname, DAVA::Set<DAVA::String> &errorLog);
    
    DAVA::String RemoveFolderFromPath(const DAVA::FilePath &pathname, const DAVA::FilePath &folderPathname);

    void PrepareFolderForCopyFile(const DAVA::String &filename, DAVA::Set<DAVA::String> &errorLog);

public:

    DAVA::FilePath dataFolder;
    DAVA::FilePath dataSourceFolder;
    DAVA::String workingFolder;
};



#endif // __SCENE_UTILS_H__