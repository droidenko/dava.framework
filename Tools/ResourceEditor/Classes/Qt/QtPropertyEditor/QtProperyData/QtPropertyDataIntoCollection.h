#ifndef __QT_PROPERTY_DATA_INTRO_COLLECTION_H__
#define __QT_PROPERTY_DATA_INTRO_COLLECTION_H__

#include "Base/Introspection.h"
#include "Qt/QtPropertyEditor/QtPropertyData.h"

class QtPropertyDataIntroCollection : public QtPropertyData
{
public:
	QtPropertyDataIntroCollection(void *_object, const DAVA::IntrospectionCollection *_collection, int hasAnyFlags = DAVA::INTROSPECTION_ALL, int hasNotAnyFlags = 0);
	virtual ~QtPropertyDataIntroCollection();

protected:
	void *object;
	const DAVA::IntrospectionCollection *collection;

	//QMap<QtPropertyDataDavaVariant*, int> childVariantIndexes;

	virtual QVariant GetValueInternal();
	//virtual void ChildChanged(const QString &key, QtPropertyData *data);
	//virtual void ChildNeedUpdate();
};

#endif // __QT_PROPERTY_DATA_INTRO_COLLECTION_H__
