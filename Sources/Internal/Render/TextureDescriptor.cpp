
/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Render/TextureDescriptor.h"
#include "FileSystem/Logger.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "Render/Texture.h"
#include "Utils/Utils.h"

namespace DAVA
{
    
void TextureDescriptor::Compression::Clear()
{
    format = FORMAT_INVALID;
	crc = 0;

    compressToWidth = 0;
    compressToHeight = 0;
}
    
    
TextureDescriptor::TextureDescriptor()
{
    isCompressedFile = false;
    InitializeValues();
}

TextureDescriptor::~TextureDescriptor()
{
}

TextureDescriptor *TextureDescriptor::CreateFromFile(const FilePath &filePathname)
{
    FilePath descriptorPathname = GetDescriptorPathname(filePathname);
    TextureDescriptor *descriptor = new TextureDescriptor();
    bool loaded = descriptor->Load(descriptorPathname);
    if(!loaded)
    {
        Logger::Error("[TextureDescriptor::CreateFromFile(]: there are no descriptor file (%s).", descriptorPathname.GetAbsolutePathname().c_str());
        SafeRelease(descriptor);
        return NULL;
    }
    
    return descriptor;
}
    
    
void TextureDescriptor::InitializeValues()
{
    SetDefaultValues();
    
    pvrCompression.Clear();
    dxtCompression.Clear();
    
    textureFileFormat = PNG_FILE;
}
    
void TextureDescriptor::SetDefaultValues()
{
    wrapModeS = Texture::WRAP_REPEAT;
    wrapModeT = Texture::WRAP_REPEAT;
    
    generateMipMaps = OPTION_ENABLED;
    
    minFilter = Texture::FILTER_LINEAR_MIPMAP_LINEAR;
    magFilter = Texture::FILTER_LINEAR;
}
    
bool TextureDescriptor::UpdateCrcForFormat(ImageFileFormat fileFormat) const
{
    bool wasUpdated = false;
    const Compression *compression = GetCompressionParams(fileFormat);

	uint32 sourceCRC = ReadSourceCRC();
	if(compression->crc != sourceCRC)
	{
		compression->crc = sourceCRC;
		wasUpdated = true;
	}
    
    return wasUpdated;
}
    
bool TextureDescriptor::Load(const FilePath &filePathname)
{
    File *file = File::Create(filePathname, File::READ | File::OPEN);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::Load] Can't open file: %s", filePathname.GetAbsolutePathname().c_str());
        return false;
    }
    
    pathname = filePathname;

    int32 signature;
    file->Read(&signature, sizeof(signature));

    int8 version = 0;
    file->Read(&version, sizeof(version));
    if(version != CURRENT_VERSION)
    {
        ConvertToCurrentVersion(version, signature, file);
        SafeRelease(file);
        return true;
    }
    
    isCompressedFile = (COMPRESSED_FILE == signature);
    if(isCompressedFile)
    {
        LoadCompressed(file);
    }
    else if(NOTCOMPRESSED_FILE == signature)
    {
        LoadNotCompressed(file);
    }
    else
    {
        Logger::Error("[TextureDescriptor::Load] Wrong descriptor file: %s", filePathname.GetAbsolutePathname().c_str());
        SafeRelease(file);
        return false;
    }
    
    SafeRelease(file);
    
    return true;
}

void TextureDescriptor::Save() const
{
    DVASSERT_MSG(!pathname.IsEmpty(), "Can use this method only after calling Load()");
    Save(pathname);
}
    
void TextureDescriptor::Save(const FilePath &filePathname) const
{
    File *file = File::Create(filePathname, File::WRITE | File::OPEN | File::CREATE);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::Save] Can't open file: %s", filePathname.GetAbsolutePathname().c_str());
        return;
    }
    
    int32 signature = NOTCOMPRESSED_FILE;
    file->Write(&signature, sizeof(signature));
    
    int8 version = CURRENT_VERSION;
    file->Write(&version, sizeof(version));
    
    WriteGeneralSettings(file);
    
    //Compression
    WriteCompression(file, pvrCompression);
    WriteCompression(file, dxtCompression);
    
    SafeRelease(file);
}
  
