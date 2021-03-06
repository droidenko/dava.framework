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



#include <QtConcurrentRun>
#include <QPainter>
#include <QProcess>
#include <QTextOption>
#include <QPushButton>
#include <QLabel>

#include "Main/mainwindow.h"
#include "TextureBrowser/TextureConvertor.h"
#include "TextureCompression/TextureConverter.h"
#include "SceneEditor/SceneValidator.h"
#include "Render/LibDxtHelper.h"

#include "FileSystem/FileSystem.h"

#include "Platform/Qt/QtLayer.h"
#include "Main/QtUtils.h"
#include "Scene/SceneHelper.h"

TextureConvertor::TextureConvertor()
	: curJobOriginal(NULL)
	, jobIdCounter(1)
	, convertJobQueueSize(0)
	, waitingComletion(0)
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
		// check if requested texture isn't the same that is loading now
		if(NULL == curJobOriginal || curJobOriginal->identity != descriptor)
		{
// 			DAVA::Vector<DAVA::TextureDescriptor> *textures = new DAVA::Vector<DAVA::TextureDescriptor>(); TODO: WTF? unusable code?
// 			textures->push_back(*descriptor);

			JobItem newJob;
			newJob.id = jobIdCounter++;
			newJob.data = new TextureDescriptor(*descriptor);
			newJob.identity = descriptor;

			jobStackOriginal.push(newJob);
			jobRunNextOriginal();

			ret = newJob.id;
		}
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

		if(jobStackConverted.push(newJob))
		{
			convertJobQueueSize++;
		}

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
		SceneHelper::EnumerateTextures(scene, allTextures);

		// add jobs to convert every texture
		if(allTextures.size() > 0)
		{
			DAVA::Map<DAVA::String, DAVA::Texture *>::iterator begin = allTextures.begin();
			DAVA::Map<DAVA::String, DAVA::Texture *>::iterator end = allTextures.end();

			for(; begin != end; begin++)
			{
				DAVA::TextureDescriptor *descriptor = begin->second->GetDescriptor();

				if(NULL != descriptor)
				{
					for(int gpu = DAVA::GPU_UNKNOWN + 1; gpu < DAVA::GPU_FAMILY_COUNT; ++gpu)
					{
						if( ! GPUFamilyDescriptor::IsFormatSupported((eGPUFamily)gpu, (PixelFormat)descriptor->compression[gpu].format))
						{
							continue;
						}

						JobItem newJob;
						newJob.id = jobIdCounter++;
						newJob.data = new DAVA::TextureDescriptor(*descriptor);
						newJob.force = forceConvert;
						newJob.type = gpu;

						if(jobStackConverted.push(newJob))
						{
							convertJobQueueSize++;
						}

						jobRunNextConvert();

						ret = newJob.id;
					}
				}
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
		waitDialog = new QtWaitDialog(parent);
		bool hasCancel = false;
		
		if(jobStackConverted.size() > 0)
		{
			hasCancel = true;
		}

		QObject::connect(waitDialog, SIGNAL(canceled()), this, SLOT(waitCanceled()));

		waitDialog->SetRange(0, convertJobQueueSize);
		waitDialog->SetValue(convertJobQueueSize - jobStackConverted.size());
		waitDialog->SetMessage(waitStatusText);

		waitingComletion = true;
		waitDialog->Exec("Waiting for conversion completion", waitStatusText, true, hasCancel);

		waitDialog->deleteLater();
		waitDialog = NULL;
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
			SafeRelease(desc);
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
			QFuture< DAVA::Vector<QImage> > f = QtConcurrent::run(this, &TextureConvertor::GetOriginalThread, curJobOriginal);
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

			QFuture< DAVA::Vector<QImage> > f = QtConcurrent::run(this, &TextureConvertor::GetConvertedThread, curJobConverted);
			convertedWatcher.setFuture(f);

			emit ConvertStatusImg(desc->pathname.GetAbsolutePathname().c_str(), curJobConverted->type);
			emit ConvertStatusQueue(convertJobQueueSize - jobStackConverted.size(), convertJobQueueSize);

			// generate current wait message, that can be displayed by wait dialog
			waitStatusText = "Path: ";
			waitStatusText += desc->pathname.GetAbsolutePathname().c_str();
			waitStatusText += "\n\nGPU: ";
			waitStatusText += GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(curJobConverted->type);

			if(NULL != waitDialog)
			{
				waitDialog->SetValue(convertJobQueueSize - jobStackConverted.size());
				waitDialog->SetMessage(waitStatusText);
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
				waitDialog->Reset();
			}
		}
	}
	else
	{
		emit ConvertStatusQueue(convertJobQueueSize - jobStackConverted.size(), convertJobQueueSize);

		if(NULL != waitDialog)
		{
			waitDialog->SetRangeMax(convertJobQueueSize);
		}
	}
}

