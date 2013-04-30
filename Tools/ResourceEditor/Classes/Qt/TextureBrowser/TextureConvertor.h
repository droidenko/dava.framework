#ifndef __TEXTURE_CONVERTOR_H__
#define __TEXTURE_CONVERTOR_H__

#include <QObject>
#include <QImage>
#include <QFutureWatcher>
#include "DAVAEngine.h"
#include "Render/TextureDescriptor.h"
#include "Render/RenderManager.h"
#include "Qt/TextureBrowser/TextureConvertorWork.h"

#define CONVERT_JOB_COUNT 2

class TextureConvertor : public QObject, public DAVA::StaticSingleton<TextureConvertor>
{
	Q_OBJECT

public:
	TextureConvertor();
	~TextureConvertor();

	static QImage fromDavaImage(DAVA::Image *image);

	void loadOriginal(const DAVA::TextureDescriptor *descriptor);
	void getPVR(const DAVA::TextureDescriptor *descriptor, bool forceConver = false);
	void getDXT(const DAVA::TextureDescriptor *descriptor, bool forceConver = false);

	bool checkAndCompressAll(bool forceConvertAll);

signals:
	void readyOriginal(const DAVA::TextureDescriptor *descriptor, const QImage &image);
	void readyPVR(const DAVA::TextureDescriptor *descriptor, const QImage &image);
	void readyDXT(const DAVA::TextureDescriptor *descriptor, const QImage &image);
	void readyAll();

	void convertStatus(const QString &curPath, int curJob, int jobCount);

private:
	QFutureWatcher<QImage> loadOriginalWatcher;
	QFutureWatcher<QImage> convertWatcher[CONVERT_JOB_COUNT];
	QFutureWatcher<void> convertAllWatcher;

	JobStack jobStackConvert;
	JobItem *curJobConvert[CONVERT_JOB_COUNT];

	JobStack jobStackOriginal;
	JobItem *curJobOriginal;

	void jobRunNextConvert();
	void jobRunNextOriginal();

	QImage loadOriginalThread(JobItem *item);
	QImage convertThreadPVR(JobItem *item);
	QImage convertThreadDXT(JobItem *item);
	void convertAllThread(DAVA::Map<DAVA::String, DAVA::Texture *> *allTextures, bool forceConverAll);

	int jobGetConvertFreeIndex();
	int jobGetConvertIndex(QFutureWatcher<QImage> *watcher);

signals:
	void convertStatusFromThread(const QString &curPath, int curJob, int jobCount);

private slots:
	void threadOriginalFinished();
	void threadConvertFinished();
	void threadConvertAllFinished();
	void threadConvertStatus(const QString &curPath, int curJob, int jobCount);

};

#endif // __TEXTURE_CONVERTOR_H__
