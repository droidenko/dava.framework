#ifndef __DAVAENGINE_DEFINITION_FILE_H__
#define __DAVAENGINE_DEFINITION_FILE_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "Math/Math2D.h"

#include "Render/Texture.h"

namespace DAVA
{
 
class DefinitionFile 
{
public:
	bool InitFromTexture(Texture* texture);
	bool Load(const FilePath & filename);
	bool LoadPNGDef(const FilePath & filename, const FilePath & pathToProcess);
	
	DefinitionFile();
	~DefinitionFile();
	
	void ClearPackedFrames();
	void LoadPNG(const FilePath & fullname, const FilePath & processDirectoryPath);

	FilePath    filename;
	int			frameCount;
	int			spriteWidth;
	int			spriteHeight;
	Rect2i		* frameRects;

	Vector<String> pathsInfo;
};

};


#endif // __DAVAENGINE_DEFINITION_FILE_H__