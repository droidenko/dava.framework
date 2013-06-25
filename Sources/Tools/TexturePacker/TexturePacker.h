#ifndef __DAVAENGINE_TEXTURE_PACKER_H__
#define __DAVAENGINE_TEXTURE_PACKER_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Math/Math2D.h"
#include "Render/Texture.h"

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
	void PackToTextures(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & defsList, eGPUFamily forGPU);
	// page each PSD file to separate texture
	void PackToTexturesSeparate(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & defsList, eGPUFamily forGPU);
	// pack one sprite and use several textures if more than one needed
	void PackToMultipleTextures(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & remainingList, eGPUFamily forGPU);
	
	// batch textures to one (simplified version of PackToTextures code).
	struct BatchTexturesOutputData
	{
		FilePath texturePath;
		int32 offsetX;
		int32 offsetY;
	};
	
	enum eBatchTexturesErrorCode
	{
		SUCCESS = 0,
		NO_TEXTURES_TO_BATCH,
		TEXTURES_TOO_LARGE
	};
	
	// The result of whole Batch Textures execution.
	struct BatchTexturesResult
	{
		eBatchTexturesErrorCode errorCode;
		FilePath batchedTexturePath;
		int32 batchedTextureWidth;
		int32 batchedTextureHeight;
		Map<FilePath, BatchTexturesOutputData> outputData;
	};
	
	BatchTexturesResult Batch(List<Texture*> texturesList, int32 batchID);

	bool TryToPack(const Rect2i & textureRect, List<DefinitionFile*> & defsList);
	bool WriteDefinition(const FilePath & excludeFolder, const FilePath & outputPath, const String & textureName, DefinitionFile * defFile);
	bool WriteMultipleDefinition(const FilePath & excludeFolder, const FilePath & outputPath, const String & _textureName, DefinitionFile * defFile);

	int TryToPackFromSortVector(ImagePacker * packer, Vector<SizeSortItem> & tempSortVector);
	float TryToPackFromSortVectorWeight(ImagePacker * packer, Vector<SizeSortItem> & tempSortVector);

	Rect2i ReduceRectToOriginalSize(const Rect2i & _input);

	void UseOnlySquareTextures();

	void SetMaxTextureSize(int32 maxTextureSize);
	
	// Determine the best resolution for the single texture. Returns FALSE if the textures can't be
	// packed to the single one.
	bool DetermineBestResolution(int& bestXResolution, int& bestYResolution, int& bestResolution,
								 List<DefinitionFile*> & defsList, eGPUFamily forGPU);

private:
    
    void ExportImage(PngImageExt *image, const FilePath &exportedPathname, eGPUFamily forGPU, bool forceGenerateMipmaps = false);
    TextureDescriptor * CreateDescriptor(eGPUFamily forGPU, bool forceGenerateMipmaps = false);

	// Batch Textures functionality.
	BatchTexturesResult BatchTextures(const FilePath & outputPath, List<DefinitionFile*> & defsList, int32 batchID);

	BatchTexturesResult BatchErrorResult(eBatchTexturesErrorCode errorCode);

	ImagePacker *			lastPackedPacker;
	Vector<ImagePacker*> usedPackers;

	Vector<SizeSortItem> sortVector;
	int32 maxTextureSize;

	bool onlySquareTextures;
    bool NeedSquareTextureForCompression(eGPUFamily forGPU);
};

};


#endif // __DAVAENGINE_TEXTURE_PACKER_H__

