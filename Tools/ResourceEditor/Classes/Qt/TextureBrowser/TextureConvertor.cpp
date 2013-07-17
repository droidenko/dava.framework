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

#include <QtConcurrentRun>
#include <QPainter>
#include <QProcess>
#include <QTextOption>
#include <QLabel>

#include "Main/mainwindow.h"
#include "TextureBrowser/TextureConvertor.h"
#include "TextureCompression/PVRConverter.h"
#include "TextureCompression/DXTConverter.h"
#include "SceneEditor/SceneValidator.h"
#include "Render/LibDxtHelper.h"

#include "Platform/Qt/QtLayer.h"

#include "../Main/QtUtils.h"

TextureConvertor::TextureConvertor()
	: curJobOriginal(NULL)
	, jobIdCounter(1)
	, convertJobQueueSize(0)
	, waitDialog(NULL)
	, waitDialogCancelBnt(NULL)
{
	// slots will be called in connector(this) thread
	QObject::connect(&originalWatcher, SIGNAL(finished()), this, SLOT(threadOriginalFinished()), Qt::QueuedConnection);
	QObject::connect(&convertedWatcher, SIGNAL(finished()), this, SLOT(threadConvertedFinished()), Qt::QueuedConnection);
}

TextureConvertor::~TextureConvertor()
{
	CancelConvert();

	originalWatcher.waitForFinished();
	convertedWatcher.waitForFinished();
}

int TextureConvertor::GetOriginal(const DAVA::TextureDescriptor *descriptor)
{
	int ret = 0;

	if(NULL != descriptor)
	{
		DAVA::Vector<DAVA::TextureDescriptor> *textures = new DAVA::Vector<DAVA::TextureDescriptor>();
		textures->push_back(*descriptor);

		JobItem newJob;
		newJob.id = jobIdCounter++;
		newJob.data = new TextureDescriptor(*descriptor);
		newJob.identity = descriptor;

		jobStackOriginal.push(newJob);
		jobRunNextOriginal();

		ret = newJob.id;
	}

	return ret;
}

int TextureConvertor::GetConverted(const DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu, bool forceConver /*= false*/ )
{
	int ret = 0;

	if(NULL != descriptor)
	{
		JobItem newJob;
		newJob.id = jobIdCounter++;
		newJob.force = forceConver;
		newJob.type = gpu;
		newJob.data = new TextureDescriptor(*descriptor);
		newJob.identity = descriptor;

		jobStackConverted.push(newJob);
		jobRunNextConvert();

		ret = newJob.id;
	}

	return ret;
}

int TextureConvertor::Reconvert(DAVA::Scene *scene, bool forceConvert)
{
	int ret = 0;

	if(NULL != scene)
	{
		// get list of all scenes textures
		DAVA::Map<DAVA::String, DAVA::Texture *> allTextures;
		SceneDataManager::EnumerateTextures(scene, allTextures);

		// add jobs to convert every texture
		if(allTextures.size() > 0)
		{
			DAVA::Map<DAVA::String, DAVA::Texture *>::iterator begin = allTextures.begin();
			DAVA::Map<DAVA::String, DAVA::Texture *>::iterator end = allTextures.end();

			for(; begin != end; begin++)
			{
				DAVA::TextureDescriptor *descriptor = begin->second->CreateDescriptor();

				if(NULL != descriptor)
				{
					for(int gpu = DAVA::GPU_UNKNOWN + 1; gpu < DAVA::GPU_FAMILY_COUNT; ++gpu)
					{
						JobItem newJob;
						newJob.id = jobIdCounter++;
						newJob.data = new DAVA::TextureDescriptor(*descriptor);
						newJob.force = forceConvert;
						newJob.type = gpu;

						jobStackConverted.push(newJob);
						jobRunNextConvert();

						ret = newJob.id;
					}
				}

				descriptor->Release();
			}
		}
	}

	// 0 means no job were created
	return ret;
}

void TextureConvertor::WaitConvertedAll(QWidget *parent)
{
	if(convertJobQueueSize > 0)
	{
		waitDialog = new QProgressDialog(parent, Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint);

		QLabel *waitDialogLabel = new QLabel(waitDialog);
		waitDialogLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		waitDialog->setLabel(waitDialogLabel);

		waitDialogCancelBnt = new QPushButton("Cancel", waitDialog);
		waitDialog->setCancelButton(waitDialogCancelBnt);

		if(0 == jobStackConverted.size())
		{
			waitDialogCancelBnt->setEnabled(false);
		}

		QObject::connect(waitDialogCancelBnt, SIGNAL(pressed()), this, SLOT(waitCanceled()));

		waitDialog->setRange(0, convertJobQueueSize);
		waitDialog->setValue(convertJobQueueSize - jobStackConverted.size());
		waitDialog->setMinimumSize(400, 150);
		waitDialog->setLabelText(waitStatusText);
		waitDialog->setWindowModified(Qt::WindowModal);
		waitDialog->exec();

		waitDialog->deleteLater();
		waitDialog = NULL;
		waitDialogCancelBnt = NULL;
	}
}

