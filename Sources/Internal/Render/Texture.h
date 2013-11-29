/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_TEXTURE_H__
#define __DAVAENGINE_TEXTURE_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Render/RenderResource.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
/**
	\ingroup render
	\brief Class that represents texture objects in our SDK. 
	This class support the following formats: RGBA8888, RGB565, RGBA4444, A8 on all platforms. 
	For iOS it also support compressed PVR formats. (PVR2 and PVR4)
 */
class Image;
class TextureDescriptor;
class File;
class Texture;
	
class TextureInvalidater
{
public:
	virtual void InvalidateTexture(Texture * texure) = 0;
};
	
class Texture : public RenderResource
{
public:
    
    enum TextureWrap
	{
		WRAP_CLAMP_TO_EDGE = 0,
		WRAP_REPEAT,
	};

    enum TextureFilter
	{
        FILTER_NEAREST  = 0,
        FILTER_LINEAR,

        FILTER_NEAREST_MIPMAP_NEAREST,
        FILTER_LINEAR_MIPMAP_NEAREST,
        FILTER_NEAREST_MIPMAP_LINEAR,
        FILTER_LINEAR_MIPMAP_LINEAR
	};

	enum DepthFormat
	{
		DEPTH_NONE = 0,
		DEPTH_RENDERBUFFER
	};
	
	//VI: each face is optional
	enum CubemapFace
	{
		CUBE_FACE_POSITIVE_X = 0,
		CUBE_FACE_NEGATIVE_X = 1,
		CUBE_FACE_POSITIVE_Y = 2,
		CUBE_FACE_NEGATIVE_Y = 3,
		CUBE_FACE_POSITIVE_Z = 4,
		CUBE_FACE_NEGATIVE_Z = 5,
		CUBE_FACE_MAX_COUNT = 6,
		CUBE_FACE_INVALID = 0xFFFFFFFF
	};
	
	enum TextureType
	{
		TEXTURE_2D = 0,
		TEXTURE_CUBE = 1,
		TEXTURE_TYPE_COUNT = 2
	};

