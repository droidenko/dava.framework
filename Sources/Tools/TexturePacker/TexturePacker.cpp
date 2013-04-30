#include "TexturePacker/TexturePacker.h"
#include "TexturePacker/CommandLineParser.h"
#include "TexturePacker/ImagePacker.h"
#include "TexturePacker/PngImage.h"
#include "TexturePacker/DefinitionFile.h"
#include "Render/TextureDescriptor.h"
#include "FileSystem/FileSystem.h"
#include "Render/Texture.h"
#include "PVRConverter.h"
#include "DXTConverter.h"


#ifdef WIN32
#define snprintf _snprintf
#endif

namespace DAVA
{

TexturePacker::TexturePacker()
{
	maxTextureSize = 1024;
	if (CommandLineParser::Instance()->IsFlagSet("--tsize2048"))
	{
		maxTextureSize = 2048;
	}

	onlySquareTextures = false;
}

bool TexturePacker::TryToPack(const Rect2i & textureRect, List<DefinitionFile*> & defsList)
{
	if (CommandLineParser::Instance()->GetVerbose())
    {
        Logger::Info("%d x %d, ", textureRect.dx, textureRect.dy);
    }
    
	ImagePacker * packer = new ImagePacker(textureRect);
	
	// Packing of sorted by size images
	for (int i = 0; i < (int)sortVector.size(); ++i)
	{
		DefinitionFile * defFile = sortVector[i].defFile;
		int frame = sortVector[i].frameIndex;
		if (!packer->AddImage(Size2i(defFile->frameRects[frame].dx, defFile->frameRects[frame].dy), &defFile->frameRects[frame]))
		{
			SafeDelete(packer);
			return false;
		}
		if (CommandLineParser::Instance()->GetVerbose())
			Logger::Info("p: %s %d\n",defFile->filename.GetAbsolutePathname().c_str(), frame);
	}
	if (CommandLineParser::Instance()->GetVerbose())
		Logger::Info("\n* %d x %d - success\n", textureRect.dx, textureRect.dy);
	
	if (lastPackedPacker)
	{
		SafeDelete(lastPackedPacker);
	}
	
	lastPackedPacker = packer;
	return true;
}

int TexturePacker::TryToPackFromSortVector(ImagePacker * packer,Vector<SizeSortItem> & tempSortVector)
{
	int packedCount = 0;
	// Packing of sorted by size images
	for (int i = 0; i < (int)tempSortVector.size(); ++i)
	{
		DefinitionFile * defFile = tempSortVector[i].defFile;
		int frame = tempSortVector[i].frameIndex;
		if (packer->AddImage(Size2i(defFile->frameRects[frame].dx, defFile->frameRects[frame].dy), &defFile->frameRects[frame]))
		{
			packedCount++;
			tempSortVector.erase(tempSortVector.begin() + i);
			i--;
		}
	}
	return packedCount;
}

float TexturePacker::TryToPackFromSortVectorWeight(ImagePacker * packer,Vector<SizeSortItem> & tempSortVector)
{
	float weight = 0.0f;
	
	// Packing of sorted by size images
	for (int i = 0; i < (int)tempSortVector.size(); ++i)
	{
		DefinitionFile * defFile = tempSortVector[i].defFile;
		int frame = tempSortVector[i].frameIndex;
		if (packer->AddImage(Size2i(defFile->frameRects[frame].dx, defFile->frameRects[frame].dy), &defFile->frameRects[frame]))
		{
			//float weightCoeff = (float)(tempSortVector.size() - i) / (float)(tempSortVector.size());
			weight += (defFile->frameRects[frame].dx * defFile->frameRects[frame].dy);// * weightCoeff;
			tempSortVector.erase(tempSortVector.begin() + i);
			i--;
		}
	}
	return weight;
}


bool sortFn(const SizeSortItem & a, const SizeSortItem & b)
{
	return a.imageSize > b.imageSize;	
}

void TexturePacker::PackToTexturesSeparate(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & defsList)
{
	lastPackedPacker = 0;
	int textureIndex = 0;
	for (List<DefinitionFile*>::iterator dfi = defsList.begin(); dfi != defsList.end(); ++dfi)
	{
		sortVector.clear();
		
		DefinitionFile * defFile = *dfi;
		for (int frame = 0; frame < defFile->frameCount; ++frame)
		{
			SizeSortItem sortItem;
			sortItem.imageSize = defFile->frameRects[frame].dx * defFile->frameRects[frame].dy;
			sortItem.defFile = defFile;
			sortItem.frameIndex = frame;
			sortVector.push_back(sortItem);
		}
		std::sort(sortVector.begin(), sortVector.end(), sortFn);

		
		// try to pack for each resolution
		int bestResolution = (maxTextureSize + 1) * (maxTextureSize + 1);
		int bestXResolution, bestYResolution;
		
		if (CommandLineParser::Instance()->GetVerbose())
			Logger::Info("* Packing tries started: ");
		
		for (int yResolution = 8; yResolution <= maxTextureSize; yResolution *= 2)
			for (int xResolution = 8; xResolution <= maxTextureSize; xResolution *= 2)
			{
				Rect2i textureRect = Rect2i(0, 0, xResolution, yResolution);
				
				if (xResolution * yResolution < bestResolution)
					if (TryToPack(textureRect, defsList))
					{
						bestResolution = xResolution * yResolution;
						bestXResolution = xResolution;
						bestYResolution = yResolution;
					}
			}
		if (CommandLineParser::Instance()->GetVerbose())
			Logger::Info("\n");
        
		if (bestResolution != (maxTextureSize + 1) * (maxTextureSize + 1))
		{
			char textureNameWithIndex[50];
			sprintf(textureNameWithIndex, "texture%d", textureIndex++);
			FilePath textureName = outputPath + textureNameWithIndex;
			if (CommandLineParser::Instance()->GetVerbose())
				Logger::Info("* Writing final texture (%d x %d): %s\n", bestXResolution, bestYResolution , textureName.GetAbsolutePathname().c_str());
			
			PngImageExt finalImage;
			finalImage.Create(bestXResolution, bestYResolution);
			
			// Writing 
			for (int frame = 0; frame < defFile->frameCount; ++frame)
			{
				Rect2i *destRect = lastPackedPacker->SearchRectForPtr(&defFile->frameRects[frame]);
				if (!destRect)Logger::Error("*** ERROR Can't find rect for frame\n");
				
				char name[256];
				FilePath withoutExt(defFile->filename);
                withoutExt.TruncateExtension();
                
				snprintf(name, 256, "%s%d.png", withoutExt.GetAbsolutePathname().c_str(), frame);
				
				PngImageExt image;
				image.Read(name);
				finalImage.DrawImage(destRect->x, destRect->y, &image);
			}
			
			if (!WriteDefinition(excludeFolder, outputPath, textureNameWithIndex, defFile))
			{
				Logger::Error("* ERROR: failed to write definition\n");
			}

            textureName.ReplaceExtension(".png");
            ExportImage(&finalImage, FilePath(textureName));
		}
	}
}

void TexturePacker::PackToTextures(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & defsList)
{
	lastPackedPacker = 0;
	for (List<DefinitionFile*>::iterator dfi = defsList.begin(); dfi != defsList.end(); ++dfi)
	{
		DefinitionFile * defFile = *dfi;
		for (int frame = 0; frame < defFile->frameCount; ++frame)
		{
			SizeSortItem sortItem;
			sortItem.imageSize = defFile->frameRects[frame].dx * defFile->frameRects[frame].dy;
			sortItem.defFile = defFile;
			sortItem.frameIndex = frame;
			sortVector.push_back(sortItem);
		}
	}

	std::sort(sortVector.begin(), sortVector.end(), sortFn);

	// try to pack for each resolution
	int bestResolution = (maxTextureSize + 1) * (maxTextureSize + 1);
	int bestXResolution, bestYResolution;
	
	if (CommandLineParser::Instance()->GetVerbose())
		Logger::Info("* Packing tries started: ");
	
	bool isPvr = CommandLineParser::Instance()->IsFlagSet("--pvr");
	bool isDxt = CommandLineParser::Instance()->IsFlagSet("--dxt");
	for (int yResolution = 8; yResolution <= maxTextureSize; yResolution *= 2)
		 for (int xResolution = 8; xResolution <= maxTextureSize; xResolution *= 2)
		 {
			 if ((isPvr || isDxt) && (xResolution != yResolution))continue;

			 if ((onlySquareTextures) && (xResolution != yResolution))continue;
			 
			 Rect2i textureRect = Rect2i(0, 0, xResolution, yResolution);
			 
			 if (xResolution * yResolution < bestResolution)
				 if (TryToPack(textureRect, defsList))
				 {
					 bestResolution = xResolution * yResolution;
					 bestXResolution = xResolution;
					 bestYResolution = yResolution;
				 }
		 }
	if (CommandLineParser::Instance()->GetVerbose())
		Logger::Info("\n");
	if (bestResolution != (maxTextureSize + 1) * (maxTextureSize + 1))
	{
		FilePath textureName = outputPath + "texture";
		if (CommandLineParser::Instance()->GetVerbose())
			Logger::Info("* Writing final texture (%d x %d): %s\n", bestXResolution, bestYResolution , textureName.GetAbsolutePathname().c_str());
	
		PngImageExt finalImage;
		finalImage.Create(bestXResolution, bestYResolution);
		
		// Writing 
		for (List<DefinitionFile*>::iterator dfi = defsList.begin(); dfi != defsList.end(); ++dfi)
		{
			DefinitionFile * defFile = *dfi;
			for (int frame = 0; frame < defFile->frameCount; ++frame)
			{
				Rect2i *destRect = lastPackedPacker->SearchRectForPtr(&defFile->frameRects[frame]);
				if (!destRect)Logger::Error("*** ERROR Can't find rect for frame\n");
				
				char name[256];
                FilePath withoutExt(defFile->filename);
                withoutExt.TruncateExtension();

				snprintf(name, 256, "%s%d.png", withoutExt.GetAbsolutePathname().c_str(), frame);

				PngImageExt image;
				image.Read(name);
				finalImage.DrawImage(destRect->x, destRect->y, &image);

				if (CommandLineParser::Instance()->IsFlagSet("--debug"))
				{
					finalImage.DrawRect(*destRect, 0xFF0000FF);
				}
			}
			
			if (!WriteDefinition(excludeFolder, outputPath, "texture", defFile))
			{
				Logger::Error("* ERROR: failed to write definition\n");
			}
		}

        textureName.ReplaceExtension(".png");
        ExportImage(&finalImage, textureName);
	}else
	{
		// 
		PackToMultipleTextures(excludeFolder, outputPath, defsList);
	}
}

void TexturePacker::PackToMultipleTextures(const FilePath & excludeFolder, const FilePath & outputPath, List<DefinitionFile*> & defList)
{
	if (defList.size() != 1)
		if (CommandLineParser::Instance()->GetVerbose())Logger::Error("* ERROR: failed to pack to multiple textures\n");

	for (int i = 0; i < (int)sortVector.size(); ++i)
	{
		DefinitionFile * defFile = sortVector[i].defFile;
		int frame = sortVector[i].frameIndex;
		if (CommandLineParser::Instance()->GetVerbose())
            Logger::Info("[MultiPack] prepack: %s frame: %d w:%d h:%d\n", defFile->filename.GetAbsolutePathname().c_str(), frame, defFile->frameRects[frame].dx, defFile->frameRects[frame].dy);
	}
	
	Vector<ImagePacker*> & packers = usedPackers;
	
	Vector<SizeSortItem> sortVectorWork = sortVector;
	
	while(sortVectorWork.size() > 0)
	{
		// try to pack for each resolution
		float maxValue = 0.0f;
		//int bestResolution = 1025 * 1025;
		
		if (CommandLineParser::Instance()->GetVerbose())Logger::Info("* Packing tries started: ");
		
		ImagePacker * bestPackerForThisStep = 0;
		Vector<SizeSortItem> newWorkVector;
		
        bool isPvr = CommandLineParser::Instance()->IsFlagSet("--pvr");
		bool isDxt = CommandLineParser::Instance()->IsFlagSet("--dxt");
		for (int yResolution = 8; yResolution <= maxTextureSize; yResolution *= 2)
			for (int xResolution = 8; xResolution <= maxTextureSize; xResolution *= 2)
			{
				if ( (isPvr || isDxt) && (xResolution != yResolution))continue;

				if ((onlySquareTextures) && (xResolution != yResolution))continue;
				
				Rect2i textureRect = Rect2i(0, 0, xResolution, yResolution);
				ImagePacker * packer = new ImagePacker(textureRect);
				
				Vector<SizeSortItem> tempSortVector = sortVectorWork;
				float n = TryToPackFromSortVectorWeight(packer, tempSortVector);
				
				if (n > maxValue) 
				{
					maxValue = n;
					SafeDelete(bestPackerForThisStep);
					bestPackerForThisStep = packer;
					newWorkVector = tempSortVector;
				}
			}
		
		sortVectorWork = newWorkVector;
		packers.push_back(bestPackerForThisStep);
	}
	
	if (CommandLineParser::Instance()->GetVerbose())Logger::Info("* Writing %d final textures \n", (int)packers.size());

	Vector<PngImageExt*> finalImages;
	
	for (int imageIndex = 0; imageIndex < (int)packers.size(); ++imageIndex)
	{
		PngImageExt * image = new PngImageExt();
		ImagePacker * packer = packers[imageIndex];
		image->Create(packer->GetRect().dx, packer->GetRect().dy);
		finalImages.push_back(image);
	}
	
	for (List<DefinitionFile*>::iterator defi = defList.begin(); defi != defList.end(); ++defi)
	{
		DefinitionFile * defFile = *defi;
		
		for (int frame = 0; frame < defFile->frameCount; ++frame)
		{
			Rect2i * destRect;
			ImagePacker * foundPacker = 0;
			int packerIndex = 0;
			char name[256];
			
			for (packerIndex = 0; packerIndex < (int)packers.size(); ++packerIndex)
			{
				destRect = packers[packerIndex]->SearchRectForPtr(&defFile->frameRects[frame]);
			
				if (destRect)
				{
					foundPacker = packers[packerIndex];
                    FilePath withoutExt(defFile->filename);
                    withoutExt.TruncateExtension();

					snprintf(name, 256, "%s%d.png", withoutExt.GetAbsolutePathname().c_str(), frame);
					break;
				}
			}
			
			if (foundPacker)
			{
				if (CommandLineParser::Instance()->GetVerbose())Logger::Info("[MultiPack] pack to texture: %d\n", packerIndex);
				PngImageExt image;
				image.Read(name);
				finalImages[packerIndex]->DrawImage(destRect->x, destRect->y, &image);
				if (CommandLineParser::Instance()->IsFlagSet("--debug"))
				{
					finalImages[packerIndex]->DrawRect(*destRect, 0xFF0000FF);
				}
			}
		}
	}
	
	for (int image = 0; image < (int)packers.size(); ++image)
	{
		char temp[256];
		sprintf(temp, "texture%d.png", image);
		FilePath textureName = outputPath + temp;
        ExportImage(finalImages[image], textureName);
	}

	for (List<DefinitionFile*>::iterator defi = defList.begin(); defi != defList.end(); ++defi)
	{
		DefinitionFile * defFile = *defi;
		
		FilePath textureName = outputPath + "texture";
		if (!WriteMultipleDefinition(excludeFolder, outputPath, "texture", defFile))
		{
			Logger::Error("* ERROR: failed to write definition\n");
		}
	}	
}


Rect2i TexturePacker::ReduceRectToOriginalSize(const Rect2i & _input)
{
	Rect2i r = _input;
	if (CommandLineParser::Instance()->IsFlagSet("--add0pixel"))
	{
	}
	else if (CommandLineParser::Instance()->IsFlagSet("--add1pixel"))
	{
		r.dx--;
		r.dy--;
	}
	else if (CommandLineParser::Instance()->IsFlagSet("--add2pixel"))
	{
		r.dx-=2;
		r.dy-=2;
	}
	else if (CommandLineParser::Instance()->IsFlagSet("--add4pixel"))
	{
		r.dx-=4;
		r.dy-=4;
	}
	else if (CommandLineParser::Instance()->IsFlagSet("--add2sidepixel"))
	{
		r.x+=1;
		r.y+=1;
		r.dx-=2;
		r.dy-=2;
	}
	else		// add 1 pixel by default
	{
		r.dx--;
		r.dy--;
	}
	return r;
}

bool TexturePacker::WriteDefinition(const FilePath & excludeFolder, const FilePath & outputPath, const String & _textureName, DefinitionFile * defFile)
{
	String fileName = defFile->filename.GetFilename();
	if (CommandLineParser::Instance()->GetVerbose())
		Logger::Info("* Write definition: %s\n", fileName.c_str());
	
	FilePath defFilePath = outputPath + fileName;
	FILE * fp = fopen(defFilePath.GetAbsolutePathname().c_str(), "wt");
	if (!fp)return false;
	
	fprintf(fp, "%d\n", 1);
	
	String textureExtension = TextureDescriptor::GetDescriptorExtension();
	fprintf(fp, "%s%s\n", _textureName.c_str(), textureExtension.c_str());
	
	fprintf(fp, "%d %d\n", defFile->spriteWidth, defFile->spriteHeight);
	fprintf(fp, "%d\n", defFile->frameCount); 
	for (int frame = 0; frame < defFile->frameCount; ++frame)
	{
		Rect2i *destRect = lastPackedPacker->SearchRectForPtr(&defFile->frameRects[frame]);
		Rect2i origRect = defFile->frameRects[frame];
		Rect2i writeRect = ReduceRectToOriginalSize(*destRect);
		fprintf(fp, "%d %d %d %d %d %d %d\n", writeRect.x, writeRect.y, writeRect.dx, writeRect.dy, origRect.x, origRect.y, 0);
	}
	
	for (int pathInfoLine = 0; pathInfoLine < (int)defFile->pathsInfo.size(); ++pathInfoLine)
	{
		String & line = defFile->pathsInfo[pathInfoLine];
		fprintf(fp, "%s", line.c_str());
	}
	
	fclose(fp);
	return true;
}

bool TexturePacker::WriteMultipleDefinition(const FilePath & excludeFolder, const FilePath & outputPath, const String & _textureName, DefinitionFile * defFile)
{
	String fileName = defFile->filename.GetFilename();
	if (CommandLineParser::Instance()->GetVerbose())
		Logger::Info("* Write definition: %s\n", fileName.c_str());
	
	FilePath defFilePath = outputPath + fileName;
	FILE * fp = fopen(defFilePath.GetAbsolutePathname().c_str(), "wt");
	if (!fp)return false;
	
	String textureExtension = TextureDescriptor::GetDescriptorExtension();
	
	Vector<int> packerIndexArray;
	packerIndexArray.resize(defFile->frameCount);
	
	Map<int, int> packerIndexToFileIndex;
	
	// find used texture indexes for this sprite
	for (int frame = 0; frame < defFile->frameCount; ++frame)
	{
		Rect2i * destRect = 0;
		int packerIndex = 0;
		for (packerIndex = 0; packerIndex < (int)usedPackers.size(); ++packerIndex)
		{
			destRect = usedPackers[packerIndex]->SearchRectForPtr(&defFile->frameRects[frame]);
			if (destRect)break;
		}
		// save packer index for frame
		packerIndexArray[frame] = packerIndex;
		// add value to map to show that this packerIndex was used
		packerIndexToFileIndex[packerIndex] = -1;
	}
		
	// write real used packers count
	fprintf(fp, "%d\n", (int)packerIndexToFileIndex.size());
	
	int realIndex = 0;
	// write user texture indexes
	for (int i = 0; i < (int)usedPackers.size(); ++i)
	{
		Map<int, int>::iterator isUsed = packerIndexToFileIndex.find(i);
		if (isUsed != packerIndexToFileIndex.end())
		{
			// here we write filename for i-th texture and write to map real index in file for this texture
			fprintf(fp, "%s%d%s\n", _textureName.c_str(), i, textureExtension.c_str());
			packerIndexToFileIndex[i] = realIndex++;
		}
	}
	
	fprintf(fp, "%d %d\n", defFile->spriteWidth, defFile->spriteHeight);
	fprintf(fp, "%d\n", defFile->frameCount); 
	for (int frame = 0; frame < defFile->frameCount; ++frame)
	{
		Rect2i * destRect = 0;
		for (int packerIndex = 0; packerIndex < (int)usedPackers.size(); ++packerIndex)
		{
			destRect = usedPackers[packerIndex]->SearchRectForPtr(&defFile->frameRects[frame]);
			if (destRect)break;
		}
		int packerIndex = packerIndexToFileIndex[packerIndexArray[frame]]; // here get real index in file for our used texture
		if (destRect)
		{
			Rect2i origRect = defFile->frameRects[frame];
			Rect2i writeRect = ReduceRectToOriginalSize(*destRect);
			fprintf(fp, "%d %d %d %d %d %d %d\n", writeRect.x, writeRect.y, writeRect.dx, writeRect.dy, origRect.x, origRect.y, packerIndex);
		}else
		{
			Logger::Error("*** FATAL ERROR: can't find rect in all of packers");
		}
	}
	
	for (int pathInfoLine = 0; pathInfoLine < (int)defFile->pathsInfo.size(); ++pathInfoLine)
	{
		String & line = defFile->pathsInfo[pathInfoLine];
		fprintf(fp, "%s", line.c_str());
	}
	
	fclose(fp);
	return true;
}

void TexturePacker::UseOnlySquareTextures()
{
	onlySquareTextures = true;
}

void TexturePacker::SetMaxTextureSize(int32 _maxTextureSize)
{
	maxTextureSize = _maxTextureSize;
}

void TexturePacker::ExportImage(PngImageExt *image, const FilePath &exportedPathname)
{
    TextureDescriptor *descriptor = CreateDescriptor();

    image->DitherAlpha();
    image->Write(exportedPathname);
    
    if (NULL != descriptor  )
    {
		if(FORMAT_INVALID != descriptor->pvrCompression.format)
		{
			PVRConverter::Instance()->ConvertPngToPvr(exportedPathname, *descriptor);
			FileSystem::Instance()->DeleteFile(exportedPathname);
		}
		else if(FORMAT_INVALID != descriptor->dxtCompression.format)
		{
			DXTConverter::ConvertPngToDxt(exportedPathname, *descriptor);
			FileSystem::Instance()->DeleteFile(exportedPathname);
		}
        
    }

    
#if defined TEXTURE_SPLICING_ENABLED
    //TODO: need to enable texture splicing of 2D resources
    //        descriptor->ExportAndSlice(descriptorPathname, texturePathname);
#else //#if defined TEXTURE_SPLICING_ENABLED
    descriptor->Export(TextureDescriptor::GetDescriptorPathname(exportedPathname));
#endif //#if defined TEXTURE_SPLICING_ENABLED
    
    SafeRelease(descriptor);
}


TextureDescriptor * TexturePacker::CreateDescriptor()
{
    TextureDescriptor *descriptor = new TextureDescriptor();
    if(descriptor)
    {
        descriptor->wrapModeS = descriptor->wrapModeT = Texture::WRAP_CLAMP_TO_EDGE;
        descriptor->generateMipMaps = CommandLineParser::Instance()->IsFlagSet(String("--generateMipMaps"));
        if(descriptor->generateMipMaps)
        {
            descriptor->minFilter = Texture::FILTER_LINEAR_MIPMAP_LINEAR;
            descriptor->magFilter = Texture::FILTER_LINEAR;
        }
        else
        {
            descriptor->minFilter = Texture::FILTER_LINEAR;
            descriptor->magFilter = Texture::FILTER_LINEAR;
        }
        
        if(CommandLineParser::Instance()->IsFlagSet("--pvr"))
        {
            descriptor->textureFileFormat = PVR_FILE;

            descriptor->pvrCompression.format = DetectPixelFormatFromFlags();
        }
        else if(CommandLineParser::Instance()->IsFlagSet("--dxt"))
        {
            descriptor->textureFileFormat = DXT_FILE;
            
            descriptor->dxtCompression.format = DetectPixelFormatFromFlags();
        }
        else
        {
            descriptor->textureFileFormat = PNG_FILE;
        }
    }
    
    return descriptor;
}

PixelFormat TexturePacker::DetectPixelFormatFromFlags()
{
    for(int32 i = FORMAT_INVALID; i < FORMAT_COUNT; ++i)
    {
        PixelFormat format = (PixelFormat)i;
        String formatFlag = String("--") + String(Texture::GetPixelFormatString(format));
        if(CommandLineParser::Instance()->IsFlagSet(formatFlag))
        {
            return format;
        }
    }
    
    return FORMAT_INVALID;
}

};