void TextureConvertor::CancelConvert()
{
	JobItem *item = jobStackConverted.pop();

	while (NULL != item)
	{
		TextureDescriptor* desc = (TextureDescriptor*) item->data;
		if(NULL != desc)
		{
			delete desc;
		}

		delete item;
		item = jobStackConverted.pop();
	}
}

void TextureConvertor::jobRunNextOriginal()
{
	// if there is no already running work
	if((originalWatcher.isFinished() || originalWatcher.isCanceled()) && NULL == curJobOriginal)
	{
		// get the new work
		curJobOriginal = jobStackOriginal.pop();
		if(NULL != curJobOriginal)
		{
			// copy descriptor
			QFuture<QImage> f = QtConcurrent::run(this, &TextureConvertor::GetOriginalThread, curJobOriginal);
			originalWatcher.setFuture(f);
		}
	}
}

void TextureConvertor::jobRunNextConvert()
{
	// if there is no already running work
	if((convertedWatcher.isFinished() || convertedWatcher.isCanceled()) && NULL == curJobConverted)
	{
		// get the next job
		curJobConverted = jobStackConverted.pop();
		if(NULL != curJobConverted)
		{
			TextureDescriptor *desc = (TextureDescriptor *) curJobConverted->data;

			QFuture<QImage> f = QtConcurrent::run(this, &TextureConvertor::GetConvertedThread, curJobConverted);
			convertedWatcher.setFuture(f);

			if(0 == convertJobQueueSize)
			{
				convertJobQueueSize = 1;
			}

			emit ConvertStatusImg(desc->pathname.GetAbsolutePathname().c_str(), curJobConverted->type);
			emit ConvertStatusQueue(convertJobQueueSize - jobStackConverted.size(), convertJobQueueSize);

			// generate current wait message, that can be displayed by wait dialog
			waitStatusText = "Please wait, convert in progress.\n\nPath:\t";
			waitStatusText += desc->pathname.GetAbsolutePathname().c_str();
			waitStatusText += "\nGPU:\t";
			waitStatusText += GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(curJobConverted->type);

			if(NULL != waitDialog)
			{
				waitDialog->setValue(convertJobQueueSize - jobStackConverted.size());
				waitDialog->setLabelText(waitStatusText);
			}
		}
		else
		{
			waitStatusText = "";

			// if no job in stack, emit signal that all jobs are finished
			if(jobStackOriginal.size() == 0 && jobStackConverted.size() == 0)
			{
				emit ReadyConvertedAll();
			}

			convertJobQueueSize = 0;

			emit ConvertStatusImg("", DAVA::GPU_UNKNOWN);
			emit ConvertStatusQueue(0, 0);

			if(NULL != waitDialog)
			{
				// close wait dialog
				waitDialog->reset();
			}
		}
	}
	else
	{
		convertJobQueueSize++;
		emit ConvertStatusQueue(convertJobQueueSize - jobStackConverted.size(), convertJobQueueSize);

		if(NULL != waitDialog)
		{
			waitDialog->setMaximum(convertJobQueueSize);
		}
	}
}

void TextureConvertor::threadOriginalFinished()
{
	if(originalWatcher.isFinished() && NULL != curJobOriginal)
	{
		const DAVA::TextureDescriptor *originalDescriptor = (DAVA::TextureDescriptor *) curJobOriginal->identity;
		DAVA::TextureDescriptor *descriptor = (DAVA::TextureDescriptor *) curJobOriginal->data;

		emit ReadyOriginal(originalDescriptor, originalWatcher.result());

		delete descriptor;
		delete curJobOriginal;
		curJobOriginal = NULL;
	}

	jobRunNextOriginal();
}

void TextureConvertor::threadConvertedFinished()
{
	if(convertedWatcher.isFinished() && NULL != curJobConverted)
	{
		const DAVA::TextureDescriptor *convertedDescriptor = (DAVA::TextureDescriptor *) curJobConverted->identity;
		DAVA::TextureDescriptor *descriptor = (DAVA::TextureDescriptor *) curJobConverted->data;

		emit ReadyConverted(convertedDescriptor, (DAVA::eGPUFamily) curJobConverted->type, convertedWatcher.result());

		delete descriptor;
		delete curJobConverted;
		curJobConverted = NULL;
	}

	jobRunNextConvert();
}

void TextureConvertor::waitCanceled()
{
	if(NULL != waitDialogCancelBnt)
	{
		CancelConvert();
		waitDialogCancelBnt->setEnabled(false);
	}
}

