#ifndef __DAVAENGINE_RESOURCEPACKER2D_H__
#define __DAVAENGINE_RESOURCEPACKER2D_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

class DefinitionFile;
class ResourcePacker2D
{
public:
	ResourcePacker2D();

	// Packing of resources section
	void InitFolders(const FilePath & inputPath,const FilePath & outputPath);
	void PackResources();
	
	void RecursiveTreeWalk(const FilePath & inputPath,const FilePath & outputPath);

	bool IsMD5ChangedDir(const FilePath & processDirectoryPath, const FilePath & pathname, const String & psdName, bool isRecursive);
	bool IsMD5ChangedFile(const FilePath & processDirectoryPath, const FilePath & pathname, const String & psdName);
	
    DefinitionFile * ProcessPSD(const FilePath & processDirectoryPath, const FilePath & psdPathname, const String & psdName);
	void ProcessFlags(const FilePath & flagsPathname);

	static String GetProcessFolderName();

public:
    
	FilePath inputGfxDirectory;
	FilePath outputGfxDirectory;
	FilePath excludeDirectory;
	String gfxDirName;
	String currentFlags;
    
	bool isGfxModified;
    
	bool isLightmapsPacking;
	bool clearProcessDirectory;
};

};


#endif // __DAVAENGINE_RESOURCEPACKER2D_H__
