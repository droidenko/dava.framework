#include "Qt/TextureBrowser/TextureBrowser.h"
#include "Qt/TextureBrowser/TextureListModel.h"
#include "Qt/TextureBrowser/TextureListDelegate.h"
#include "Qt/TextureBrowser/TextureConvertor.h"
#include "Qt/TextureBrowser/TextureCache.h"
#include "Qt/Main/QtUtils.h"
#include "Qt/Main/mainwindow.h"
#include "Qt/Scene/SceneData.h"
#include "Render/LibPVRHelper.h"
#include "Render/LibDxtHelper.h"
#include "SceneEditor/EditorSettings.h"

#include "ui_texturebrowser.h"

#include <QComboBox>
#include <QAbstractItemModel>
#include <QStatusBar>
#include <QToolButton>
#include <QFileInfo>
#include <QList>
#include <QMessageBox>

TextureBrowser::TextureBrowser(QWidget *parent)
    : QDialog(parent)
	, ui(new Ui::TextureBrowser)
	, curScene(NULL)
	, curTextureView(ViewPVR)
	, curTexture(NULL)
	, curDescriptor(NULL)
{
	ui->setupUi(this);
	setWindowFlags(Qt::Window);

	textureListModel = new TextureListModel();
	textureListImagesDelegate = new TextureListDelegate();

	textureListSortModes["File size"] = TextureListModel::SortByFileSize;
	textureListSortModes["Data size"] = TextureListModel::SortByDataSize;
	textureListSortModes["Image size"] = TextureListModel::SortByImageSize;
	textureListSortModes["Name"] = TextureListModel::SortByName;

	// global scene manager signals
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneActivated(SceneData *)), this, SLOT(sceneActivated(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneChanged(SceneData *)), this, SLOT(sceneChanged(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneReleased(SceneData *)), this, SLOT(sceneReleased(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneNodeSelected(SceneData *, DAVA::Entity *)), this, SLOT(sceneNodeSelected(SceneData *, DAVA::Entity *)));

	// convertor signals
	QObject::connect(TextureConvertor::Instance(), SIGNAL(readyOriginal(const DAVA::TextureDescriptor *, const QImage &)), this, SLOT(textureReadyOriginal(const DAVA::TextureDescriptor *, const QImage &)));
	QObject::connect(TextureConvertor::Instance(), SIGNAL(readyPVR(const DAVA::TextureDescriptor *, const QImage &)), this, SLOT(textureReadyPVR(const DAVA::TextureDescriptor *, const QImage &)));
	QObject::connect(TextureConvertor::Instance(), SIGNAL(readyDXT(const DAVA::TextureDescriptor *, const QImage &)), this, SLOT(textureReadyDXT(const DAVA::TextureDescriptor *, const QImage &)));

	setupStatusBar();
	setupTexturesList();
	setupImagesScrollAreas();
	setupTextureListToolbar();
	setupTextureToolbar();
	setupTextureListFilter();
	setupTextureConverAllButton();
	setupTextureProperties();
	setupTextureViewToolbar();

	resetTextureInfo();

	// let textures list show images-view by default
	ui->actionViewImagesList->trigger();

	// let textures view show border by default
	// ui->actionShowBorder->trigger();

	// set initial empty texture
	setTexture(curTexture, curDescriptor);
	setTextureView(curTextureView);

	// ui->splitter->setSizes(QList<int>() << 60 << 0 << 40);

	posSaver.Attach(this);
	posSaver.LoadState(ui->splitterMain);
}

TextureBrowser::~TextureBrowser()
{
	posSaver.SaveState(ui->splitterMain);

	delete textureListImagesDelegate;
	delete textureListModel;
    delete ui;

	DAVA::SafeRelease(curScene);

	// clear cache
	TextureCache::Instance()->clearAll();
}

void TextureBrowser::closeEvent(QCloseEvent * e)
{
	QDialog::closeEvent(e);
	this->deleteLater();
}

void TextureBrowser::setTexture(DAVA::Texture *texture, DAVA::TextureDescriptor *descriptor)
{
	curTexture = texture;
	curDescriptor = descriptor;

	// color channels to default value
	ui->actionColorR->setChecked(true);
	ui->actionColorG->setChecked(true);
	ui->actionColorB->setChecked(true);
	ui->actionColorA->setChecked(true);
	ui->textureAreaOriginal->setColorChannel(TextureScrollArea::ChannelAll);
	ui->textureAreaPVR->setColorChannel(TextureScrollArea::ChannelAll);

	ui->textureAreaOriginal->resetTexturePosZoom();
	ui->textureAreaPVR->resetTexturePosZoom();
	toolbarZoomSlider->setValue(0);

	// disable texture views by default
	ui->textureToolbar->setEnabled(false);
	ui->textureViewToolbar->setEnabled(false);
	ui->textureAreaOriginal->setEnabled(false);
	ui->textureAreaPVR->setEnabled(false);

	// set texture to properties control.
	// this should be done as a first step
	ui->textureProperties->setTexture(curTexture, curDescriptor);
	updatePropertiesWarning();

	// if texture is ok - set it and enable texture views
	if(NULL != curTexture)
	{
		// enable toolbar
		ui->textureToolbar->setEnabled(true);
		ui->textureViewToolbar->setEnabled(true);

		// load original image
		// check if image is in cache
		QImage img = TextureCache::Instance()->getOriginal(curDescriptor);
		if(!img.isNull())
		{
			// image is in cache, so set it imidiatly (by calling image-ready slot)
			textureReadyOriginal(curDescriptor, img);
		}
		else
		{
			// set empty info
			updateInfoOriginal(QImage());

			// there is no image in cache - start loading it in different thread. image-ready slot will be called 
			ui->textureAreaOriginal->setImage(QImage());
			TextureConvertor::Instance()->loadOriginal(curDescriptor);

			// show loading bar
			ui->textureAreaOriginal->waitbarShow(true);
		}
	}
	else
	{
		// no texture - set empty images to original/PVR views
		ui->textureAreaOriginal->setImage(QImage());
		ui->textureAreaPVR->setImage(QImage());
	}
}

void TextureBrowser::setTextureView(TextureView view, bool forceConvert /* */)
{
	// if force convert - clear cached images
	if(forceConvert)
	{
		TextureCache::Instance()->clearPVR(curDescriptor);
		TextureCache::Instance()->clearDXT(curDescriptor);
	}

	curTextureView = view;

	// first set texture view to default
	ui->actionViewPVR->setChecked(false);
	ui->actionViewDXT->setChecked(false);

	// second set texture view to appropriate state
	if(NULL != curTexture && NULL != curDescriptor)
	{
		// set empty image to converted image view. it will be visible until
		// conversion done (signal by textureConvertor).
		ui->textureAreaPVR->setImage(QImage());
		ui->textureAreaPVR->waitbarShow(true);

		switch(view)
		{
		case ViewPVR:
			{
				ui->actionViewPVR->setChecked(true);

				QImage img = TextureCache::Instance()->getPVR(curDescriptor);
				if(!forceConvert && !img.isNull())
				{
					// image already in cache, just draw it
					updateConvertedImageAndInfo(img);
				}
				else
				{
					// Start PVR convert. Signal will be emited when conversion done
					TextureConvertor::Instance()->getPVR(curDescriptor, forceConvert);
				}
			}
			break;
		case ViewDXT:
			{
				ui->actionViewDXT->setChecked(true);

				QImage img = TextureCache::Instance()->getDXT(curDescriptor);
				if(!forceConvert && !img.isNull())
				{
					// image already in cache, just draw it
					updateConvertedImageAndInfo(img);
				}
				else
				{
					// Start DXT convert. Signal will be emited when conversion done
					TextureConvertor::Instance()->getDXT(curDescriptor, forceConvert);
				}
			}
			break;
		default:
			break;
		}
	}

	updateInfoConverted();
}

void TextureBrowser::updatePropertiesWarning()
{
	if(NULL != curDescriptor && NULL != curTexture)
	{
		QString warningText = "";

		if(
		   ((curDescriptor->pvrCompression.format == DAVA::FORMAT_PVR4 || curDescriptor->pvrCompression.format == DAVA::FORMAT_PVR2)   ||
		   (curDescriptor->dxtCompression.format >= DAVA::FORMAT_DXT1 && curDescriptor->dxtCompression.format <= DAVA::FORMAT_DXT5NM)) &&
		   (curTexture->width != curTexture->height))
		{
			warningText += "WARNING: Not square texture.\n";
		}

		ui->warningLabel->setText(warningText);
	}
}

void TextureBrowser::updateConvertedImageAndInfo(const QImage &image)
{
	ui->textureAreaPVR->setImage(image);
	ui->textureAreaPVR->setEnabled(true);
	ui->textureAreaPVR->waitbarShow(false);

	// set info about converted image
	updateInfoConverted();
}

void TextureBrowser::updateInfoColor(QLabel *label, const QColor &color /* = QColor() */)
{
	if(NULL != label)
	{
		char tmp[1024];

		sprintf(tmp, "R: %02X\nG: %02X\nB: %02X\nA: %02X",
			color.red(), color.green(), color.blue(), color.alpha());

		label->setText(tmp);
	}
}

void TextureBrowser::updateInfoPos(QLabel *label, const QPoint &pos /* = QPoint() */)
{
	if(NULL != label)
	{
		char tmp[1024];

		sprintf(tmp, "X : %d\nY : %d",
			pos.x(), pos.y());

		label->setText(tmp);
	}
}

void TextureBrowser::updateInfoOriginal(const QImage &origImage)
{
	if(NULL != curTexture && NULL != curDescriptor)
	{
		char tmp[1024];

		const char *formatStr = DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA8888);

		int datasize = origImage.width() * origImage.height() * DAVA::Texture::GetPixelFormatSizeInBytes(DAVA::FORMAT_RGBA8888);
		int filesize = QFileInfo(curDescriptor->GetSourceTexturePathname().GetAbsolutePathname().c_str()).size();

		sprintf(tmp, "Format\t: %s\nSize\t: %dx%d\nData size\t: %s\nFile size\t: %s", formatStr, origImage.width(), origImage.height(),
			 SizeInBytesToString(datasize).c_str(),
			 SizeInBytesToString(filesize).c_str());

		ui->labelOriginalFormat->setText(tmp);
	}
	else
	{
		ui->labelOriginalFormat->setText("");
	}
}