void TextureDescriptor::Export(const FilePath &filePathname)
{
    File *file = File::Create(filePathname, File::WRITE | File::OPEN | File::CREATE);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::ExportAndSplice] Can't open file: %s", filePathname.GetAbsolutePathname().c_str());
        return;
    }

    int32 signature = COMPRESSED_FILE;
    file->Write(&signature, sizeof(signature));
    
    int8 version = CURRENT_VERSION;
    file->Write(&version, sizeof(version));

    WriteGeneralSettings(file);
    file->Write(&textureFileFormat, sizeof(textureFileFormat));

    SafeRelease(file);
}
    
void TextureDescriptor::ConvertToCurrentVersion(int8 version, int32 signature, DAVA::File *file)
{
//    Logger::Info("[TextureDescriptor::ConvertToCurrentVersion] (%s) from version %d", pathname.c_str(), version);
    
    if(version == 2)
    {
        LoadVersion2(signature, file);
    }
	if(version == 3)
	{
		LoadVersion3(signature, file);
	}
	if(version == 4)
	{
		LoadVersion4(signature, file);
	}
}
    
void TextureDescriptor::LoadVersion2(int32 signature, DAVA::File *file)
{
    if(signature == COMPRESSED_FILE)
    {
        file->Read(&wrapModeS, sizeof(wrapModeS));
        file->Read(&wrapModeT, sizeof(wrapModeT));
        file->Read(&generateMipMaps, sizeof(generateMipMaps));
        file->Read(&textureFileFormat, sizeof(textureFileFormat));
    }
    else if(signature == NOTCOMPRESSED_FILE)
    {
        file->Read(&wrapModeS, sizeof(wrapModeS));
        file->Read(&wrapModeT, sizeof(wrapModeT));
        file->Read(&generateMipMaps, sizeof(generateMipMaps));
        
        ReadCompressionWithDateOld(file, pvrCompression);
        ReadCompressionWithDateOld(file, dxtCompression);
    }
}
 
void TextureDescriptor::LoadVersion3(int32 signature, DAVA::File *file)
{
	if(signature == COMPRESSED_FILE)
	{
		file->Read(&wrapModeS, sizeof(wrapModeS));
		file->Read(&wrapModeT, sizeof(wrapModeT));
		file->Read(&generateMipMaps, sizeof(generateMipMaps));
		file->Read(&minFilter, sizeof(minFilter));
		file->Read(&magFilter, sizeof(magFilter));
		file->Read(&textureFileFormat, sizeof(textureFileFormat));
	}
	else if(signature == NOTCOMPRESSED_FILE)
	{
		file->Read(&wrapModeS, sizeof(wrapModeS));
		file->Read(&wrapModeT, sizeof(wrapModeT));
		file->Read(&generateMipMaps, sizeof(generateMipMaps));
		file->Read(&minFilter, sizeof(minFilter));
		file->Read(&magFilter, sizeof(magFilter));

		ReadCompressionWithDateOld(file, pvrCompression);
		ReadCompressionWithDateOld(file, dxtCompression);
	}
}

void TextureDescriptor::LoadVersion4(int32 signature, DAVA::File *file)
{
	if(signature == COMPRESSED_FILE)
	{
		file->Read(&wrapModeS, sizeof(wrapModeS));
		file->Read(&wrapModeT, sizeof(wrapModeT));
		file->Read(&generateMipMaps, sizeof(generateMipMaps));
		file->Read(&minFilter, sizeof(minFilter));
		file->Read(&magFilter, sizeof(magFilter));
		file->Read(&textureFileFormat, sizeof(textureFileFormat));
	}
	else if(signature == NOTCOMPRESSED_FILE)
	{
		file->Read(&wrapModeS, sizeof(wrapModeS));
		file->Read(&wrapModeT, sizeof(wrapModeT));
		file->Read(&generateMipMaps, sizeof(generateMipMaps));
		file->Read(&minFilter, sizeof(minFilter));
		file->Read(&magFilter, sizeof(magFilter));

		ReadCompressionWith16CRCOld(file, pvrCompression);
		ReadCompressionWith16CRCOld(file, dxtCompression);
	}
}