	enum TextureState
	{
		STATE_INVALID	=	0,
		STATE_DATA_LOADED,
		STATE_VALID
	};
	
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	static const int MAX_WIDTH = 1024;
	static const int MIN_WIDTH = 8;
	static const int MAX_HEIGHT = 1024;
	static const int MIN_HEIGHT = 8;
#else //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	static const int MAX_WIDTH = 4096;
	static const int MIN_WIDTH = 8;
	static const int MAX_HEIGHT = 4096;
	static const int MIN_HEIGHT = 8;
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)

	// Main constructors
	
    static void InitializePixelFormatDescriptors();

    
	/**
        \brief Return size of pixel format in bits 
        \returns size in bits, for example for FORMAT_RGBA8888 function will return 32.
     */
	static int32 GetPixelFormatSizeInBytes(PixelFormat format);
	static int32 GetPixelFormatSizeInBits(PixelFormat format);
	/**
        \brief Return string representation of pixel format
        \returns string value describing pixel format
     */
    static const char * GetPixelFormatString(PixelFormat format);
    static PixelFormat GetPixelFormatByName(const String &formatName);
    
    /**
        \brief Create texture from data arrray
        This function creates texture from given format, data pointer and width + height
     
        \param[in] format desired pixel format
        \param[in] data desired data 
        \param[in] width width of new texture
        \param[in] height height of new texture
     */
	static Texture * CreateFromData(PixelFormat format, const uint8 *data, uint32 width, uint32 height, bool generateMipMaps);

    /**
        \brief Create text texture from data arrray
        This function creates texture from given format, data pointer and width + height, but adds addInfo string to relativePathname variable for easy identification of textures
        
        \param[in] format desired pixel format
        \param[in] data desired data 
        \param[in] width width of new texture
        \param[in] height height of new texture
        \param[in] addInfo additional info
     */
	static Texture * CreateTextFromData(PixelFormat format, uint8 * data, uint32 width, uint32 height, bool generateMipMaps, const char * addInfo = 0);
    
	/**
        \brief Create texture from given file. Supported formats .png, .pvr (only on iOS). 
		If file cannot be opened, returns "pink placeholder" texture.
        \param[in] pathName path to the png or pvr file
     */
	static Texture * CreateFromFile(const FilePath & pathName, TextureType typeHint = Texture::TEXTURE_2D);

	/**
        \brief Create texture from given file. Supported formats .png, .pvr (only on iOS). 
		If file cannot be opened, returns 0
        \param[in] pathName path to the png or pvr file
     */
	static Texture * PureCreate(const FilePath & pathName);
    
	/**
        \brief Create FBO from given width, height and format
        \param[in] width width of the fbo
        \param[in] height height of the fbo
        \param[in] format format of the fbo
		\param[in] useDepthbuffer if set to true, addition depthbuffer will be created for this fbo
        \todo reorder variables in function, and make format variable first to make it similar to CreateFromData function.
     */
	static Texture * CreateFBO(uint32 width, uint32 height, PixelFormat format, DepthFormat depthFormat);
	
	static Texture * CreatePink(TextureType requestedType = Texture::TEXTURE_2D);


	/**
        \brief Sets default RGBA format that is used for textures loaded from files. 
        This functino define which format is used by default when you are loading files from disk. 
        By default it's RGBA8888 format. But for example if you want to load something in RGBA4444 format you 
        can use the following code
     
        \code
        Texture::SetDefaultRGBAFormat(FORMAT_RGBA4444);
        texRGBA4444 = Texture::CreateFromFile("~res:/Scenes/Textures/texture.png");
        Texture::SetDefaultRGBAFormat(FORMAT_RGBA8888);
        \endcode
     */
	static void SetDefaultRGBAFormat(PixelFormat format);
	static PixelFormat GetDefaultRGBAFormat();
	
	virtual int32 Release();

	static void	DumpTextures();

	inline int32 GetWidth() const { return width; }
	inline int32 GetHeight() const { return height; }
	
	void GenerateMipmaps();
	void GeneratePixelesation();
	
	void TexImage(int32 level, uint32 width, uint32 height, const void * _data, uint32 dataSize, uint32 cubeFaceId);
    
	void SetWrapMode(TextureWrap wrapS, TextureWrap wrapT);
	
    /**
        \brief This function can enable / disable autosave for render targets.
        It's actual only for DX9 and for other systems is does nothing
        If you refreshing your rendertargets every frame you can disable autosave for them for performance on DX9
        By default autosave is enabled for all DX9 textures. 
     */
    inline void EnableRenderTargetAutosave(bool isEnabled);
    
    
    /**
        \brief Function to receive pathname of texture object
        \returns pathname of texture
     */
    inline const FilePath & GetPathname() const;
    
    Image * CreateImageFromMemory();

	bool IsPinkPlaceholder();
    
    static PixelFormatDescriptor GetPixelFormatDescriptor(PixelFormat formatID);
	
	static void GenerateCubeFaceNames(const String& baseName, Vector<String>& faceNames);
	static void GenerateCubeFaceNames(const String& baseName, const Vector<String>& faceNameSuffixes, Vector<String>& faceNames);

    void Reload();
    void ReloadAs(eGPUFamily gpuFamily);
	void SetInvalidater(TextureInvalidater* invalidater);

	inline TextureState GetState() const;

public:							// properties for fast access

#if defined(__DAVAENGINE_OPENGL__)
	uint32		id;				// OpenGL id for texture

#if defined(__DAVAENGINE_ANDROID__)
	virtual void Lost();
	virtual void Invalidate();
#endif

#elif defined(__DAVAENGINE_DIRECTX9__)
	static LPDIRECT3DTEXTURE9 CreateTextureNative(Vector2 & size, PixelFormat & format, bool isRenderTarget, int32 flags);
	void SetAsHardwareCursor(const Vector2 & hotSpot);
	LPDIRECT3DTEXTURE9 id;
	LPDIRECT3DTEXTURE9 saveTexture;
	bool		 renderTargetModified;
    bool         renderTargetAutosave;

	virtual void SaveToSystemMemory();
	virtual void Lost();
	virtual void Invalidate();
	
#endif //#if defined(__DAVAENGINE_OPENGL__)

	FilePath relativePathname;

	bool		isPink;
	String		debugInfo;
	uint32		width;			// texture width 
	uint32		height;			// texture height