void TextureBrowser::updateInfoConverted()
{
	if(NULL != curTexture && NULL != curDescriptor)
	{
		char tmp[1024];

		const char *formatStr = "";

		int datasize = 0;
		int filesize = 0;

		switch(curTextureView)
		{
		case ViewPVR:
			if(curDescriptor->pvrCompression.format != DAVA::FORMAT_INVALID)
			{
				DAVA::FilePath compressedTexturePath = DAVA::TextureDescriptor::GetPathnameForFormat(curTexture->GetPathname(), DAVA::PVR_FILE);

				formatStr = DAVA::Texture::GetPixelFormatString(curDescriptor->pvrCompression.format);
				filesize = QFileInfo(compressedTexturePath.GetAbsolutePathname().c_str()).size();
				datasize = DAVA::LibPVRHelper::GetDataLength(compressedTexturePath);
			}
			break;
		case ViewDXT:
			if(curDescriptor->dxtCompression.format != DAVA::FORMAT_INVALID)
			{
				DAVA::FilePath compressedTexturePath = DAVA::TextureDescriptor::GetPathnameForFormat(curTexture->GetPathname(), DAVA::DXT_FILE);

				formatStr = DAVA::Texture::GetPixelFormatString(curDescriptor->dxtCompression.format);
				filesize = QFileInfo(compressedTexturePath.GetAbsolutePathname().c_str()).size();

				// TODO: more accurate dxt data size calculation
				//datasize = (curTexture->width * curTexture->height * DAVA::Texture::GetPixelFormatSizeInBits(curDescriptor->dxtCompression.format)) >> 3;
				datasize = LibDxtHelper::GetDataSize(compressedTexturePath);
			}
			break;
		}

		sprintf(tmp, "Format\t: %s\nSize\t: %dx%d\nData size\t: %s\nFile size\t: %s", formatStr, curTexture->width, curTexture->height,
			SizeInBytesToString(datasize).c_str(),
			SizeInBytesToString(filesize).c_str());

		ui->labelConvertedFormat->setText(tmp);
	}
	else
	{
		ui->labelConvertedFormat->setText("");
	}
}

