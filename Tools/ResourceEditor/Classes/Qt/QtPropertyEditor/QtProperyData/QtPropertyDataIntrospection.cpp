#include "DAVAEngine.h"
#include "Qt/QtPropertyEditor/QtProperyData/QtPropertyDataIntrospection.h"
#include "Qt/QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"
#include "Qt/QtPropertyEditor/QtProperyData/QtPropertyDataDavaKeyedArchive.h"
#include "Qt/QtPropertyEditor/QtProperyData/QtPropertyDataIntoCollection.h"

QtPropertyDataIntrospection::QtPropertyDataIntrospection(void *_object, const DAVA::IntrospectionInfo *_info, int hasAnyFlags, int hasNotAnyFlags)
	: object(_object)
	, info(_info)
{
	CreateCustomButtonsForRenderObject();

	while(NULL != _info && NULL != object)
	{
		for(DAVA::int32 i = 0; i < info->MembersCount(); ++i)
		{
			const DAVA::IntrospectionMember *member = _info->Member(i);
			if(member && 
				(member->Flags() & hasAnyFlags) &&
				!(member->Flags() & hasNotAnyFlags))
			{
                AddMember(member, hasAnyFlags, hasNotAnyFlags);
			}
		}

		_info = _info->BaseInfo();
	}

	SetFlags(FLAG_IS_DISABLED);
}

QtPropertyDataIntrospection::~QtPropertyDataIntrospection()
{ }

void QtPropertyDataIntrospection::AddMember(const DAVA::IntrospectionMember *member, int hasAnyFlags, int hasNotAnyFlags)
{
	void *memberObject = member->Data(object);
	const DAVA::MetaInfo *memberMetaInfo = member->Type();
	const DAVA::IntrospectionInfo *memberIntrospection = memberMetaInfo->GetIntrospection(memberObject);
	bool isKeyedArchive = false;

	// keyed archive
	if(NULL != memberIntrospection && (memberIntrospection->Type() == DAVA::MetaInfo::Instance<DAVA::KeyedArchive>()))
	{
		QtPropertyDataDavaKeyedArcive *childData = new QtPropertyDataDavaKeyedArcive((DAVA::KeyedArchive *) memberObject);
		ChildAdd(member->Name(), childData);
	}
	// introspection
	else if(NULL != memberObject && NULL != memberIntrospection)
    {
		QtPropertyDataIntrospection *childData = new QtPropertyDataIntrospection(memberObject, memberIntrospection, hasAnyFlags, hasNotAnyFlags);
		ChildAdd(member->Name(), childData);
    }
	// any other value
    else
    {
		// pointer
        if(memberMetaInfo->IsPointer())
        {
			QString s;
            QtPropertyData* childData = new QtPropertyData(s.sprintf("[%p] Pointer", memberObject));
            childData->SetFlags(childData->GetFlags() | FLAG_IS_DISABLED);
            ChildAdd(member->Name(), childData);
        }
		// other value
        else
        {
			// collection
            if(member->Collection() && !isKeyedArchive)
            {
                QtPropertyDataIntroCollection *childCollection = new QtPropertyDataIntroCollection(memberObject, member->Collection(), hasAnyFlags, hasNotAnyFlags);
                ChildAdd(member->Name(), childCollection);
            }
			// variant
            else
            {
                QtPropertyDataDavaVariant *childData = new QtPropertyDataDavaVariant(member->Value(object));
                if(member->Flags() & DAVA::INTROSPECTION_EDITOR_READONLY)
                {
                    childData->SetFlags(childData->GetFlags() | FLAG_IS_NOT_EDITABLE);
                }
                
                ChildAdd(member->Name(), childData);
                childVariantMembers.insert(childData, member);
            }
        }
    }
}

QVariant QtPropertyDataIntrospection::GetValueInternal()
{
	ChildNeedUpdate();
	return QVariant(info->Name());
}

void QtPropertyDataIntrospection::ChildChanged(const QString &key, QtPropertyData *data)
{
	QtPropertyDataDavaVariant *dataVariant = (QtPropertyDataDavaVariant *) data;

	if(childVariantMembers.contains(dataVariant))
	{
		const DAVA::IntrospectionMember *member = childVariantMembers[dataVariant];
		member->SetValue(object, dataVariant->GetVariantValue());
	}
}

void QtPropertyDataIntrospection::ChildNeedUpdate()
{
	QMapIterator<QtPropertyDataDavaVariant*, const DAVA::IntrospectionMember *> i = QMapIterator<QtPropertyDataDavaVariant*, const DAVA::IntrospectionMember *>(childVariantMembers);

	while(i.hasNext())
	{
		i.next();

		QtPropertyDataDavaVariant *childData = i.key();
		DAVA::VariantType childCurValue = i.value()->Value(object);

		if(childCurValue != childData->GetVariantValue())
		{
			childData->SetVariantValue(childCurValue);
		}

	}
}

void QtPropertyDataIntrospection::CreateCustomButtonsForRenderObject()
{
	if(NULL != info && (info->Type() == DAVA::MetaInfo::Instance<DAVA::RenderObject>()))
	{
		QPushButton *bakeButton = new QPushButton(QIcon(":/QtIcons/transform_bake.png"), "");
		bakeButton->setToolTip("Bake Transform");
		bakeButton->setIconSize(QSize(12, 12));
		AddOW(QtPropertyOW(bakeButton));
		QObject::connect(bakeButton, SIGNAL(pressed()), this, SLOT(BakeTransform()));
	}
}

void QtPropertyDataIntrospection::BakeTransform()
{
	QtPropertyDataIntrospection * renderComponentProperty = static_cast<QtPropertyDataIntrospection*>(parent);
	DAVA::Entity * entity = (static_cast<DAVA::RenderComponent*>(renderComponentProperty->object))->GetEntity();
	DAVA::RenderObject * ro = static_cast<DAVA::RenderObject*>(object);
	ro->BakeTransform(entity->GetLocalTransform());
	entity->SetLocalTransform(DAVA::Matrix4::IDENTITY);
}