void TextureDescriptor::LoadNotCompressed(File *file)
{
    ReadGeneralSettings(file);
    
    ReadCompression(file, pvrCompression);
    ReadCompression(file, dxtCompression);
}
    
void TextureDescriptor::LoadCompressed(File *file)
{
    ReadGeneralSettings(file);
    file->Read(&textureFileFormat, sizeof(textureFileFormat));
}

void TextureDescriptor::ReadGeneralSettings(File *file)
{
    file->Read(&wrapModeS, sizeof(wrapModeS));
    file->Read(&wrapModeT, sizeof(wrapModeT));
    file->Read(&generateMipMaps, sizeof(generateMipMaps));
    file->Read(&minFilter, sizeof(minFilter));
    file->Read(&magFilter, sizeof(magFilter));
}
    
void TextureDescriptor::WriteGeneralSettings(File *file) const
{
    file->Write(&wrapModeS, sizeof(wrapModeS));
    file->Write(&wrapModeT, sizeof(wrapModeT));
    file->Write(&generateMipMaps, sizeof(generateMipMaps));
    file->Write(&minFilter, sizeof(minFilter));
    file->Write(&magFilter, sizeof(magFilter));
}

    
void TextureDescriptor::ReadCompression(File *file, Compression &compression)
{
    int8 format;
    file->Read(&format, sizeof(format));
    compression.format = (PixelFormat)format;

    file->Read(&compression.compressToWidth, sizeof(compression.compressToWidth));
    file->Read(&compression.compressToHeight, sizeof(compression.compressToHeight));
	file->Read(&compression.crc, sizeof(compression.crc));
}

void TextureDescriptor::ReadCompressionWithDateOld( File *file, Compression &compression )
{
	int8 format;
	file->Read(&format, sizeof(format));
	compression.format = (PixelFormat)format;

	file->Read(&compression.compressToWidth, sizeof(compression.compressToWidth));
	file->Read(&compression.compressToHeight, sizeof(compression.compressToHeight));

	// skip modification date
	file->Seek(DATE_BUFFER_SIZE * sizeof(char8), File::SEEK_FROM_CURRENT);

	// skip old crc
	file->Seek((MD5::DIGEST_SIZE*2 + 1) * sizeof(char8), File::SEEK_FROM_CURRENT);
	compression.crc = 0;
}

void TextureDescriptor::ReadCompressionWith16CRCOld( File *file, Compression &compression )
{
	int8 format;
	file->Read(&format, sizeof(format));
	compression.format = (PixelFormat)format;

	file->Read(&compression.compressToWidth, sizeof(compression.compressToWidth));
	file->Read(&compression.compressToHeight, sizeof(compression.compressToHeight));

	// skip old crc
	file->Seek((MD5::DIGEST_SIZE*2 + 1) * sizeof(char8), File::SEEK_FROM_CURRENT);
	compression.crc = 0;
}

void TextureDescriptor::WriteCompression(File *file, const Compression &compression) const
{
    int8 format = compression.format;
    file->Write(&format, sizeof(format));
    file->Write(&compression.compressToWidth, sizeof(compression.compressToWidth));
    file->Write(&compression.compressToHeight, sizeof(compression.compressToHeight));
	file->Write(&compression.crc, sizeof(compression.crc));
}