void TextureBrowser::setupStatusBar()
{
	statusBar = new QStatusBar(this);
	statusBarLabel = new QLabel();

	statusBar->addWidget(statusBarLabel);
	ui->mainLayout->addWidget(statusBar);

//	QObject::connect(TextureConvertor::Instance(), SIGNAL(convertStatus(const JobItem *, int)), this, SLOT(convertStatus(const JobItem *, int)));
}

void TextureBrowser::setupTextureConverAllButton()
{
	QObject::connect(ui->convertAll, SIGNAL(pressed()), this, SLOT(textureConverAll()));
}

void TextureBrowser::setupTexturesList()
{
	QObject::connect(ui->listViewTextures, SIGNAL(selected(const QModelIndex &)), this, SLOT(texturePressed(const QModelIndex &)));

	ui->listViewTextures->setItemDelegate(textureListImagesDelegate);
	ui->listViewTextures->setModel(textureListModel);
}

void TextureBrowser::setupImagesScrollAreas()
{
	// pos change
	QObject::connect(ui->textureAreaOriginal, SIGNAL(texturePosChanged(const QPoint &)), ui->textureAreaPVR, SLOT(setTexturePos(const QPoint &)));
	QObject::connect(ui->textureAreaPVR, SIGNAL(texturePosChanged(const QPoint &)), ui->textureAreaOriginal, SLOT(setTexturePos(const QPoint &)));

	// mouse over pixel
	QObject::connect(ui->textureAreaOriginal, SIGNAL(mouseOverPixel(const QPoint &)), this, SLOT(texturePixelOver(const QPoint &)));
	QObject::connect(ui->textureAreaPVR, SIGNAL(mouseOverPixel(const QPoint &)), this, SLOT(texturePixelOver(const QPoint &)));

	// mouse wheel
	QObject::connect(ui->textureAreaOriginal, SIGNAL(mouseWheel(int)), this, SLOT(textureAreaWheel(int)));
	QObject::connect(ui->textureAreaPVR, SIGNAL(mouseWheel(int)), this, SLOT(textureAreaWheel(int)));
}

