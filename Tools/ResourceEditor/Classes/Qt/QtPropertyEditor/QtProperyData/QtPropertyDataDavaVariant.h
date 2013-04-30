#ifndef __QT_PROPERTY_DATA_DAVA_VARIANT_H__
#define __QT_PROPERTY_DATA_DAVA_VARIANT_H__

#include "Base/Introspection.h"
#include "Qt/QtPropertyEditor/QtPropertyData.h"

class QtPropertyDataDavaVariant : public QtPropertyData
{
	Q_OBJECT

public:
	QtPropertyDataDavaVariant(const DAVA::VariantType &value);
	virtual ~QtPropertyDataDavaVariant();

	const DAVA::VariantType& GetVariantValue() const;
	void SetVariantValue(const DAVA::VariantType& value);

	virtual void SetIcon(const QIcon &icon);
	virtual QIcon GetIcon();

protected:
	DAVA::VariantType curVariantValue;

	virtual QVariant GetValueInternal();
	virtual void SetValueInternal(const QVariant &value);

	virtual void ChildChanged(const QString &key, QtPropertyData *data);
	virtual void ChildNeedUpdate();

	virtual QWidget* CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option);
	virtual void EditorDoneInternal(QWidget *editor);
    virtual void SetEditorDataInternal(QWidget *editor);

protected slots:
	void ColorOWPressed();

private:
	bool iconCacheIsValid;
	QIcon iconCache;

	void ChildsCreate();
	void ChildsSetFromMe();
	void MeSetFromChilds(const QString &lastChangedChildKey, QtPropertyData *lastChangedChildData);

	QVariant FromKeyedArchive(DAVA::KeyedArchive *archive);
	QVariant FromVector4(const DAVA::Vector4 &vector);
	QVariant FromVector3(const DAVA::Vector3 &vector);
	QVariant FromVector2(const DAVA::Vector2 &vector);
	QVariant FromMatrix4(const DAVA::Matrix4 &matrix);
	QVariant FromMatrix3(const DAVA::Matrix3 &matrix);
	QVariant FromMatrix2(const DAVA::Matrix2 &matrix);
	QVariant FromColor(const DAVA::Color &color);
	QVariant FromAABBox3(const DAVA::AABBox3 &aabbox);

	void ToKeyedArchive(const QVariant &value);
	void ToVector4(const QVariant &value);
	void ToVector3(const QVariant &value);
	void ToVector2(const QVariant &value);
	void ToMatrix4(const QVariant &value);
	void ToMatrix3(const QVariant &value);
	void ToMatrix2(const QVariant &value);
	void ToColor(const QVariant &value);
	void ToAABBox3(const QVariant &value);
};

#endif // __QT_PROPERTY_DATA_DAVA_VARIANT_H__
