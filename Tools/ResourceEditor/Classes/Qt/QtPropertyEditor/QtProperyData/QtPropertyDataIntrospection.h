#ifndef __QT_PROPERTY_DATA_INTROSPECTION_H__
#define __QT_PROPERTY_DATA_INTROSPECTION_H__

#include "Base/Introspection.h"
#include "Qt/QtPropertyEditor/QtPropertyData.h"

#include <QMap>

class QtPropertyDataDavaVariant;

class QtPropertyDataIntrospection : public QtPropertyData
{
	Q_OBJECT
public:
	QtPropertyDataIntrospection(void *object, const DAVA::IntrospectionInfo *info, int hasAnyFlags = DAVA::INTROSPECTION_ALL, int hasNotAnyFlags = 0);
	virtual ~QtPropertyDataIntrospection();

protected:
	void *object;
	const DAVA::IntrospectionInfo *info;
	QMap<QtPropertyDataDavaVariant*, const DAVA::IntrospectionMember *> childVariantMembers;

	void AddMember(const DAVA::IntrospectionMember *member, int hasAnyFlags, int hasNotAnyFlags);

	virtual QVariant GetValueInternal();
	virtual void ChildChanged(const QString &key, QtPropertyData *data);
	virtual void ChildNeedUpdate();

	DAVA_DEPRECATED(void CreateCustomButtonsForRenderObject());

protected slots:
	void BakeTransform();
};

#endif // __QT_PROPERTY_DATA_INTROSPECTION_H__