void TextureBrowser::setupTextureToolbar()
{
	QLabel *textureZoomLabel = new QLabel("Zoom: ");

	toolbarZoomSlider = new QSlider();
	toolbarZoomSliderValue = new QLabel();
	toolbarZoomSlider->setOrientation(Qt::Horizontal);
	toolbarZoomSlider->setMaximumWidth(100);
	toolbarZoomSlider->setTracking(true);
	toolbarZoomSlider->setRange(-90, 90);
	toolbarZoomSlider->setTickPosition(QSlider::TicksBelow);
	toolbarZoomSlider->setTickInterval(15);
	toolbarZoomSlider->setSingleStep(5);
	textureZoomSlide(0);

	ui->textureToolbar->addWidget(textureZoomLabel);
	ui->textureToolbar->addWidget(toolbarZoomSlider);
	ui->textureToolbar->addWidget(toolbarZoomSliderValue);
	ui->textureToolbar->addSeparator();

	QObject::connect(ui->actionColorA, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));
	QObject::connect(ui->actionColorB, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));
	QObject::connect(ui->actionColorG, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));
	QObject::connect(ui->actionColorR, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));

	QObject::connect(ui->actionShowBorder, SIGNAL(triggered(bool)), this, SLOT(textureBorderPressed(bool)));
	QObject::connect(ui->actionShowBgMask, SIGNAL(triggered(bool)), this, SLOT(textureBgMaskPressed(bool)));

	QObject::connect(ui->actionZoom100, SIGNAL(triggered(bool)), this, SLOT(textureZoom100(bool)));
	QObject::connect(ui->actionZoomFit, SIGNAL(triggered(bool)), this, SLOT(textureZoomFit(bool)));

	QObject::connect(toolbarZoomSlider, SIGNAL(valueChanged(int)), this, SLOT(textureZoomSlide(int)));
}

void TextureBrowser::setupTextureListToolbar()
{
	QComboBox *texturesSortCombo = new QComboBox();
	QLabel *texturesSortComboLabel = new QLabel();

	QMapIterator<QString, int> i(textureListSortModes);
	while(i.hasNext())
	{
		i.next();
		texturesSortCombo->addItem(i.key());
	}

	texturesSortComboLabel->setText("Sort by: ");

	ui->textureListToolbar->addWidget(texturesSortComboLabel);
	ui->textureListToolbar->addWidget(texturesSortCombo);

	QObject::connect(texturesSortCombo, SIGNAL(activated(const QString &)), this, SLOT(textureListSortChanged(const QString &)));
	QObject::connect(ui->actionViewTextList, SIGNAL(triggered(bool)), this, SLOT(textureListViewText(bool)));
	QObject::connect(ui->actionViewImagesList, SIGNAL(triggered(bool)), this, SLOT(textureListViewImages(bool)));
	QObject::connect(ui->actionFilterSelectedNode, SIGNAL(triggered(bool)), this, SLOT(textureListFilterSelectedNodeChanged(bool)));
}

