#include "DXTConverter.h"

#include "FileSystem/FilePath.h"
#include "Render/TextureDescriptor.h"
#include "Render/Image.h"
#include "Render/ImageLoader.h"
#include "Render/LibDxtHelper.h"
#include "Render/GPUFamilyDescriptor.h"

namespace DAVA
{
    
FilePath DXTConverter::ConvertPngToDxt(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
    FilePath fileToConvert = FilePath::CreateWithNewExtension(descriptor.pathname, ".png");
    
    Vector<Image*> inputImages = ImageLoader::CreateFromFile(fileToConvert);
    if(inputImages.size() == 1)
    {
        Image* image = inputImages[0];
        
        FilePath outputName = GetDXTOutput(descriptor, gpuFamily);
        
        if((descriptor.compression[gpuFamily].compressToWidth != 0) && (descriptor.compression[gpuFamily].compressToHeight != 0))
        {
            Logger::Warning("[DXTConverter::ConvertPngToDxt] convert to compression size");
            image->ResizeImage(descriptor.compression[gpuFamily].compressToWidth, descriptor.compression[gpuFamily].compressToHeight);
        }
        
        if(LibDxtHelper::WriteDdsFile(outputName,
                                      image->width, image->height, image->data,
                                      (PixelFormat) descriptor.compression[gpuFamily].format,
                                      (descriptor.settings.generateMipMaps == TextureDescriptor::OPTION_ENABLED)))
        {
            for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
            return outputName;
        }
    }
    
    Logger::Error("[DXTConverter::ConvertPngToDxt] can't convert %s to DXT", fileToConvert.GetAbsolutePathname().c_str());
    
    for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
    return FilePath();
}

FilePath DXTConverter::GetDXTOutput(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
    return GPUFamilyDescriptor::CreatePathnameForGPU(&descriptor, gpuFamily);
}

};