#if defined(__DAVAENGINE_OPENGL__)
	uint32		fboID;			// id of frame buffer object
	uint32		rboID;
#endif //#if defined(__DAVAENGINE_OPENGL__)
	PixelFormat format;			// texture format 
	DepthFormat depthFormat;
	bool		isRenderTarget;
	uint32		textureType;
	TextureInvalidater* invalidater;

	void SetDebugInfo(const String & _debugInfo);

	static const Map<String, Texture*> & GetTextureMap();
    
    int32 GetDataSize() const;
    
    void ReleaseTextureData();
	struct ReleaseTextureDataContainer
	{
		uint32 textureType;
		uint32 id;
		uint32 fboID;
		uint32 rboID;
	};

    void GenerateID();
    
    static void SetDefaultGPU(eGPUFamily gpuFamily);
    static eGPUFamily GetDefaultGPU();
    
    
    inline const eGPUFamily GetSourceFileGPUFamily() const;
    inline TextureDescriptor * GetDescritor() const;
    
private:
    
	static Map<String, Texture*> textureMap;
	static Texture * Get(const FilePath & name);
	static void AddToMap(Texture *tex);
    
	static Texture * CreateFromImage(TextureDescriptor *descriptor, eGPUFamily gpu);


	Vector<Image *> images;
	bool LoadImages(eGPUFamily gpu);
	void SetParamsFromImages();
	void FlushDataToRendererInternal(BaseObject * caller, void * param, void *callerData);
	void FlushDataToRenderer();
	void ReleaseImages();

    void MakePink(TextureType requestedType = Texture::TEXTURE_2D);
	void ReleaseTextureDataInternal(BaseObject * caller, void * param, void *callerData);

	void GeneratePixelesationInternal(BaseObject * caller, void * param, void *callerData);

    static bool CheckImageSize(const Vector<Image *> &imageSet);
    static bool IsCompressedFormat(PixelFormat format);
    
	static uint32 ConvertToPower2FBOValue(uint32 value);

	void GenerateMipmapsInternal(BaseObject * caller, void * param, void *callerData);

	static PixelFormat defaultRGBAFormat;
	Texture();
	virtual ~Texture();
    
    Image * ReadDataToImage();

	static Texture * pinkPlaceholder;
    
    static PixelFormatDescriptor pixelDescriptors[FORMAT_COUNT];
    static void SetPixelDescription(PixelFormat index, const String &name, int32 size, GLenum type, GLenum format, GLenum internalFormat);
    
#if defined(__DAVAENGINE_OPENGL__)
	void HWglCreateFBOBuffers();
	void HWglCreateFBOBuffersInternal(BaseObject * caller, void * param, void *callerData);

    static GLint HWglFilterToGLFilter(TextureFilter filter);
    static GLint HWglConvertWrapMode(TextureWrap wrap);
#endif //#if defined(__DAVAENGINE_OPENGL__)
    
    static eGPUFamily defaultGPU;
    eGPUFamily loadedAsFile;
    
    static bool IsLoadAvailable(const eGPUFamily gpuFamily, const TextureDescriptor *descriptor);
    
	static eGPUFamily GetFormatForLoading(const eGPUFamily requestedGPU, const TextureDescriptor *descriptor);


	TextureState state;
	TextureDescriptor *texDescriptor;
};
    
// Implementation of inline functions
inline void Texture::EnableRenderTargetAutosave(bool isEnabled)
{
#if defined(__DAVAENGINE_DIRECTX9__) //|| defined(__DAVAENGINE_ANDROID__)
    renderTargetAutosave = isEnabled;
#endif //#if defined(__DAVAENGINE_DIRECTX9__) //|| defined(__DAVAENGINE_ANDROID__)
}
inline const FilePath & Texture::GetPathname() const
{
	return relativePathname;
}
    
inline const eGPUFamily Texture::GetSourceFileGPUFamily() const
{
    return loadedAsFile;
}

inline Texture::TextureState Texture::GetState() const
{
	return state;
}

inline TextureDescriptor * Texture::GetDescritor() const
{
    return texDescriptor;
}


};
#endif // __DAVAENGINE_TEXTUREGLES_H__