void TextureBrowser::setupTextureListFilter()
{
	QObject::connect(ui->textureFilterEdit, SIGNAL(textChanged(const QString &)), this, SLOT(textureListFilterChanged(const QString&)));
}

void TextureBrowser::setupTextureProperties()
{
	QObject::connect(ui->textureProperties, SIGNAL(propertyChanged(const int)), this, SLOT(texturePropertyChanged(const int)));

	QPalette palette = ui->warningLabel->palette();
	palette.setColor(ui->warningLabel->foregroundRole(), Qt::red);
	ui->warningLabel->setPalette(palette);
}

void TextureBrowser::setupTextureViewToolbar()
{
	QObject::connect(ui->actionViewPVR, SIGNAL(triggered(bool)), this, SLOT(textureViewPVR(bool)));
	QObject::connect(ui->actionViewDXT, SIGNAL(triggered(bool)), this, SLOT(textureViewDXT(bool)));
}

void TextureBrowser::resetTextureInfo()
{
	updateInfoColor(ui->labelOriginalRGBA);
	updateInfoPos(ui->labelOriginalXY);

	updateInfoColor(ui->labelConvertedRGBA);
	updateInfoPos(ui->labelConvertedXY);

	updateInfoOriginal(QImage());
	updateInfoConverted();
}

void TextureBrowser::reloadTextureToScene(DAVA::Texture *texture, const DAVA::TextureDescriptor *descriptor, DAVA::ImageFileFormat format)
{
	if(NULL != descriptor && NULL != texture)
	{
		DAVA::ImageFileFormat curEditorImageFormatForTextures = (DAVA::ImageFileFormat) EditorSettings::Instance()->GetTextureViewFileFormat();

		// reload only when editor view format is the same as given texture format
		// or if given texture format if not a file (will happened if some common texture params changed - mipmap/filtering etc.)
		if(DAVA::NOT_FILE == format || format == curEditorImageFormatForTextures)
		{
			DAVA::Texture *newTexture = SceneDataManager::Instance()->TextureReload(descriptor, texture, curEditorImageFormatForTextures);

			if(NULL != newTexture)
			{
				// need reset cur texture to newly loaded
				if(curTexture == texture)
				{
					curTexture = newTexture;
				}

				// need to set new Texture into Textures list model
				textureListModel->setTexture(descriptor, newTexture);
			}
		}
	}
}

void TextureBrowser::texturePressed(const QModelIndex & index)
{
	setTexture(textureListModel->getTexture(index), textureListModel->getDescriptor(index));
	setTextureView(curTextureView);

	// zoom fit selected texture
	textureZoomFit(true);
}

void TextureBrowser::textureListViewText(bool checked)
{
	ui->actionViewImagesList->setChecked(false);
	ui->actionViewTextList->setChecked(true);

	textureListImagesDelegate->setDrawRule(TextureListDelegate::DRAW_PREVIEW_SMALL);
	ui->listViewTextures->reset();
}

void TextureBrowser::textureListViewImages(bool checked)
{
	ui->actionViewTextList->setChecked(false);
	ui->actionViewImagesList->setChecked(true);

	textureListImagesDelegate->setDrawRule(TextureListDelegate::DRAW_PREVIEW_BIG);
	ui->listViewTextures->reset();
}

void TextureBrowser::textureListFilterChanged(const QString &text)
{
	textureListModel->setFilter(text);
}

void TextureBrowser::textureListFilterSelectedNodeChanged(bool checked)
{
	textureListModel->setFilterBySelectedNode(checked);
}

void TextureBrowser::textureListSortChanged(const QString &text)
{
	textureListModel->setSortMode((TextureListModel::TextureListSortMode) textureListSortModes[text]);
}

void TextureBrowser::textureColorChannelPressed(bool checked)
{
	int channelsMask = 0;

	if(ui->actionColorA->isChecked()) channelsMask |= TextureScrollArea::ChannelA;
	if(ui->actionColorB->isChecked()) channelsMask |= TextureScrollArea::ChannelB;
	if(ui->actionColorG->isChecked()) channelsMask |= TextureScrollArea::ChannelG;
	if(ui->actionColorR->isChecked()) channelsMask |= TextureScrollArea::ChannelR;

	ui->textureAreaOriginal->setColorChannel(channelsMask);
	ui->textureAreaPVR->setColorChannel(channelsMask);
}