void TextureConvertor::threadOriginalFinished()
{
	if(originalWatcher.isFinished() && NULL != curJobOriginal)
	{
		const DAVA::TextureDescriptor *originalDescriptor = (DAVA::TextureDescriptor *) curJobOriginal->identity;
		DAVA::TextureDescriptor *descriptor = (DAVA::TextureDescriptor *) curJobOriginal->data;

		DAVA::Vector<QImage> watcherResult = originalWatcher.result();
		emit ReadyOriginal(originalDescriptor, watcherResult);

		SafeRelease(descriptor);
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

		DAVA::Vector<QImage> watcherResult = convertedWatcher.result();
		emit ReadyConverted(convertedDescriptor, (DAVA::eGPUFamily) curJobConverted->type, watcherResult);

		SafeRelease(descriptor);
		delete curJobConverted;
		curJobConverted = NULL;
	}

	jobRunNextConvert();
}

void TextureConvertor::waitCanceled()
{
	CancelConvert();
}

DAVA::Vector<QImage> TextureConvertor::GetOriginalThread(JobItem *item)
{
	DAVA::Vector<QImage> resultArray;
    void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();
    
	if(NULL != item && NULL != item->data)
	{
		TextureDescriptor *descriptor = (TextureDescriptor *) item->data;
		
		if(descriptor->IsCubeMap())
		{
			DAVA::Vector<DAVA::String> cubeFaceNames;
			DAVA::Texture::GenerateCubeFaceNames(descriptor->GetSourceTexturePathname().GetAbsolutePathname().c_str(), cubeFaceNames);
			
			for(int i = 0; i < DAVA::Texture::CUBE_FACE_MAX_COUNT; ++i)
			{
				if((descriptor->faceDescription & (1 << i)) != 0)
				{
					QImage img;
					img = QImage(cubeFaceNames[i].c_str());
					resultArray.push_back(img);
				}
			}
		}
		else
		{
			QImage img;
			img = QImage(descriptor->GetSourceTexturePathname().GetAbsolutePathname().c_str());
			resultArray.push_back(img);
		}
	}

    DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
	
	return resultArray;
}

DAVA::Vector<QImage> TextureConvertor::GetConvertedThread(JobItem *item)
{
	void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();

	DAVA::Vector<QImage> ret;
	DAVA::Vector<DAVA::Image*> convertedImages;
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
				DAVA::Logger::FrameworkDebug("Starting PVR conversion (%s), id %d...",
					GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(descriptor->compression[gpu].format), item->id);
				convertedImages = ConvertFormat(descriptor, gpu, item->force);
				DAVA::Logger::FrameworkDebug("Done, id %d", item->id);
			}
			else if(outExtension == ".dds")
			{
				DAVA::Logger::FrameworkDebug("Starting DXT conversion (%s), id %d...",
					GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(descriptor->compression[gpu].format), item->id);
				convertedImages = ConvertFormat(descriptor, gpu, item->force);
				DAVA::Logger::FrameworkDebug("Done, id %d", item->id);
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

	if(convertedImages.size() > 0)
	{
		for(size_t i = 0; i < convertedImages.size(); ++i)
		{
			if(convertedImages[i] != NULL)
			{
				QImage img = FromDavaImage(convertedImages[i]);
				ret.push_back(img);
			
				convertedImages[i]->Release();
			}
			else
			{
				QImage img;
				ret.push_back(img);
			}
		}
	}
	else
	{
		int stubImageCount = Texture::CUBE_FACE_MAX_COUNT;
		if(NULL != item)
		{
			DAVA::TextureDescriptor *descriptor = (DAVA::TextureDescriptor*) item->data;
			if(NULL != descriptor &&
			   !descriptor->IsCubeMap())
			{
				stubImageCount = 1;
			}
		}
		
		for(int i = 0; i < stubImageCount; ++i)
		{
			QImage img;
			ret.push_back(img);
		}
	}

	DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
	
	return ret;
}

