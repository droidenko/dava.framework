#include "Qt/QtPropertyEditor/QtProperyData/QtPropertyDataIntoCollection.h"
#include "Qt/QtPropertyEditor/QtProperyData/QtPropertyDataIntrospection.h"
#include "Qt/QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"

QtPropertyDataIntroCollection::QtPropertyDataIntroCollection(void *_object, const DAVA::IntrospectionCollection *_collection, int hasAnyFlags, int hasNotAnyFlags)
	: object(_object)
	, collection(_collection)
{
	if(NULL != collection && collection->Size(object) > 0)
	{
		int index = 0;
		DAVA::MetaInfo *valueType = collection->ItemType();
		DAVA::IntrospectionCollection::Iterator i = collection->Begin(object);
		while(NULL != i)
		{
			if(NULL != valueType->GetIntrospection())
			{
				void * itemObject = collection->ItemData(i);
				const DAVA::IntrospectionInfo *itemInfo = valueType->GetIntrospection(itemObject);

				if(NULL != itemInfo && NULL != itemObject)
				{
					QtPropertyData *childData = new QtPropertyDataIntrospection(itemObject, itemInfo, hasAnyFlags, hasNotAnyFlags);
					ChildAdd(QString::number(index), childData);
				}
				else
				{
					QString s;
					QtPropertyData* childData = new QtPropertyData(s.sprintf("[%p] Pointer", itemObject));
					childData->SetFlags(FLAG_IS_DISABLED);
					ChildAdd(QString::number(index), childData);
				}
			}
			else
			{
				if(!valueType->IsPointer())
				{
					QtPropertyDataDavaVariant *childData = new QtPropertyDataDavaVariant(DAVA::VariantType::LoadData(collection->ItemPointer(i), valueType));
					ChildAdd(QString::number(index), childData);
				}
				else
				{
					QString s;
					QtPropertyData* childData = new QtPropertyData(s.sprintf("[%p] Pointer", collection->ItemData(i)));
					childData->SetFlags(FLAG_IS_DISABLED);
					ChildAdd(QString::number(index), childData);
				}
			}

			index++;
			i = collection->Next(i);
		}
	}

	SetFlags(FLAG_IS_DISABLED);
}

QtPropertyDataIntroCollection::~QtPropertyDataIntroCollection()
{ }

QVariant QtPropertyDataIntroCollection::GetValueInternal()
{
	ChildNeedUpdate();
	return QString().sprintf("Collection, size %d", collection->Size(object));
}