bool TextureDescriptor::GetGenerateMipMaps() const
{
    return (OPTION_DISABLED != generateMipMaps);
}
    
    
FilePath TextureDescriptor::GetSourceTexturePathname() const
{
    if(pathname.IsEmpty())
    {
        return FilePath();
    }

    return FilePath::CreateWithNewExtension(pathname, GetSourceTextureExtension());
}

FilePath TextureDescriptor::GetDescriptorPathname(const FilePath &texturePathname)
{
    if(texturePathname.IsEmpty())
        return FilePath();
    
    return FilePath::CreateWithNewExtension(texturePathname, GetDescriptorExtension());
}


String TextureDescriptor::GetDescriptorExtension()
{
    return String(".tex");
}
    
String TextureDescriptor::GetSourceTextureExtension()
{
    return String(".png");
}
    
bool TextureDescriptor::IsSourceChanged(ImageFileFormat fileFormat)
{
    const Compression *compression = GetCompressionParams(fileFormat);
	uint32 sourceCRC = ReadSourceCRC();

	return (compression->crc != sourceCRC);
}
    
const TextureDescriptor::Compression * TextureDescriptor::GetCompressionParams(ImageFileFormat fileFormat) const
{
    DVASSERT((fileFormat == PVR_FILE) || (fileFormat == DXT_FILE));
    
    if(fileFormat == PVR_FILE)
    {
        return &pvrCompression;
    }
    else if(fileFormat == DXT_FILE)
    {
        return &dxtCompression;
    }
    
    return NULL;
}

String TextureDescriptor::GetSupportedTextureExtensions()
{
    return String(".png;.pvr;") + TextureDescriptor::GetDescriptorExtension();
}

FilePath TextureDescriptor::GetPathnameForFormat(const FilePath &pathname, ImageFileFormat fileFormat)
{
    if(fileFormat == PNG_FILE)
    {
        return FilePath::CreateWithNewExtension(pathname, ".png");
    }
    else if(fileFormat == PVR_FILE)
    {
        return FilePath::CreateWithNewExtension(pathname, ".pvr");
    }
    else if(fileFormat == DXT_FILE)
    {
        return FilePath::CreateWithNewExtension(pathname, ".dds");
    }

    return FilePath();
}
    
ImageFileFormat TextureDescriptor::GetFormatForPathname(const FilePath &pathname)
{
    return GetFormatForExtension(pathname.GetExtension());
}
    

ImageFileFormat TextureDescriptor::GetFormatForExtension(const String &extension)
{
    if(0 == CompareCaseInsensitive(extension, ".png"))
    {
        return PNG_FILE;
    }
    else if(0 == CompareCaseInsensitive(extension, ".pvr"))
    {
        return PVR_FILE;
    }
    else if(0 == CompareCaseInsensitive(extension, ".dds"))
    {
        return DXT_FILE;
    }

    return NOT_FILE;
}

const bool TextureDescriptor::IsCompressedFile() const
{
    return isCompressedFile;
}

uint32 TextureDescriptor::ReadSourceCRC() const
{
	uint32 crc = 0;

	DAVA::File *f = DAVA::File::Create(GetSourceTexturePathname(), DAVA::File::OPEN | DAVA::File::READ);
	if(NULL != f)
	{
		uint8 buffer[8];

		// Read PNG header
		f->Read(buffer, 8);

		// read chunk header
		while (0 != f->Read(buffer, 8))
		{
			int32 chunk_size = 0;
			chunk_size |= (buffer[0] << 24);
			chunk_size |= (buffer[1] << 16);
			chunk_size |= (buffer[2] << 8);
			chunk_size |= buffer[3];

			// jump thought data
			DVASSERT(chunk_size >= 0);
			f->Seek(chunk_size, File::SEEK_FROM_CURRENT);

			// read crc
			f->Read(buffer, 4);
			crc += ((uint32 *) buffer)[0];
		}

		f->Release();
	}

	return crc;
}

};
