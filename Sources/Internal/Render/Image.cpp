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


#include "Render/Image.h"
#include "Render/Texture.h"
#include "Render/ImageConvert.h"

namespace DAVA 
{
    

Image::Image()
:	data(0)
,   dataSize(0)
,	width(0)
,	height(0)
,	format(FORMAT_RGB565)
,	cubeFaceID(Texture::CUBE_FACE_INVALID)
,	mipmapLevel(-1)
{
}

Image::~Image()
{
	SafeDeleteArray(data);
	
	width = 0;
	height = 0;
}

Image * Image::Create(uint32 width, uint32 height, PixelFormat format)
{
	Image * image = new Image();
	image->width = width;
	image->height = height;
	image->format = format;
    
    int32 formatSize = Texture::GetPixelFormatSizeInBytes(format);
    if (formatSize ||
		(format >= FORMAT_DXT1 && format <= FORMAT_DXT1A) ||
		(format >= FORMAT_ATC_RGB && format <= FORMAT_ATC_RGBA_INTERPOLATED_ALPHA))
    {
		//workaround, because formatSize is not designed for formats with 4 bits per pixel
		image->dataSize = width * height * formatSize;
		
		if ((format >= FORMAT_DXT1 && format <= FORMAT_DXT5NM) ||
			(format >= FORMAT_ATC_RGB && format <= FORMAT_ATC_RGBA_INTERPOLATED_ALPHA))
		{
			uint32 dSize = formatSize == 0 ? (width * height) / 2 : width * height ;
			if(width < 4 || height < 4)// size lower than  block's size
			{
				uint32 minvalue = width < height ? width : height;
				uint32 maxvalue = width > height ? width : height;
				minvalue = minvalue < 4 ? 4 : minvalue;
				maxvalue = maxvalue < 4 ? 4 : maxvalue;
				dSize = Texture::GetPixelFormatSizeInBits(format) * minvalue * maxvalue;
				dSize /= 8;
			}
			image->dataSize = dSize;
		}
        
        image->data = new uint8[image->dataSize];
    }
    else 
    {
        Logger::Error("[Image::Create] trying to create image with wrong format");
		SafeRelease(image);
    }
    
	return image;
}

Image * Image::CreateFromData(uint32 width, uint32 height, PixelFormat format, const uint8 *data)
{
	Image * image = Image::Create(width, height, format);
	if(!image) return NULL;

	if(data)
	{
		Memcpy(image->data, data, image->dataSize);
	}

	return image;
}

Image * Image::CreatePinkPlaceholder()
{
    Image * image = new Image();
	image->width = 16;
	image->height = 16;
	image->format = FORMAT_RGBA8888;
    image->dataSize = image->width * image->height * Texture::GetPixelFormatSizeInBytes(FORMAT_RGBA8888);
    image->data = new uint8[image->dataSize];

    
    uint32 pink = 0xffff00ff;
    uint32 gray = 0xff7f7f7f;
    bool pinkOrGray = false;
    
    uint32 * writeData = (uint32*) image->data;
    for(uint32 w = 0; w < image->width; ++w)
    {
        pinkOrGray = !pinkOrGray;
        for(uint32 h = 0; h < image->height; ++h)
        {
            *writeData++ = pinkOrGray ? pink : gray;
            pinkOrGray = !pinkOrGray;
        }
    }

    return image;
}

Vector<Image *> Image::CreateMipMapsImages()
{
    Vector<Image *> imageSet;

    int32 formatSize = Texture::GetPixelFormatSizeInBytes(format);
    if(!formatSize)
        return imageSet;

    Image * image0 = SafeRetain(this);
    uint32 imageWidth = width;
    uint32 imageHeight = height;
    uint32 curMipMapLevel = 0;
    image0->mipmapLevel = curMipMapLevel;

    imageSet.push_back(image0);
    while(imageHeight > 1 || imageWidth > 1)
    {
        uint32 newWidth = imageWidth;
        uint32 newHeight = imageHeight;
        if(newWidth > 1) newWidth >>= 1;
        if(newHeight > 1) newHeight >>= 1;
        uint8 * newData = new uint8[newWidth * newHeight * formatSize];
        memset(newData, 0, newWidth * newHeight * formatSize);

        ImageConvert::DownscaleTwiceBillinear(format, format,
            image0->data, imageWidth, imageHeight, imageWidth * formatSize,
            newData, newWidth, newHeight, newWidth * formatSize);

        curMipMapLevel++;

        Image * halfSizeImg = Image::CreateFromData(newWidth, newHeight, format, newData);
        halfSizeImg->cubeFaceID = image0->cubeFaceID;
        halfSizeImg->mipmapLevel = curMipMapLevel;
        imageSet.push_back(halfSizeImg);

        imageWidth = newWidth;
        imageHeight = newHeight;
        SafeDeleteArray(newData);

        image0 = halfSizeImg;
    }

    return imageSet;
}

void Image::ResizeImage(uint32 newWidth, uint32 newHeight)
{
	uint8 * newData = NULL;
	int32 formatSize = Texture::GetPixelFormatSizeInBytes(format);

	if(formatSize>0)
	{
		newData = new uint8[newWidth * newHeight * formatSize];
		memset(newData, 0, newWidth * newHeight * formatSize);

		float32 kx = (float32)width / (float32)newWidth;
		float32 ky = (float32)height / (float32)newHeight;

		float32 xx = 0, yy = 0;
		uint32 offset = 0;
		uint32 offsetOld = 0;
		uint32 posX, posY;
		for (uint32 y = 0; y < newHeight; ++y)
		{
			for (uint32 x = 0; x < newWidth; ++x)
			{
				posX = (uint32)(xx + 0.5f);
				posY = (uint32)(yy + 0.5f);
				if (posX >= width)
					posX = width - 1;

				if (posY >= height)
					posY = height - 1;


				offsetOld = (posY * width + posX) * formatSize;
				memcpy(newData + offset, data + offsetOld, formatSize);

				xx += kx;
				offset += formatSize;
			}
			yy += ky;
			xx = 0;
		}

		// resized data
		width = newWidth;
		height = newHeight;
		SafeDeleteArray(data);
		data = newData;
	}
}

void Image::ResizeCanvas(uint32 newWidth, uint32 newHeight)
{
    uint8 * newData = NULL;
    uint32 newDataSize = 0;
    int32 formatSize = Texture::GetPixelFormatSizeInBytes(format);
        
    if(formatSize>0)
    {
        newDataSize = newWidth * newHeight * formatSize;
        newData = new uint8[newDataSize];
            
        uint32 currentLine = 0;
        uint32 indexOnLine = 0;
        uint32 indexInOldData = 0;
            
        for(uint32 i = 0; i < newDataSize; ++i)
        {
            if((currentLine+1)*newWidth*formatSize<=i)
            {
                currentLine++;
            }
                
            indexOnLine = i - currentLine*newWidth*formatSize;
                
            if(currentLine<(uint32)height)
            {
                // within height of old image
                if(indexOnLine<(uint32)(width*formatSize))
                {
                    // we have data in old image for new image
                    indexInOldData = currentLine*width*formatSize + indexOnLine;
                    newData[i] = data[indexInOldData];
                }
                else
                {
                    newData[i] = 0;
                }
            }
            else
            {
                newData[i] = 0;
            }
        }
            
        // resized data
        width = newWidth;
        height = newHeight;
            
        SafeDeleteArray(data);
        data = newData;
        dataSize = newDataSize;
    }
}

void Image::ResizeToSquare()
{
    uint32 newImageSize = Max(width, height);
    ResizeCanvas(newImageSize, newImageSize);
}

Image* Image::CopyImageRegion(const Image* imageToCopy,
							  uint32 newWidth, uint32 newHeight,
							  uint32 xOffset, uint32 yOffset)
{
	DVASSERT(newWidth >= 0 && newHeight >= 0 && xOffset >= 0 && yOffset >= 0);

	uint32 oldWidth = imageToCopy->GetWidth();
	uint32 oldHeight = imageToCopy->GetHeight();
	DVASSERT((newWidth + xOffset) <= oldWidth && (newHeight + yOffset) <= oldHeight);

	PixelFormat format = imageToCopy->GetPixelFormat();
	int32 formatSize = Texture::GetPixelFormatSizeInBytes(format);

	Image* newImage = Image::Create(newWidth, newHeight, format);

	uint8* oldData = imageToCopy->GetData();
	uint8* newData = newImage->data;

	for (uint32 i = 0; i < newHeight; ++i)
	{
		memcpy((newData + newWidth * i * formatSize),
			   (oldData + (oldWidth * (yOffset + i) + xOffset) * formatSize),
			   formatSize * newWidth);
	}

	return newImage;
}

Image* Image::CopyImageRegion(const Image* imageToCopy, const Rect& rect)
{
	return CopyImageRegion(imageToCopy, (uint32)rect.dx, (uint32)rect.dy, (uint32)rect.x, (uint32)rect.y);
}

void Image::InsertImage(const Image* image, uint32 dstX, uint32 dstY,
						uint32 srcX /* = 0 */, uint32 srcY /* = 0 */,
						uint32 srcWidth /* = -1 */, uint32 srcHeight /* = -1 */)
{
	if (GetPixelFormat() != image->GetPixelFormat())
	{
		return;
	}

	if (image == NULL || dstX >= width || dstY >= height ||
		srcX >= image->GetWidth() || srcY >= image->GetHeight())
	{
		return;
	}

	uint32 insertWidth = (srcWidth == (uint32)-1) ? image->GetWidth() : srcWidth;
	uint32 insertHeight = (srcHeight == (uint32)-1) ? image->GetHeight() : srcHeight;

	if (srcX + insertWidth > image->GetWidth())
	{
		insertWidth = image->GetWidth() - srcX;
	}
	if (dstX + insertWidth > width)
	{
		insertWidth = width - dstX;
	}

	if (srcY + insertHeight > image->GetHeight())
	{
		insertHeight = image->GetHeight() - srcY;
	}
	if (dstY + insertHeight > height)
	{
		insertHeight = height - dstY;
	}

	PixelFormat format = GetPixelFormat();
	int32 formatSize = Texture::GetPixelFormatSizeInBytes(format);

	uint8* srcData = image->GetData();
	uint8* dstData = data;

	for (uint32 i = 0; i < insertHeight; ++i)
	{
		memcpy(dstData + (width * (dstY + i) + dstX) * formatSize,
			   srcData + (image->GetWidth() * (srcY + i) + srcX) * formatSize,
			   formatSize * insertWidth);
	}
}

void Image::InsertImage(const Image* image, const Vector2& dstPos, const Rect& srcRect)
{
	InsertImage(image, (uint32)dstPos.x, (uint32)dstPos.y,
				(uint32)srcRect.x, (uint32)srcRect.y, (uint32)srcRect.dx, (uint32)srcRect.dy);
}

};
