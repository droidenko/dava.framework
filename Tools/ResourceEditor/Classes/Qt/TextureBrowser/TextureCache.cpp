#include "Qt/TextureBrowser/TextureCache.h"


QImage TextureCache::getOriginal(const DAVA::TextureDescriptor *descriptor)
{
	QImage img;

	if(NULL != descriptor && cacheOriginal.contains(descriptor))
	{
		img = cacheOriginal[descriptor];
	}

	return img;
}

QImage TextureCache::getPVR(const DAVA::TextureDescriptor *descriptor)
{
	QImage img;

	if(NULL != descriptor && cacheOriginal.contains(descriptor))
	{
		img = cachePVR[descriptor];
	}

	return img;
}

QImage TextureCache::getDXT(const DAVA::TextureDescriptor *descriptor)
{
	QImage img;

	if(NULL != descriptor && cacheOriginal.contains(descriptor))
	{
		img = cacheDXT[descriptor];
	}

	return img;
}

void TextureCache::setOriginal(const DAVA::TextureDescriptor *descriptor, const QImage &image)
{
	if(NULL != descriptor)
	{
		cacheOriginal[descriptor] = image;
	}
}

void TextureCache::setPVR(const DAVA::TextureDescriptor *descriptor, const QImage &image)
{
	if(NULL != descriptor)
	{
		cachePVR[descriptor] = image;
	}
}

void TextureCache::setDXT(const DAVA::TextureDescriptor *descriptor, const QImage &image)
{
	if(NULL != descriptor)
	{
		cacheDXT[descriptor] = image;
	}
}

void TextureCache::clearAll()
{
	cacheOriginal.clear();
	cachePVR.clear();
	cacheDXT.clear();
}

void TextureCache::clearOriginal(const DAVA::TextureDescriptor *descriptor)
{
	cacheOriginal.remove(descriptor);
}

void TextureCache::clearPVR(const DAVA::TextureDescriptor *descriptor)
{
	cachePVR.remove(descriptor);
}

void TextureCache::clearDXT(const DAVA::TextureDescriptor *descriptor)
{
	cacheDXT.remove(descriptor);
}