DAVA::Vector<DAVA::Image*> TextureConvertor::ConvertFormat(DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu, bool forceConvert)
{
	DAVA::Vector<DAVA::Image*> resultImages;
	DAVA::FilePath outputPath = TextureConverter::GetOutputPath(*descriptor, gpu);
	if(!outputPath.IsEmpty())
	{
		if(forceConvert || !DAVA::FileSystem::Instance()->IsFile(outputPath))
		{
			TextureConverter::CleanupOldTextures(descriptor, gpu, (DAVA::PixelFormat)descriptor->compression[gpu].format);
			outputPath = TextureConverter::ConvertTexture(*descriptor, gpu, true);
        }
		
		Vector<DAVA::Image *> davaImages = DAVA::ImageLoader::CreateFromFile(outputPath);
		
		if(davaImages.size() > 0)
		{
			if(!descriptor->IsCubeMap())
			{
				DAVA::Image* image = davaImages[0];
				image->Retain();
				
				resultImages.push_back(image);
			}
			else
			{
				//select images with mipmap level = 0 for cube map display
				for(size_t i = 0; i < davaImages.size(); ++i)
				{
					DAVA::Image* image = davaImages[i];
					if(0 == image->mipmapLevel)
					{
						image->Retain();
						resultImages.push_back(image);
					}
				}
				
				if(resultImages.size() < Texture::CUBE_FACE_MAX_COUNT)
				{
					int imagesToAdd = Texture::CUBE_FACE_MAX_COUNT - resultImages.size();
					for(int i = 0; i < imagesToAdd; ++i)
					{
						resultImages.push_back(NULL);
					}
				}
			}
			
			for_each(davaImages.begin(), davaImages.end(),  DAVA::SafeRelease<DAVA::Image>);
		}
		else
		{
			int stubImageCount = (descriptor->IsCubeMap()) ? Texture::CUBE_FACE_MAX_COUNT : 1;
			for(int i = 0; i < stubImageCount; ++i)
			{
				resultImages.push_back(NULL);
			}
		}
	}
	
	return resultImages;
}

