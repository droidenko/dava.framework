#include "DAVAEngine.h"
#include "Debug/DVAssert.h"
#include "Qt/Main/QtUtils.h"
#include "Qt/QtPropertyEditor/QtProperyData/QtPropertyDataDavaKeyedArchive.h"
#include "Qt/QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"

#include <QSet>
#include <QMenu>
#include <QGridLayout>
#include <QAction>
#include <QLabel>
#include <QMessageBox>
#include <QKeyEvent>

QtPropertyDataDavaKeyedArcive::QtPropertyDataDavaKeyedArcive(DAVA::KeyedArchive *archive)
	: curArchive(archive)
	, lastAddedType(DAVA::VariantType::TYPE_STRING)
{
	if(NULL != curArchive)
	{
		curArchive->Retain();
	}

	SetFlags(FLAG_IS_DISABLED);
	ChildsSync();

	// add optional widget (button) to add new key
	QPushButton *addButton = new QPushButton(QIcon(":/QtIcons/keyplus.png"), "");
	addButton->setIconSize(QSize(12, 12));
	AddOW(QtPropertyOW(addButton));
	QObject::connect(addButton, SIGNAL(pressed()), this, SLOT(AddKeyedArchiveField()));
}

QtPropertyDataDavaKeyedArcive::~QtPropertyDataDavaKeyedArcive()
{
	if(NULL != curArchive)
	{
		curArchive->Release();
	}
}

QVariant QtPropertyDataDavaKeyedArcive::GetValueInternal()
{
	QVariant v;

	if(NULL != curArchive)
	{
		v = QString("KeyedArchive");
	}
	else
	{
		v = QString("KeyedArchive[NULL]");
	}

	return v;
}

void QtPropertyDataDavaKeyedArcive::SetValueInternal(const QVariant &value)
{ }

void QtPropertyDataDavaKeyedArcive::ChildChanged(const QString &key, QtPropertyData *data)
{
	if(NULL != curArchive)
	{
		QtPropertyDataDavaVariant *variantData = dynamic_cast<QtPropertyDataDavaVariant *>(data);
		if(NULL != variantData)
		{
			curArchive->SetVariant(key.toAscii().constData(), variantData->GetVariantValue());
		}
	}
}

void QtPropertyDataDavaKeyedArcive::ChildsSync()
{
	QSet<QtPropertyData *> dataToRemove;

	// at first step of sync we mark (placing to vector) items to remove
	for(int i = 0; i < ChildCount(); ++i)
	{
		QPair<QString, QtPropertyData *> pair = ChildGet(i);
		if(NULL != pair.second)
		{
			dataToRemove.insert(pair.second);
		}
	}

	// as second step we go throught keyed archive and add new data items,
	// and remove deleting mark from items that are still in archive
	if(NULL != curArchive)
	{
		DAVA::Map<DAVA::String, DAVA::VariantType*> data = curArchive->GetArchieveData();
		DAVA::Map<DAVA::String, DAVA::VariantType*>::iterator i = data.begin();

		for(; i != data.end(); ++i)
		{
			QtPropertyData *childData = ChildGet(i->first.c_str());

			// this key already in items list
			if(NULL != childData)
			{
				// remove deleting mark
				dataToRemove.remove(childData);
			}
			// create new child data
			else
			{
				ChildCreate(i->first.c_str(), i->second);
			}
		}
	}

	// delete all marked items
	QSetIterator<QtPropertyData *> it(dataToRemove);
	while(it.hasNext())
	{
		ChildRemove(it.next());
	}
}

void QtPropertyDataDavaKeyedArcive::ChildCreate(const QString &key, DAVA::VariantType *value)
{
	QtPropertyData *childData = NULL;

	if(value->type == DAVA::VariantType::TYPE_KEYED_ARCHIVE)
	{
		childData = new QtPropertyDataDavaKeyedArcive(value->AsKeyedArchive());
	}
	else
	{
		childData = new QtPropertyDataDavaVariant(*value);
	}

	ChildAdd(key, childData);

	// add optional widget (button) to remove this key
	QPushButton *remButton = new QPushButton(QIcon(":/QtIcons/keyminus.png"), "");
	remButton->setIconSize(QSize(12, 12));
	childData->AddOW(QtPropertyOW(remButton));
	childData->SetOWViewport(GetOWViewport());

	QObject::connect(remButton, SIGNAL(pressed()), this, SLOT(RemKeyedArchiveField()));
}

void QtPropertyDataDavaKeyedArcive::AddKeyedArchiveField()
{
	QPushButton* btn = dynamic_cast<QPushButton*>(QObject::sender());
	if(NULL != curArchive && NULL != btn)
	{
		KeyedArchiveItemWidget *w = new KeyedArchiveItemWidget(curArchive, lastAddedType);
		QObject::connect(w, SIGNAL(ValueReady(const DAVA::String&, const DAVA::VariantType&)), this, SLOT(NewKeyedArchiveFieldReady(const DAVA::String&, const DAVA::VariantType&)));

		w->show();

		QRect bRect = btn->geometry();
		QPoint bPos = btn->mapToGlobal(btn->mapFromParent(bRect.topLeft()));

		QRect wRect = w->geometry();
		QPoint wPos = QPoint(bPos.x() - wRect.width() + bRect.width(), bPos.y() + bRect.height());

		w->move(wPos);
	}
}

