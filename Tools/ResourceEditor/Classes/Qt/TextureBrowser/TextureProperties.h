#ifndef __TEXTURE_PROPERTIES_H__
#define __TEXTURE_PROPERTIES_H__

#include "DAVAEngine.h"
#include "Qt/QtPropertyBrowser/qttreepropertybrowser.h"
#include "Qt/QtPropertyBrowser/qtgroupboxpropertybrowser.h"

#include <QMap>
#include <QSize>

class QtGroupPropertyManager;
class QtIntPropertyManager;
class QtBoolPropertyManager;
class QtEnumPropertyManager;
class QtStringPropertyManager;

class QtSpinBoxFactory;
class QtCheckBoxFactory;
class QtLineEditFactory;
class QtEnumEditorFactory;

class TextureProperties : public QtGroupBoxPropertyBrowser//QtTreePropertyBrowser
{
	Q_OBJECT

public:
	TextureProperties(QWidget *parent = 0);
	~TextureProperties();

	typedef enum PropertiesType
	{
		TYPE_COMMON,
		TYPE_COMMON_MIPMAP,
		TYPE_PVR,
		TYPE_DXT
	} PropertiesType;

	void setTexture(DAVA::Texture *texture, DAVA::TextureDescriptor *descriptor);
	void setOriginalImageSize(const QSize &size);

	const DAVA::Texture* getTexture();
	const DAVA::TextureDescriptor* getTextureDescriptor();

public slots:
	void resetCommonProp();

signals:
	void propertyChanged(const int propGroup);

private slots:
	void propertyChanged(QtProperty * property);

private:
	template<typename T>
	struct enumPropertiesHelper
	{
		T value(const QString &key)
		{
			int i = keys.indexOf(key);

			if(i != -1)
			{
				return values[i];
			}

			return T();
		}

		void push_back(const QString &key, const T &value)
		{
			keys.push_back(key);
			values.push_back(value);
		}

		void clear()
		{
			keys.clear();
			values.clear();
		}

		int indexK(const QString &key)
		{
			return keys.indexOf(key);
		}

		int indexV(const T &value)
		{
			return values.indexOf(value);
		}

		QStringList keyList()
		{
			QStringList ret;

			for(int i = 0; i < keys.count(); ++i)
			{
				ret.append(keys[i]);
			}

			return ret;
		}

	private:
		QVector<QString> keys;
		QVector<T> values;
	};

	QWidget *oneForAllParent;

	enumPropertiesHelper<int> helperPVRFormats;
	enumPropertiesHelper<int> helperDXTFormats;
	enumPropertiesHelper<int> helperWrapModes;
	enumPropertiesHelper<int> helperMinGLModes;
	enumPropertiesHelper<int> helperMagGLModes;
	enumPropertiesHelper<int> helperMinGLModesWithMipmap;
	enumPropertiesHelper<QSize> helperMipMapSizes;

	DAVA::Texture *curTexture;
	DAVA::TextureDescriptor *curTextureDescriptor;
	QSize origImageSize;

	bool texturePropertiesChanged;
	bool reactOnPropertyChange;

	QtGroupPropertyManager *propertiesGroup;
	QtIntPropertyManager *propertiesInt;
	QtBoolPropertyManager *propertiesBool;
	QtEnumPropertyManager *propertiesEnum;
	QtStringPropertyManager *propertiesString;

	QtSpinBoxFactory *editorInt;
	QtCheckBoxFactory *editorBool;
	QtLineEditFactory *editorString;
	QtEnumEditorFactory *editorEnum;

	QtProperty *enumPVRFormat;
	QtProperty *enumBasePVRMipmapLevel;

	QtProperty *enumDXTFormat;
	QtProperty *enumBaseDXTMipmapLevel;

	QtProperty *boolGenerateMipMaps;
	QtProperty *enumWrapModeS;
	QtProperty *enumWrapModeT;
	QtProperty *enumMinGL;
	QtProperty *enumMagGL;

	void Save();

	void MinFilterCustomSetup();
	void MipMapSizesInit(int baseWidth, int baseHeight);
	void MipMapSizesReset();
};

#endif // __TEXTURE_PROPERTIES_H__