/*DAVA::Vector<DAVA::Image*> TextureConvertor::ConvertPVR(DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu, bool forceConvert)
 {
 DAVA::Vector<DAVA::Image*> resultImages;
 DAVA::FilePath compressedTexturePath = DAVA::GPUFamilyDescriptor::CreatePathnameForGPU(descriptor, gpu);
 DAVA::FilePath outputPath = PVRConverter::Instance()->GetPVRToolOutput(*descriptor, gpu);
 if(!outputPath.IsEmpty())
 {
 if(forceConvert || !DAVA::FileSystem::Instance()->IsFile(outputPath))
 {
 DeleteOldPVRTextureIfPowerVr_IOS(descriptor, gpu);
 
 DAVA::FilePath pathToConvert = (descriptor->IsCubeMap()) ? PrepareCubeMapForConvert(*descriptor) : FilePath::CreateWithNewExtension(descriptor->pathname, ".png");
 
 QString command = PVRConverter::Instance()->GetCommandLinePVR(*descriptor, pathToConvert, gpu).c_str();
 DAVA::Logger::FrameworkDebug("%s", command.toStdString().c_str());
 
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
 
 CleanupCubemapAfterConversion(*descriptor);
 }
 
 Vector<DAVA::Image *> davaImages = DAVA::ImageLoader::CreateFromFile(outputPath);
 
 if(davaImages.size() > 0)
 {
 if(!descriptor->IsCubeMap())
 {
 DAVA::Image* image = davaImages[0];
 image->Retain();
 
 resultImages.push_back(image);
 }
 else
 {
 //select images with mipmap level = 0 for cube map display
 for(size_t i = 0; i < davaImages.size(); ++i)
 {
 DAVA::Image* image = davaImages[i];
 if(0 == image->mipmapLevel)
 {
 image->Retain();
 resultImages.push_back(image);
 }
 }
 
 if(resultImages.size() < Texture::CUBE_FACE_MAX_COUNT)
 {
 int imagesToAdd = Texture::CUBE_FACE_MAX_COUNT - resultImages.size();
 for(int i = 0; i < imagesToAdd; ++i)
 {
 resultImages.push_back(NULL);
 }
 }
 }
 
 for_each(davaImages.begin(), davaImages.end(),  DAVA::SafeRelease<DAVA::Image>);
 }
 else
 {
 int stubImageCount = (descriptor->IsCubeMap()) ? Texture::CUBE_FACE_MAX_COUNT : 1;
 for(int i = 0; i < stubImageCount; ++i)
 {
 resultImages.push_back(NULL);
 }
 }
 }
 
 return resultImages;
 }
 
 DAVA::Vector<DAVA::Image*> TextureConvertor::ConvertDXT(DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu, bool forceConvert)
 {
 DAVA::Vector<DAVA::Image*> images;
 DAVA::FilePath outputPath = DXTConverter::GetDXTOutput(*descriptor, gpu);
 if(!outputPath.IsEmpty())
 {
 if(forceConvert || !DAVA::FileSystem::Instance()->IsFile(outputPath))
 {
 DeleteOldDXTTextureIfTegra(descriptor, gpu);
 
 if(descriptor->IsCubeMap())
 {
 outputPath = DXTConverter::ConvertCubemapPngToDxt(*descriptor, gpu);
 }
 else
 {
 outputPath = DXTConverter::ConvertPngToDxt(*descriptor, gpu);
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
 for(size_t i = 0; i < davaImages.size(); ++i)
 {
 Image* image = davaImages[i];
 
 if(0 == image->mipmapLevel)
 {
 image->Retain();
 images.push_back(image);
 }
 }
 
 if(descriptor->IsCubeMap() &&
 images.size() < Texture::CUBE_FACE_MAX_COUNT)
 {
 int imagesToAdd = Texture::CUBE_FACE_MAX_COUNT - images.size();
 for(int i = 0; i < imagesToAdd; ++i)
 {
 images.push_back(NULL);
 }
 }
 
 for_each(davaImages.begin(), davaImages.end(),  DAVA::SafeRelease< DAVA::Image>);
 }
 else
 {
 int stubImageCount = (descriptor->IsCubeMap()) ? Texture::CUBE_FACE_MAX_COUNT : 1;
 for(int i = 0; i < stubImageCount; ++i)
 {
 images.push_back(NULL);
 }
 }
 }
 else
 {
 int stubImageCount = (descriptor->IsCubeMap()) ? Texture::CUBE_FACE_MAX_COUNT : 1;
 for(int i = 0; i < stubImageCount; ++i)
 {
 images.push_back(NULL);
 }
 }
 
 return images;
 }
*/

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

		case DAVA::FORMAT_RGB888:
			{
				DAVA::uint8 *data = (DAVA::uint8 *) image->data;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGB888 into Qt ARGB8888
				int32 imagewidth = image->width * 3;
				for (int y = 0; y < (int)image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0, i = 0; x < imagewidth; x += 3, i++) 
					{
						DAVA::uint32 a = 0xff000000;
						DAVA::uint32 r = data[y * imagewidth + x];
						DAVA::uint32 g = data[y * imagewidth + x + 1];
						DAVA::uint32 b = data[y * imagewidth + x + 2];

 						line[i] = (a) | (r << 16) | (g << 8) | (b);
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