void QtPropertyDataDavaKeyedArcive::RemKeyedArchiveField()
{
	QPushButton* btn = dynamic_cast<QPushButton*>(QObject::sender());
	if(NULL != btn && NULL != curArchive)
	{
		// search for child data with such button
		for(int i = 0; i < ChildCount(); ++i)
		{
			QPair<QString, QtPropertyData *> child = ChildGet(i);
			QtPropertyData *childData = child.second;

			if(NULL != childData)
			{
				// search btn thought this child optional widgets
				for (int j = 0; j < childData->GetOWCount(); j++)
				{
					const QtPropertyOW *ow = childData->GetOW(j);
					if(NULL != ow && ow->widget == btn)
					{
						curArchive->DeleteKey(child.first.toAscii().constData());
						ChildsSync();
						break;
					}
				}
				
			}
		}
	}
}

void QtPropertyDataDavaKeyedArcive::NewKeyedArchiveFieldReady(const DAVA::String &key, const DAVA::VariantType &value)
{
	if(NULL != curArchive)
	{
		curArchive->SetVariant(key, value);
		lastAddedType = value.type;
		ChildsSync();
	}
}


KeyedArchiveItemWidget::KeyedArchiveItemWidget(DAVA::KeyedArchive *_arch, int defaultType, QWidget *parent /* = NULL */) 
	: QWidget(parent)
	, arch(_arch)
{
	QGridLayout *grLayout = new QGridLayout();
	int delautTypeIndex = 0;

	if(NULL != arch)
	{
		arch->Retain();
	}

	defaultBtn = new QPushButton("Ok", this);
	keyWidget = new QLineEdit(this);
	valueWidget = new QComboBox(this);
	presetWidget = new QComboBox(this);

	int j = 0;
	for (int type = (DAVA::VariantType::TYPE_NONE + 1); type < DAVA::VariantType::TYPES_COUNT; type++)
	{
		// don't allow byte array
		if(type != DAVA::VariantType::TYPE_BYTE_ARRAY)
		{
			valueWidget->addItem(DAVA::VariantType::variantNamesMap[type].variantName.c_str(), type);

			if(type == defaultType)
			{
				delautTypeIndex = j;
			}

			j++;
		}
	}
	valueWidget->setCurrentIndex(delautTypeIndex);

	grLayout->addWidget(new QLabel("Key:", this), 0, 0, 1, 1);
	grLayout->addWidget(keyWidget, 0, 1, 1, 2);
	grLayout->addWidget(new QLabel("Value type:", this), 1, 0, 1, 1);
	grLayout->addWidget(valueWidget, 1, 1, 1, 2);
	grLayout->addWidget(new QLabel("Preset:", this), 2, 0, 1, 1);
	grLayout->addWidget(presetWidget, 2, 1, 1, 2);
	grLayout->addWidget(defaultBtn, 3, 2, 1, 1);

	grLayout->setMargin(5);
	grLayout->setSpacing(3);
	setLayout(grLayout);

	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
	setWindowOpacity(0.95);

	QObject::connect(defaultBtn, SIGNAL(pressed()), this, SLOT(OkKeyPressed()));
}

KeyedArchiveItemWidget::~KeyedArchiveItemWidget()
{
	if(NULL != arch)
	{
		arch->Release();
	} 
}

void KeyedArchiveItemWidget::showEvent(QShowEvent * event)
{
	QWidget::showEvent(event);
	keyWidget->setFocus();
}

void KeyedArchiveItemWidget::keyPressEvent(QKeyEvent *e)
{
	if (!e->modifiers() || (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter)) 
	{
		switch(e->key())
		{
		case Qt::Key_Enter:
		case Qt::Key_Return:
			defaultBtn->click();
			break;
		case Qt::Key_Escape:
			this->deleteLater();
			break;
		default:
			e->ignore();
			return;
		}
	} 
	else 
	{
		e->ignore();
	}
}

void KeyedArchiveItemWidget::OkKeyPressed()
{
	if(NULL != arch)
	{
		DAVA::String key = keyWidget->text().toAscii().constData();

		if(key.empty())
		{
			// TODO:
			// other way to report error without losing focus
			// ...
			// 

			QMessageBox::warning(NULL, "Wrong key value", "Key value can't be empty");
		}
		else if(arch->IsKeyExists(key))
		{
			// TODO:
			// other way to report error without losing focus
			// ...
			// 

			QMessageBox::warning(NULL, "Wrong key value", "That key already exists");
		}
		else
		{
			emit ValueReady(key, DAVA::VariantType::FromType(valueWidget->itemData(valueWidget->currentIndex()).toInt()));
			this->deleteLater();
		}
	}
	else
	{
		this->deleteLater();
	}
}

