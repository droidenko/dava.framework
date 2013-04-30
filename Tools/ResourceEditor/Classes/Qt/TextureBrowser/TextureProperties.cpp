#include "Qt/TextureBrowser/TextureProperties.h"
#include "Qt/QtPropertyBrowser/qtpropertymanager.h"
#include "Qt/QtPropertyBrowser/qteditorfactory.h"

#include "Render/TextureDescriptor.h"

TextureProperties::TextureProperties(QWidget *parent /* = 0 */)
	: QtGroupBoxPropertyBrowser(parent)//QtTreePropertyBrowser(parent)
	, curTexture(NULL)
	, curTextureDescriptor(NULL)
	, reactOnPropertyChange(true)
	, texturePropertiesChanged(false)
{
	// initialize list with string for different comboboxs
	{
		helperPVRFormats.push_back("None", DAVA::FORMAT_INVALID);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA8888), DAVA::FORMAT_RGBA8888);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA5551), DAVA::FORMAT_RGBA5551);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA4444), DAVA::FORMAT_RGBA4444);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGB565), DAVA::FORMAT_RGB565);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_A8), DAVA::FORMAT_A8);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_PVR4), DAVA::FORMAT_PVR4);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_PVR2), DAVA::FORMAT_PVR2);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_ETC1), DAVA::FORMAT_ETC1);


		helperDXTFormats.push_back("None", DAVA::FORMAT_INVALID);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA8888), DAVA::FORMAT_RGBA8888);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_DXT1), DAVA::FORMAT_DXT1);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_DXT1NM), DAVA::FORMAT_DXT1NM);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_DXT1A), DAVA::FORMAT_DXT1A);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_DXT3), DAVA::FORMAT_DXT3);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_DXT5), DAVA::FORMAT_DXT5);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_DXT5NM), DAVA::FORMAT_DXT5NM);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_ATC_RGB), DAVA::FORMAT_ATC_RGB);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_ATC_RGBA_EXPLICIT_ALPHA), DAVA::FORMAT_ATC_RGBA_EXPLICIT_ALPHA);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_ATC_RGBA_INTERPOLATED_ALPHA), DAVA::FORMAT_ATC_RGBA_INTERPOLATED_ALPHA);

		helperWrapModes.push_back("Clamp", DAVA::Texture::WRAP_CLAMP_TO_EDGE);
		helperWrapModes.push_back("Repeat", DAVA::Texture::WRAP_REPEAT);

		helperMinGLModes.push_back("Nearest", DAVA::Texture::FILTER_NEAREST);
		helperMinGLModes.push_back("Linear", DAVA::Texture::FILTER_LINEAR);

		helperMinGLModesWithMipmap.push_back("Nearest", DAVA::Texture::FILTER_NEAREST);
		helperMinGLModesWithMipmap.push_back("Linear", DAVA::Texture::FILTER_LINEAR);
		helperMinGLModesWithMipmap.push_back("Nearest, Mipmap Nearest", DAVA::Texture::FILTER_NEAREST_MIPMAP_NEAREST);
		helperMinGLModesWithMipmap.push_back("Linear, Mipmap Nearest", DAVA::Texture::FILTER_LINEAR_MIPMAP_NEAREST);
		helperMinGLModesWithMipmap.push_back("Nearest, Mipmap Linear", DAVA::Texture::FILTER_NEAREST_MIPMAP_LINEAR);
		helperMinGLModesWithMipmap.push_back("Linear, Mipmap Linear", DAVA::Texture::FILTER_LINEAR_MIPMAP_LINEAR);

		helperMagGLModes.push_back("Nearest", DAVA::Texture::FILTER_NEAREST);
		helperMagGLModes.push_back("Linear", DAVA::Texture::FILTER_LINEAR);
	}

	// parent widget
	oneForAllParent = new QWidget();

	// property managers
	propertiesGroup = new QtGroupPropertyManager(oneForAllParent);
	propertiesInt = new QtIntPropertyManager(oneForAllParent);
	propertiesBool = new QtBoolPropertyManager(oneForAllParent);
	propertiesEnum = new QtEnumPropertyManager(oneForAllParent);
	propertiesString = new QtStringPropertyManager(oneForAllParent);

	// property editors
	editorInt = new QtSpinBoxFactory(oneForAllParent);
	editorBool = new QtCheckBoxFactory(oneForAllParent);
	editorString = new QtLineEditFactory(oneForAllParent);
	editorEnum = new QtEnumEditorFactory(oneForAllParent);

	// setup property managers with appropriate property editors
	setFactoryForManager(propertiesInt, editorInt);
	setFactoryForManager(propertiesBool, editorBool);
	setFactoryForManager(propertiesEnum, editorEnum);
	setFactoryForManager(propertiesString, editorString);

	// Adding properties

	// groups
	QtProperty* groupPVR = propertiesGroup->addProperty("PVR");
	QtProperty* groupDXT = propertiesGroup->addProperty("DXT");
	QtProperty* groupCommon = propertiesGroup->addProperty("Common");

	// PVR group
	enumPVRFormat = propertiesEnum->addProperty("Format");
	propertiesEnum->setEnumNames(enumPVRFormat, helperPVRFormats.keyList());

	enumBasePVRMipmapLevel = propertiesEnum->addProperty("Base Mipmap level");

	groupPVR->addSubProperty(enumPVRFormat);
	groupPVR->addSubProperty(enumBasePVRMipmapLevel);
	addProperty(groupPVR);

	// DXT group
	enumDXTFormat = propertiesEnum->addProperty("Format");
	propertiesEnum->setEnumNames(enumDXTFormat, helperDXTFormats.keyList());

	enumBaseDXTMipmapLevel = propertiesEnum->addProperty("Base Mipmap level");

	groupDXT->addSubProperty(enumDXTFormat);
	groupDXT->addSubProperty(enumBaseDXTMipmapLevel);
	addProperty(groupDXT);

	// Common group

	// Mip maps
	boolGenerateMipMaps = propertiesBool->addProperty("Generate MipMaps");
	groupCommon->addSubProperty(boolGenerateMipMaps);

	// wrapmode t
	enumWrapModeS = propertiesEnum->addProperty("Wrap mode S");
	propertiesEnum->setEnumNames(enumWrapModeS, helperWrapModes.keyList());
	groupCommon->addSubProperty(enumWrapModeS);

	// wrapmode s
	enumWrapModeT = propertiesEnum->addProperty("Wrap mode T");
	propertiesEnum->setEnumNames(enumWrapModeT, helperWrapModes.keyList());
	groupCommon->addSubProperty(enumWrapModeT);

	// min OpenGL filter
	enumMinGL = propertiesEnum->addProperty("Min Filter");
	propertiesEnum->setEnumNames(enumMinGL, helperMinGLModes.keyList());
	groupCommon->addSubProperty(enumMinGL);

	// mag OpenGl filter
	enumMagGL = propertiesEnum->addProperty("Mag Filter");
	propertiesEnum->setEnumNames(enumMagGL, helperMagGLModes.keyList());
	groupCommon->addSubProperty(enumMagGL);

	addProperty(groupCommon);

	QObject::connect(propertiesEnum, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	QObject::connect(propertiesInt, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	QObject::connect(propertiesBool, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	QObject::connect(propertiesString, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
}

TextureProperties::~TextureProperties()
{
	Save();
	DAVA::SafeRelease(curTexture);
	DAVA::SafeRelease(curTextureDescriptor);

	delete oneForAllParent;
}

void TextureProperties::setTexture(DAVA::Texture *texture, DAVA::TextureDescriptor *descriptor)
{
	reactOnPropertyChange = false;

	Save();
	DAVA::SafeRelease(curTexture);
	DAVA::SafeRelease(curTextureDescriptor);

	curTexture = DAVA::SafeRetain(texture);
	curTextureDescriptor = DAVA::SafeRetain(descriptor);
	origImageSize = QSize(0, 0);

	if(NULL != curTexture && NULL != curTextureDescriptor)
	{
		// enable this widget
		setEnabled(true);

		// reset mimmap sizes
		// we don't know avaliable mipmap sizes for newly set texture until setOriginalSize() method will be called by user
		MipMapSizesReset();

		// set loaded descriptor to current properties
		{
			// pvr
			QSize curPVRSize(curTextureDescriptor->pvrCompression.compressToWidth, curTextureDescriptor->pvrCompression.compressToHeight);
			propertiesEnum->setValue(enumPVRFormat, helperPVRFormats.indexV(curTextureDescriptor->pvrCompression.format));
			propertiesEnum->setEnumNames(enumBasePVRMipmapLevel, helperMipMapSizes.keyList());
			propertiesEnum->setValue(enumBasePVRMipmapLevel, helperMipMapSizes.indexV(curPVRSize));

			// dxt
			QSize curDXTSize(curTextureDescriptor->dxtCompression.compressToWidth, curTextureDescriptor->dxtCompression.compressToHeight);
			propertiesEnum->setValue(enumDXTFormat, helperDXTFormats.indexV(curTextureDescriptor->dxtCompression.format));
			propertiesEnum->setEnumNames(enumBaseDXTMipmapLevel, helperMipMapSizes.keyList());
			propertiesEnum->setValue(enumBaseDXTMipmapLevel, helperMipMapSizes.indexV(curDXTSize));

			// mipmap
			propertiesBool->setValue(boolGenerateMipMaps, curTextureDescriptor->generateMipMaps);

			// wrap mode
			propertiesEnum->setValue(enumWrapModeS, helperWrapModes.indexV(curTextureDescriptor->wrapModeS));
			propertiesEnum->setValue(enumWrapModeT, helperWrapModes.indexV(curTextureDescriptor->wrapModeT));

			// min gl filter
			MinFilterCustomSetup();

			// mag lg filter
			propertiesEnum->setValue(enumMagGL, helperMagGLModes.indexV(curTextureDescriptor->magFilter));

		}
	}
	else
	{
		// no texture - disable this widget
		setEnabled(false);
	}

	// mark this new properties not changed
	texturePropertiesChanged = false;

	reactOnPropertyChange = true;
}

void TextureProperties::setOriginalImageSize(const QSize &size)
{
	reactOnPropertyChange = false;

	origImageSize = size;

	// Init mimmap sizes based on original image size
	MipMapSizesInit(size.width(), size.height());

	if(NULL != curTextureDescriptor)
	{
		// reload values into PVR property
		QSize curPVRSize(curTextureDescriptor->pvrCompression.compressToWidth, curTextureDescriptor->pvrCompression.compressToHeight);
		propertiesEnum->setEnumNames(enumBasePVRMipmapLevel, helperMipMapSizes.keyList());
		propertiesEnum->setValue(enumBasePVRMipmapLevel, helperMipMapSizes.indexV(curPVRSize));

		// reload values into DXT property
		QSize curDXTSize(curTextureDescriptor->dxtCompression.compressToWidth, curTextureDescriptor->dxtCompression.compressToHeight);
		propertiesEnum->setEnumNames(enumBaseDXTMipmapLevel, helperMipMapSizes.keyList());
		propertiesEnum->setValue(enumBaseDXTMipmapLevel, helperMipMapSizes.indexV(curDXTSize));
	}

	reactOnPropertyChange = true;
}

const DAVA::Texture* TextureProperties::getTexture()
{
	return curTexture;
}

const DAVA::TextureDescriptor* TextureProperties::getTextureDescriptor()
{
	return curTextureDescriptor;
}

void TextureProperties::propertyChanged(QtProperty * property)
{
	if(reactOnPropertyChange && NULL != curTextureDescriptor && NULL != curTexture)
	{
		PropertiesType type = TYPE_COMMON;

		// mark current properties changed
		texturePropertiesChanged = true;

		if(property == enumPVRFormat)
		{
			DAVA::PixelFormat newPVRFormat = (DAVA::PixelFormat) helperPVRFormats.value(enumPVRFormat->valueText());
			curTextureDescriptor->pvrCompression.format = newPVRFormat;
			type = TYPE_PVR;
		}
		else if(property == enumDXTFormat)
		{
			DAVA::PixelFormat newDXTFormat = (DAVA::PixelFormat) helperDXTFormats.value(enumDXTFormat->valueText());
			curTextureDescriptor->dxtCompression.format = newDXTFormat;
			type = TYPE_DXT;
		}
		else if(property == enumBasePVRMipmapLevel)
		{
			QString curSizeStr = enumBasePVRMipmapLevel->valueText();
			QSize size = helperMipMapSizes.value(curSizeStr);

			if(size.width() == origImageSize.width() || size.height() == origImageSize.height())
			{
				curTextureDescriptor->pvrCompression.compressToWidth = 0;
				curTextureDescriptor->pvrCompression.compressToHeight = 0;
			}
			else
			{
				curTextureDescriptor->pvrCompression.compressToWidth = size.width();
				curTextureDescriptor->pvrCompression.compressToHeight = size.height();
			}
			type = TYPE_PVR;
		}
		else if(property == enumBaseDXTMipmapLevel)
		{
			QString curSizeStr = enumBaseDXTMipmapLevel->valueText();
			QSize size = helperMipMapSizes.value(curSizeStr);

			if(size.width() == origImageSize.width() || size.height() == origImageSize.height())
			{
				curTextureDescriptor->dxtCompression.compressToWidth = 0;
				curTextureDescriptor->dxtCompression.compressToHeight = 0;
			}
			else
			{
				curTextureDescriptor->dxtCompression.compressToWidth = size.width();
				curTextureDescriptor->dxtCompression.compressToHeight = size.height();
			}
			type = TYPE_DXT;
		}
		else if(property == boolGenerateMipMaps)
		{
			curTextureDescriptor->generateMipMaps = (int) propertiesBool->value(boolGenerateMipMaps);
			MinFilterCustomSetup();

			type = TYPE_COMMON_MIPMAP;
		}
		else if(property == enumWrapModeS)
		{
			curTextureDescriptor->wrapModeS = (DAVA::Texture::TextureWrap) helperWrapModes.value(enumWrapModeS->valueText());
		}
		else if(property == enumWrapModeS)
		{
			curTextureDescriptor->wrapModeT = (DAVA::Texture::TextureWrap) helperWrapModes.value(enumWrapModeT->valueText());
		}
		else if(property == enumMinGL)
		{
			if(curTextureDescriptor->generateMipMaps)
			{
				curTextureDescriptor->minFilter = helperMinGLModesWithMipmap.value(enumMinGL->valueText());
			}
			else
			{
				curTextureDescriptor->minFilter = helperMinGLModes.value(enumMinGL->valueText());
			}
		}
		else if(property == enumMagGL)
		{
			curTextureDescriptor->magFilter = helperMinGLModes.value(enumMagGL->valueText());
		}

		emit propertyChanged(type);
	}
}

void TextureProperties::Save()
{
	if(NULL != curTextureDescriptor && true == texturePropertiesChanged)
	{
		curTextureDescriptor->Save();
	}
}

void TextureProperties::MipMapSizesInit(int baseWidth, int baseHeight)
{
	int level = 0;

	helperMipMapSizes.clear();
	while(baseWidth > 1 && baseHeight > 1)
	{
		QSize size(baseWidth, baseHeight);
		QString shownKey;

		shownKey.sprintf("%d - %dx%d", level, baseWidth, baseHeight);
		helperMipMapSizes.push_back(shownKey, size);

		level++;
		baseWidth = baseWidth >> 1;
		baseHeight = baseHeight >> 1;
	}

	enumBaseDXTMipmapLevel->setEnabled(true);
	enumBasePVRMipmapLevel->setEnabled(true);
}

void TextureProperties::MipMapSizesReset()
{
	helperMipMapSizes.clear();

	enumBaseDXTMipmapLevel->setEnabled(false);
	enumBasePVRMipmapLevel->setEnabled(false);
}

void TextureProperties::resetCommonProp()
{
	if(NULL != curTextureDescriptor)
	{
		curTextureDescriptor->SetDefaultValues();

		// mipmap
		propertiesBool->setValue(boolGenerateMipMaps, curTextureDescriptor->generateMipMaps);

		// wrap mode
		propertiesEnum->setValue(enumWrapModeS, helperWrapModes.indexV(curTextureDescriptor->wrapModeS));
		propertiesEnum->setValue(enumWrapModeT, helperWrapModes.indexV(curTextureDescriptor->wrapModeT));

		// min gl filter
        if(curTextureDescriptor->generateMipMaps)
        {
            propertiesEnum->setValue(enumMinGL, helperMinGLModesWithMipmap.indexV(curTextureDescriptor->minFilter));
        }
        else
        {
            propertiesEnum->setValue(enumMinGL, helperMinGLModes.indexV(curTextureDescriptor->minFilter));
        }
		propertiesEnum->setValue(enumMagGL, helperMagGLModes.indexV(curTextureDescriptor->magFilter));
	}
}

void TextureProperties::MinFilterCustomSetup()
{
	reactOnPropertyChange = false;

	if(curTextureDescriptor->generateMipMaps)
	{
		propertiesEnum->setEnumNames(enumMinGL, helperMinGLModesWithMipmap.keyList());
		propertiesEnum->setValue(enumMinGL, helperMinGLModesWithMipmap.indexV(curTextureDescriptor->minFilter));
	}
	else
	{
		if(-1 == helperMinGLModes.indexV(curTextureDescriptor->minFilter))
		{
			curTextureDescriptor->minFilter =  DAVA::Texture::FILTER_LINEAR;
		}

		propertiesEnum->setEnumNames(enumMinGL, helperMinGLModes.keyList());
		propertiesEnum->setValue(enumMinGL, helperMinGLModes.indexV(curTextureDescriptor->minFilter));
	}

	reactOnPropertyChange = true;
}