void TextureBrowser::textureBorderPressed(bool checked)
{
	ui->textureAreaPVR->borderShow(checked);
	ui->textureAreaOriginal->borderShow(checked);
}

void TextureBrowser::textureBgMaskPressed(bool checked)
{
	ui->textureAreaPVR->bgmaskShow(checked);
	ui->textureAreaOriginal->bgmaskShow(checked);
}

void TextureBrowser::texturePropertyChanged(const int propGroup)
{
	// settings that need texture to reconvert
	if( (propGroup == TextureProperties::TYPE_PVR && curTextureView == ViewPVR) ||
		(propGroup == TextureProperties::TYPE_DXT && curTextureView == ViewDXT) ||
		(propGroup == TextureProperties::TYPE_COMMON_MIPMAP))
	{
		// set current Texture view and force texture convertion
		// new texture will be applyed to scene after conversion (by signal)
		setTextureView(curTextureView, true);
	}
	// common settings than dont need texture to reconvert
	else if(propGroup == TextureProperties::TYPE_COMMON)
	{
		// common settings
		// new texture can be applyed to scene immediately
		reloadTextureToScene(curTexture, ui->textureProperties->getTextureDescriptor(), DAVA::NOT_FILE);
	}

	// update warning message
	updatePropertiesWarning();
}

void TextureBrowser::textureViewPVR(bool checked)
{
	setTextureView(ViewPVR);
}

void TextureBrowser::textureViewDXT(bool checked)
{
	setTextureView(ViewDXT);
}

void TextureBrowser::textureReadyOriginal(const DAVA::TextureDescriptor *descriptor, const QImage &image)
{
	if(NULL != descriptor)
	{
		// put this image into cache
		TextureCache::Instance()->setOriginal(descriptor, image);

		if(curDescriptor == descriptor)
		{
			ui->textureAreaOriginal->setImage(image);
			ui->textureAreaOriginal->setEnabled(true);
			ui->textureAreaOriginal->waitbarShow(false);

			// set info about original image size info texture properties
			ui->textureProperties->setOriginalImageSize(image.size());

			// set info about loaded image
			updateInfoOriginal(image);
		}
	}
}

void TextureBrowser::textureReadyPVR(const DAVA::TextureDescriptor *descriptor, const QImage &image)
{
	// put this image into cache
	TextureCache::Instance()->setPVR(descriptor, image);

	if(curDescriptor == descriptor && curTextureView == ViewPVR)
	{
		updateConvertedImageAndInfo(image);
	}

	if(NULL != descriptor)
	{
		DAVA::Texture *texture = textureListModel->getTexture(descriptor);
		if(NULL != texture)
		{
			// reload this texture into scene
			reloadTextureToScene(texture, descriptor, DAVA::PVR_FILE);
		}
	}
}

void TextureBrowser::textureReadyDXT(const DAVA::TextureDescriptor *descriptor, const QImage &image)
{
	// put this image into cache
	TextureCache::Instance()->setDXT(descriptor, image);

	if(curDescriptor == descriptor && curTextureView == ViewDXT)
	{
		updateConvertedImageAndInfo(image);
	}

	if(NULL != descriptor)
	{
		DAVA::Texture *texture = textureListModel->getTexture(descriptor);
		if(NULL != texture)
		{
			// reload this texture into scene
			reloadTextureToScene(texture, descriptor, DAVA::DXT_FILE);
		}
	}
}

void TextureBrowser::texturePixelOver(const QPoint &pos)
{
	if(ui->textureAreaOriginal->sceneRect().contains(QPointF(pos.x(), pos.y())))
	{
		QColor origColor = ui->textureAreaOriginal->getPixelColor(pos);
		QColor convColor = ui->textureAreaPVR->getPixelColor(pos);

		updateInfoPos(ui->labelOriginalXY, pos);
		updateInfoPos(ui->labelConvertedXY, pos);

		updateInfoColor(ui->labelOriginalRGBA, origColor);
		updateInfoColor(ui->labelConvertedRGBA, convColor);
	}
	else
	{
		// default pos
		updateInfoPos(ui->labelOriginalXY);
		updateInfoPos(ui->labelConvertedXY);

		// default color
		updateInfoColor(ui->labelOriginalRGBA);
		updateInfoColor(ui->labelConvertedRGBA);
	}
}

