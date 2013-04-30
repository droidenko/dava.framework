#ifndef __DAVAENGINE_TEXTURE_PACKER_H__
#define __DAVAENGINE_TEXTURE_PACKER_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Math/Math2D.h"

namespace DAVA
{

class DefinitionFile;
class TextureDescriptor;
class PngImageExt;
class FilePath;
class ImagePacker;

struct SizeSortItem
{
	int					imageSize;
	DefinitionFile *	defFile;
	int					frameIndex;
};
    
class TexturePacker 
{
public:
	TexturePacker();
	
	// pack textures to single texture
	void PackToTextures(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & defsList);
	// page each PSD file to separate texture
	void PackToTexturesSeparate(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & defsList);
	// pack one sprite and use several textures if more than one needed
	void PackToMultipleTextures(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & remainingList);

	bool TryToPack(const Rect2i & textureRect, List<DefinitionFile*> & defsList);
	bool WriteDefinition(const FilePath & excludeFolder, const FilePath & outputPath, const String & textureName, DefinitionFile * defFile);
	bool WriteMultipleDefinition(const FilePath & excludeFolder, const FilePath & outputPath, const String & _textureName, DefinitionFile * defFile);

	int TryToPackFromSortVector(ImagePacker * packer, Vector<SizeSortItem> & tempSortVector);
	float TryToPackFromSortVectorWeight(ImagePacker * packer, Vector<SizeSortItem> & tempSortVector);

	Rect2i ReduceRectToOriginalSize(const Rect2i & _input);

	void UseOnlySquareTextures();

	void SetMaxTextureSize(int32 maxTextureSize);
	
private:
    
    void ExportImage(PngImageExt *image, const FilePath &exportedPathname);
    TextureDescriptor * CreateDescriptor();
    PixelFormat DetectPixelFormatFromFlags();
    
    
	ImagePacker *			lastPackedPacker;
	Vector<ImagePacker*> usedPackers;

	Vector<SizeSortItem> sortVector;
	int32 maxTextureSize;

	bool onlySquareTextures;
};

};


#endif // __DAVAENGINE_TEXTURE_PACKER_H__

