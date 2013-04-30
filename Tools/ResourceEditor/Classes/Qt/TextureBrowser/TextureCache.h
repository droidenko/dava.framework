#ifndef __TEXTURE_CACHE_H__
#define __TEXTURE_CACHE_H__

#include "DAVAEngine.h"
#include <QMap>
#include <QImage>

#include "Qt/TextureBrowser/TextureConvertor.h"

class TextureCache : public QObject, public DAVA::StaticSingleton<TextureCache>
{
	Q_OBJECT

public:
	QImage getOriginal(const DAVA::TextureDescriptor *descriptor);
	QImage getPVR(const DAVA::TextureDescriptor *descriptor);
	QImage getDXT(const DAVA::TextureDescriptor *descriptor);

	void setOriginal(const DAVA::TextureDescriptor *descriptor, const QImage &image);
	void setPVR(const DAVA::TextureDescriptor *descriptor, const QImage &image);
	void setDXT(const DAVA::TextureDescriptor *descriptor, const QImage &image);

	void clearAll();
	void clearOriginal(const DAVA::TextureDescriptor *descriptor);
	void clearPVR(const DAVA::TextureDescriptor *descriptor);
	void clearDXT(const DAVA::TextureDescriptor *descriptor);

private:
	QMap<const DAVA::TextureDescriptor*, QImage> cacheOriginal;
	QMap<const DAVA::TextureDescriptor*, QImage> cachePVR;
	QMap<const DAVA::TextureDescriptor*, QImage> cacheDXT;
};

#endif // __TEXTURE_CACHE_H__