QImage TextureConvertor::GetOriginalThread(JobItem *item)
{
	QImage img;
    void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();
    
	if(NULL != item && NULL != item->data)
	{
		TextureDescriptor *descriptor = (TextureDescriptor *) item->data;
		img = QImage(descriptor->GetSourceTexturePathname().GetAbsolutePathname().c_str());
	}

    DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
	return img;
}

QImage TextureConvertor::GetConvertedThread(JobItem *item)
{
	void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();

	QImage ret;
	DAVA::Image* davaImg = NULL;

	if(NULL != item)
	{
		DAVA::TextureDescriptor *descriptor = (DAVA::TextureDescriptor*) item->data;
		DAVA::eGPUFamily gpu = (DAVA::eGPUFamily) item->type;

		if( NULL != descriptor &&
			gpu > DAVA::GPU_UNKNOWN && gpu < DAVA::GPU_FAMILY_COUNT && 
			descriptor->compression[gpu].format > DAVA::FORMAT_INVALID && descriptor->compression[gpu].format < DAVA::FORMAT_COUNT)
		{
			const String& outExtension = GPUFamilyDescriptor::GetCompressedFileExtension(gpu, (DAVA::PixelFormat) descriptor->compression[gpu].format);
			if(outExtension == ".pvr")
			{
				DAVA::Logger::Info("Starting PVR conversion (%s), id %d...", 
					GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(descriptor->compression[gpu].format), item->id);
				davaImg = ConvertPVR(descriptor, gpu, item->force);
				DAVA::Logger::Info("Done, id %d", item->id);
			}
			else if(outExtension == ".dds")
			{
				DAVA::Logger::Info("Starting DXT conversion (%s), id %d...",
					GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(descriptor->compression[gpu].format), item->id);
				davaImg = ConvertDXT(descriptor, gpu, item->force);
				DAVA::Logger::Info("Done, id %d", item->id);
			}
			else
			{
				DVASSERT(false);
			}
		}
		else
		{
			DAVA::Logger::Error("NULL descriptor or wrong GPU type", item->id);
		}
	}

	if(NULL != davaImg)
	{
		ret = FromDavaImage(davaImg);
		davaImg->Release();
	}

	DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
	return ret;
}

DAVA::Image* TextureConvertor::ConvertPVR(DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu, bool forceConvert)
{
	DAVA::Image* image = NULL;
	DAVA::FilePath compressedTexturePath = DAVA::GPUFamilyDescriptor::CreatePathnameForGPU(descriptor, gpu);
	DAVA::FilePath outputPath = PVRConverter::Instance()->GetPVRToolOutput(*descriptor, gpu);
	if(!outputPath.IsEmpty())
	{
		if(forceConvert || !DAVA::FileSystem::Instance()->IsFile(outputPath))
		{
            DeleteOldPVRTextureIfPowerVr_IOS(descriptor, gpu);

            
			QString command = PVRConverter::Instance()->GetCommandLinePVR(*descriptor, gpu).c_str();
			DAVA::Logger::Info("%s", command.toStdString().c_str());

			QProcess p;
			p.start(command);
			p.waitForFinished(-1);

			if(QProcess::NormalExit != p.exitStatus())
			{
				DAVA::Logger::Error("Converter process crushed");
			}
			if(0 != p.exitCode())
			{
				DAVA::Logger::Error("Converter exit with error %d", p.exitCode());
				DAVA::Logger::Error("Stderror:\n%s", p.readAllStandardError().constData());
				DAVA::Logger::Error("Stdout:\n%s", p.readAllStandardOutput().constData());
				DAVA::Logger::Error("---");
			}

			bool wasUpdated = descriptor->UpdateCrcForFormat(gpu);
            if(wasUpdated)
            {
                descriptor->Save();
            }
        }

		Vector<DAVA::Image *> davaImages = DAVA::ImageLoader::CreateFromFile(outputPath);

		if(davaImages.size() > 0)
		{
			image = davaImages[0];
			image->Retain();
		}

		for_each(davaImages.begin(), davaImages.end(),  DAVA::SafeRelease<DAVA::Image>);
	}

	return image;
}

DAVA::Image* TextureConvertor::ConvertDXT(DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu, bool forceConvert)
{
	DAVA::Image* image = NULL;
	DAVA::FilePath outputPath = DXTConverter::GetDXTOutput(*descriptor, gpu);
	if(!outputPath.IsEmpty())
	{
		if(forceConvert || !DAVA::FileSystem::Instance()->IsFile(outputPath))
		{
            DeleteOldDXTTextureIfTegra(descriptor, gpu);

			outputPath = DXTConverter::ConvertPngToDxt(*descriptor, gpu);

			bool wasUpdated = descriptor->UpdateCrcForFormat(gpu);
            if(wasUpdated)
            {
                descriptor->Save();
            }
		}

		Vector<DAVA::Image *> davaImages = DAVA::ImageLoader::CreateFromFile(outputPath);

		if(davaImages.size() > 0)
		{
			image = davaImages[0];
			image->Retain();
		}

		for_each(davaImages.begin(), davaImages.end(),  DAVA::SafeRelease< DAVA::Image>);
	}

	return image;
}