void TextureBrowser::textureZoomSlide(int value)
{
	float zoom;
	int v;

	if(value > 0)
	{
		value *= 10;
	}

	zoom = 1.0 + (float) value / 100.0;
	v = 100 + value;

	ui->textureAreaOriginal->setTextureZoom(zoom);
	ui->textureAreaPVR->setTextureZoom(zoom);

	toolbarZoomSliderValue->setText(QString("%1%").arg(v));
}

void TextureBrowser::textureZoomFit(bool checked)
{
	if(NULL != curTexture)
	{
		int w = 0;
		int h = 0;

		if(curTexture == Texture::GetPinkPlaceholder())
		{
			QImage img = ui->textureAreaOriginal->getImage();
			w = img.width();
			h = img.height();
		}
		else
		{
			w = curTexture->width;
			h = curTexture->height;
		}

		if(0 != w && 0 != h)
		{
			int v = 0;
			float needWidth = (float) ui->textureAreaOriginal->width();
			float needHeight = (float) ui->textureAreaOriginal->height();
			float scaleX = needWidth / (float) w;
			float scaleY = needHeight / (float) h;
			float scale = DAVA::Min(scaleX, scaleY);

			v = (int) ((scale - 1.0) * 100.0);

			if(v >= 0)
			{
				v = v / 10;
				v -= v % toolbarZoomSlider->singleStep();
			}
			else
			{
				v -= toolbarZoomSlider->singleStep();
				v -= v % toolbarZoomSlider->singleStep();
			}

			toolbarZoomSlider->setValue(v);
		}
	}
}

void TextureBrowser::textureZoom100(bool checked)
{
	toolbarZoomSlider->setValue(0);
}

void TextureBrowser::textureAreaWheel(int delta)
{
	int v = toolbarZoomSlider->value();
	v += delta / 20;
	v -= v % toolbarZoomSlider->singleStep();
	toolbarZoomSlider->setValue(v);
}

void TextureBrowser::textureConverAll()
{
	QMessageBox msgBox(this);
	msgBox.setText("You chose to convert all textures.");
	msgBox.setInformativeText("This could take a long time. Would you like to continue?");
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Cancel);
	msgBox.setIcon(QMessageBox::Warning);
	int ret = msgBox.exec();

	if(ret == QMessageBox::Ok)
	{
		QtMainWindow::Instance()->TextureCheckConvetAndWait(true);
	}
}

void TextureBrowser::convertStatus(const JobItem *jobCur, int jobLeft)
{
	QString statusText;

	if(NULL != jobCur && NULL != jobCur->descriptor)
	{
		statusText += QString("Converting %1").arg(QString(jobCur->descriptor->pathname.GetAbsolutePathname().c_str()));
	}

	if(jobLeft > 0)
	{
		statusText += QString("; %1 more texture(s) to convert").arg(QString::number(jobLeft));
	}

	statusBarLabel->setText(statusText);
}

void TextureBrowser::setScene(DAVA::Scene *scene)
{
	DAVA::SafeRelease(curScene);
	curScene = DAVA::SafeRetain(scene);

	textureListModel->setScene(curScene);
}

void TextureBrowser::sceneChanged(SceneData *sceneData)
{
	DAVA::Scene *scene = NULL;
	if(NULL != sceneData)
	{
		scene = sceneData->GetScene();

		// reload current scene if it is the same as changed
		// or if there is no current scene now - set it
		//if(scene == curScene || curScene == NULL)
		{
			setScene(scene);
		}
	}
}

void TextureBrowser::sceneActivated(SceneData *sceneData)
{
	DAVA::Scene *scene = NULL;
	if(NULL != sceneData)
	{
		scene = sceneData->GetScene();

		// set new scene
		setScene(scene);
	}
}

void TextureBrowser::sceneReleased(SceneData *sceneData)
{
	DAVA::Scene *scene = NULL;
	if(NULL != sceneData)
	{
		scene = sceneData->GetScene();

		// close current scene
		if(scene == curScene)
		{
			setScene(NULL);
		}
	}
}

void TextureBrowser::sceneNodeSelected(SceneData *sceneData, DAVA::Entity *node)
{
	textureListModel->setHighlight(node);
}
