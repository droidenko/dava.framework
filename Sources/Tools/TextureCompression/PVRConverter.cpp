#include "PVRConverter.h"
#include "Render/TextureDescriptor.h"
#include "FileSystem/FileSystem.h"
#include "Utils/StringFormat.h"

#include "Render/GPUFamilyDescriptor.h"

namespace DAVA
{


PVRConverter::PVRConverter()
{
	// pvr map
	pixelFormatToPVRFormat[FORMAT_RGBA8888] = "OGL8888";
	pixelFormatToPVRFormat[FORMAT_RGBA4444] = "OGL4444";
	pixelFormatToPVRFormat[FORMAT_RGBA5551] = "OGL5551";
	pixelFormatToPVRFormat[FORMAT_RGB565] = "OGL565";
	pixelFormatToPVRFormat[FORMAT_RGB888] = "OGL888";
	pixelFormatToPVRFormat[FORMAT_PVR2] = "OGLPVRTC2";
	pixelFormatToPVRFormat[FORMAT_PVR4] = "OGLPVRTC4";
	pixelFormatToPVRFormat[FORMAT_A8] = "OGL8";
	pixelFormatToPVRFormat[FORMAT_ETC1] = "ETC";
}

PVRConverter::~PVRConverter()
{

}

FilePath PVRConverter::ConvertPngToPvr(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
	FilePath outputName;
	String command = GetCommandLinePVR(descriptor, gpuFamily);
    Logger::Info("[PVRConverter::ConvertPngToPvr] (%s)", command.c_str());
    
	if(!command.empty())
	{
		FileSystem::Instance()->Spawn("\"" + command + "\"");
		outputName = GetPVRToolOutput(descriptor, gpuFamily);
	}

	return outputName;
}

String PVRConverter::GetCommandLinePVR(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
	String command = pvrTexToolPathname.GetAbsolutePathname();
	String format = pixelFormatToPVRFormat[(PixelFormat) descriptor.compression[gpuFamily].format];

	if(command != "" && format != "")
	{
        FilePath fileToConvert = FilePath::CreateWithNewExtension(descriptor.pathname, ".png");
		FilePath outputFile = GetPVRToolOutput(descriptor, gpuFamily);

		// assemble command

		// input file
		command += Format(" -i \"%s\"", fileToConvert.GetAbsolutePathname().c_str());

		// output format
		command += Format(" -f%s", format.c_str());

		// pvr should be always flipped-y
		command += " -yflip0";

		// mipmaps
		if(descriptor.settings.generateMipMaps)
		{
			command += " -m";
		}

		// base mipmap level (base resize)
		if(0 != descriptor.compression[gpuFamily].compressToWidth && descriptor.compression[gpuFamily].compressToHeight != 0)
		{
			command += Format(" -x %d -y %d", descriptor.compression[gpuFamily].compressToWidth, descriptor.compression[gpuFamily].compressToHeight);
		}

		// output file
		command += Format(" -o \"%s\"", outputFile.GetAbsolutePathname().c_str());
	}
    else
    {
        Logger::Error("[PVRConverter::GetCommandLinePVR] Can't create command line for file with descriptor (%s)", descriptor.pathname.GetAbsolutePathname().c_str());
        command = "";
    }

	return command;
}

FilePath PVRConverter::GetPVRToolOutput(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
    return GPUFamilyDescriptor::CreatePathnameForGPU(&descriptor, gpuFamily);
}

void PVRConverter::SetPVRTexTool(const FilePath &textToolPathname)
{
	pvrTexToolPathname = textToolPathname;

	if(!FileSystem::Instance()->IsFile(pvrTexToolPathname))
	{
		Logger::Error("PVRTexTool doesn't found in %s\n", pvrTexToolPathname.GetAbsolutePathname().c_str());
		pvrTexToolPathname = FilePath();
	}
}

};