QImage TextureConvertor::FromDavaImage(DAVA::Image *image)
{
	QImage qtImage;

	if(NULL != image)
	{
		QRgb *line;

		switch(image->format)
		{
		case DAVA::FORMAT_DXT1:
		case DAVA::FORMAT_DXT1A:
		case DAVA::FORMAT_DXT1NM:
		case DAVA::FORMAT_DXT3:
		case DAVA::FORMAT_DXT5:
		case DAVA::FORMAT_DXT5NM:
		{
			Vector<Image* > vec;
			LibDxtHelper::DecompressImageToRGBA(*image, vec, true);
			if(vec.size() == 1)
			{
				qtImage = TextureConvertor::FromDavaImage(vec.front());
			}
			else
			{
				DAVA::Logger::Error("Error during conversion from DDS to QImage.");
			}
			break;
		}
		case DAVA::FORMAT_PVR4:
		case DAVA::FORMAT_PVR2:
		case DAVA::FORMAT_RGBA8888:
			{
				DAVA::uint32 *data = (DAVA::uint32 *) image->data;
				DAVA::uint32 c;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA8888 into Qt ARGB8888
				for (int y = 0; y < (int)image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0; x < (int)image->width; x++) 
					{
						c = data[y * image->width + x];
						line[x] = (c & 0xFF00FF00) | ((c & 0x00FF0000) >> 16) | ((c & 0x000000FF) << 16);
					}
				}
			}
			break;

		case DAVA::FORMAT_RGBA5551:
			{
				DAVA::uint16 *data = (DAVA::uint16 *) image->data;
				DAVA::uint32 c;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA5551 into Qt ARGB8888
				for (int y = 0; y < (int)image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0; x < (int)image->width; x++) 
					{
						DAVA::uint32 a;
						DAVA::uint32 r;
						DAVA::uint32 g;
						DAVA::uint32 b;

						c = data[y * image->width + x];
						r = (c >> 11) & 0x1f;
						g = (c >> 6) & 0x1f;
						b = (c >> 1) & 0x1f;
						a = (c & 0x1) ? 0xff000000 : 0x0;

						line[x] = (a | (r << (16 + 3)) | (g << (8 + 3)) | (b << 3));
					}
				}
			}
			break;

		case DAVA::FORMAT_RGBA4444:
			{
				DAVA::uint16 *data = (DAVA::uint16 *) image->data;
				DAVA::uint32 c;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA4444 into Qt ARGB8888
				for (int y = 0; y < (int)image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0; x < (int)image->width; x++) 
					{
						DAVA::uint32 a;
						DAVA::uint32 r;
						DAVA::uint32 g;
						DAVA::uint32 b;

						c = data[y * image->width + x];
						r = (c >> 12) & 0xf;
						g = (c >> 8) & 0xf;
						b = (c >> 4) & 0xf;
						a = (c & 0xf);

						line[x] = ((a << (24 + 4)) | (r << (16 + 4)) | (g << (8+4)) | (b << 4));
					}
				}
			}
			break;

		case DAVA::FORMAT_RGB565:
			{
				DAVA::uint16 *data = (DAVA::uint16 *) image->data;
				DAVA::uint32 c;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA565 into Qt ARGB8888
				for (int y = 0; y < (int)image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0; x < (int)image->width; x++) 
					{
						DAVA::uint32 a;
						DAVA::uint32 r;
						DAVA::uint32 g;
						DAVA::uint32 b;

						c = data[y * image->width + x];
						a = 0xff;
						r = (c >> 11) & 0x1f;
						g = (c >> 5) & 0x3f;
						b = c & 0x1f;

						line[x] = ((a << 24) | (r << (16 + 3)) | (g << (8 + 2)) | (b << 3));
					}
				}
			}
			break;

		case DAVA::FORMAT_A8:
			{
				DAVA::uint8 *data = (DAVA::uint8 *) image->data;
				DAVA::uint32 c;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA565 into Qt ARGB8888
				for (int y = 0; y < (int)image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0; x < (int)image->width; x++) 
					{
						c = data[y * image->width + x];
						line[x] = ((0xff << 24) | (c << 16) | (c << 8) | c);
					}
				}
			}
			break;

		default:
			break;
		}
	}

	return qtImage;
}
