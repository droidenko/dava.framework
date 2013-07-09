#ifndef __DAVAENGINE_PVR_CONVERTER_H__
#define __DAVAENGINE_PVR_CONVERTER_H__

#include "Base/StaticSingleton.h"
#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

class TextureDescriptor;
class PVRConverter: public StaticSingleton<PVRConverter>
{    
public:
 
	PVRConverter();
	virtual ~PVRConverter();

	FilePath ConvertPngToPvr(const TextureDescriptor &descriptor, eGPUFamily gpuFamily);

	String GetCommandLinePVR(const TextureDescriptor &descriptor, eGPUFamily gpuFamily);

	void SetPVRTexTool(const FilePath &textToolPathname);

	FilePath GetPVRToolOutput(const TextureDescriptor &descriptor, eGPUFamily gpuFamily);

protected:
	
    Map<PixelFormat, String> pixelFormatToPVRFormat;

	FilePath pvrTexToolPathname;
};

};


#endif // __DAVAENGINE_PVR_CONVERTER_H__