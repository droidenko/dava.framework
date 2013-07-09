/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __TEXTURE_CONVERTOR_H__
#define __TEXTURE_CONVERTOR_H__

#include <QObject>
#include <QImage>
#include <QFutureWatcher>
#include <QProgressDialog>

#include "DAVAEngine.h"
#include "Render/TextureDescriptor.h"
#include "Render/RenderManager.h"
#include "TextureBrowser/TextureConvertorWork.h"

#define CONVERT_JOB_COUNT 2

class TextureConvertor : public QObject, public DAVA::StaticSingleton<TextureConvertor>
{
	Q_OBJECT

public:
	TextureConvertor();
	~TextureConvertor();

	static QImage FromDavaImage(DAVA::Image *image);
	static DAVA::Image* ConvertPVR(DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu, bool forceConvert);
	static DAVA::Image* ConvertDXT(DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu, bool forceConvert);

	int GetOriginal(const DAVA::TextureDescriptor *descriptor);
	int GetConverted(const DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu, bool forceConver = false);
	int Reconvert(DAVA::Scene *scene, bool forceConvert);
	
	void WaitConvertedAll(QWidget *parent = NULL);
	void CancelConvert();

signals:
	void ReadyOriginal(const DAVA::TextureDescriptor *descriptor, const QImage &image);
	void ReadyConverted(const DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu, const QImage &image);
	void ReadyReconvert();

	void ReadyConvertedAll();

	void ConvertStatusImg(const QString &imgPath, int imgGpu);
	void ConvertStatusQueue(int curJob, int jobCount);

private:
	int jobIdCounter;
	
	int convertJobQueueSize;

	QString waitStatusText;

	QFutureWatcher<QImage> originalWatcher;
	QFutureWatcher<QImage> convertedWatcher;

	JobStack jobStackOriginal;
	JobStack jobStackConverted;

	JobItem *curJobOriginal;
	JobItem *curJobConverted;

	QProgressDialog* waitDialog;
	QPushButton* waitDialogCancelBnt;

	void jobRunNextConvert();
	void jobRunNextOriginal();

	QImage GetOriginalThread(JobItem *item);
	QImage GetConvertedThread(JobItem *item);

private slots:
	void waitCanceled();
	void threadOriginalFinished();
	void threadConvertedFinished();

};

#endif // __TEXTURE_CONVERTOR_H__
