#include "DAVAEngine.h"
#include "Qt/Scene/SceneDataManager.h"
#include "Entity/Component.h"
#include "Qt/Main/mainwindow.h"

#include "Qt/DockProperties/PropertyEditor.h"
#include "Qt/QtPropertyEditor/QtPropertyItem.h"
#include "Qt/QtPropertyEditor/QtProperyData/QtPropertyDataIntrospection.h"
#include "Qt/QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"

#include "PropertyEditorStateHelper.h"

PropertyEditor::PropertyEditor(QWidget *parent /* = 0 */)
	: QtPropertyEditor(parent)
	, advancedMode(false)
	, curNode(NULL)
	, treeStateHelper(this, this->curModel)
{
	// global scene manager signals
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneActivated(SceneData *)), this, SLOT(sceneActivated(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneChanged(SceneData *)), this, SLOT(sceneChanged(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneReleased(SceneData *)), this, SLOT(sceneReleased(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneNodeSelected(SceneData *, DAVA::Entity *)), this, SLOT(sceneNodeSelected(SceneData *, DAVA::Entity *)));

	// MainWindow actions
	QObject::connect(QtMainWindow::Instance()->GetUI()->actionShowAdvancedProp, SIGNAL(triggered()), this, SLOT(actionShowAdvanced()));
	advancedMode = QtMainWindow::Instance()->GetUI()->actionShowAdvancedProp->isChecked();


	posSaver.Attach(this, "DocPropetyEditor");
	
	DAVA::VariantType v = posSaver.LoadValue("splitPos");
	if(v.GetType() == DAVA::VariantType::TYPE_INT32) header()->resizeSection(0, v.AsInt32());
}

PropertyEditor::~PropertyEditor()
{
	DAVA::VariantType v(header()->sectionSize(0));
	posSaver.SaveValue("splitPos", v);

	SafeRelease(curNode);
}

void PropertyEditor::SetNode(DAVA::Entity *node)
{
	// Store the current Property Editor Tree state before switching to the new node.
	// Do not clear the current states map - we are using one storage to share opened
	// Property Editor nodes between the different Scene Nodes.
	treeStateHelper.SaveTreeViewState(false);
	
	SafeRelease(curNode);
	curNode = SafeRetain(node);

	RemovePropertyAll();
	if(NULL != curNode)
	{
        AppendIntrospectionInfo(curNode, curNode->GetTypeInfo());

		for(int32 i = 0; i < Component::COMPONENT_COUNT; ++i)
        {
            Component *component = curNode->GetComponent(i);
            if(component)
            {
                QtPropertyData *componentData = AppendIntrospectionInfo(component, component->GetTypeInfo());

				if(NULL != componentData)
				{
					// Add optional button to track "remove this component" command
					QPushButton *removeButton = new QPushButton(QIcon(":/QtIcons/removecomponent.png"), "");
					removeButton->setFlat(true);

					componentData->AddOW(QtPropertyOW(removeButton, true));
				}
            }
        }
	}

	// Restore back the tree view state from the shared storage.
	if (!treeStateHelper.IsTreeStateStorageEmpty())
	{
		treeStateHelper.RestoreTreeViewState();
	}
	else
	{
		// Expand the root elements as default value.
		expandToDepth(0);
	}
}

void PropertyEditor::SetAdvancedMode(bool set)
{
	if(advancedMode != set)
	{
		advancedMode = set;
		SetNode(curNode);
	}
}

QtPropertyData* PropertyEditor::AppendIntrospectionInfo(void *object, const DAVA::IntrospectionInfo *info)
{
	QtPropertyData* propData = NULL;

	if(NULL != info)
	{
		bool hasMembers = false;
		const IntrospectionInfo *currentInfo = info;

		// check if there are any memebers
		while (NULL != currentInfo)
		{
			if(currentInfo->MembersCount() > 0)
			{
				hasMembers = true;
				break;
			}
			currentInfo = currentInfo->BaseInfo();
		}

        if(hasMembers)
        {
			int hasFlags = DAVA::INTROSPECTION_EDITOR;
			int hasNotFlags = 0;

			if(!advancedMode) hasNotFlags |= DAVA::INTROSPECTION_EDITOR_READONLY;

			propData = new QtPropertyDataIntrospection(object, currentInfo, hasFlags, hasNotFlags);

			if(propData->ChildCount() > 0)
			{
				QPair<QtPropertyItem*, QtPropertyItem*> prop = AppendProperty(currentInfo->Name(), propData);
            
	            prop.first->setBackground(QBrush(QColor(Qt::lightGray)));
		        prop.second->setBackground(QBrush(QColor(Qt::lightGray)));
			}
			else
			{
				delete propData;
				propData = NULL;
			}
        }
    }

	return propData;
}

void PropertyEditor::sceneChanged(SceneData *sceneData)
{
	if(NULL != sceneData)
	{
		SetNode(sceneData->GetSelectedNode());
	}
}

void PropertyEditor::sceneActivated(SceneData *sceneData)
{
	if(NULL != sceneData)
	{
		SetNode(sceneData->GetSelectedNode());
	}
}

void PropertyEditor::sceneReleased(SceneData *sceneData)
{ }

void PropertyEditor::sceneNodeSelected(SceneData *sceneData, DAVA::Entity *node)
{
	SetNode(node);
}

void PropertyEditor::actionShowAdvanced()
{
	QAction *showAdvancedAction = dynamic_cast<QAction *>(QObject::sender());
	if(NULL != showAdvancedAction)
	{
		SetAdvancedMode(showAdvancedAction->isChecked());
	}
}
