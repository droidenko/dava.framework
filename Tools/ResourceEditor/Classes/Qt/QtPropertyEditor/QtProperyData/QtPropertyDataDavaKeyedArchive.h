#ifndef __QT_PROPERTY_DATA_DAVA_KEYEDARCHIVE_H__
#define __QT_PROPERTY_DATA_DAVA_KEYEDARCHIVE_H__

#include "Base/Introspection.h"
#include "Qt/QtPropertyEditor/QtPropertyData.h"

#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

class QtPropertyDataDavaKeyedArcive : public QtPropertyData
{
	Q_OBJECT;

public:
	QtPropertyDataDavaKeyedArcive(DAVA::KeyedArchive *archive);
	virtual ~QtPropertyDataDavaKeyedArcive();

protected:
	DAVA::KeyedArchive* curArchive;
	int lastAddedType;

	virtual QVariant GetValueInternal();
	virtual void SetValueInternal(const QVariant &value);
	virtual void ChildChanged(const QString &key, QtPropertyData *data);

private:
	void ChildsSync();
	void ChildCreate(const QString &key, DAVA::VariantType *value);

protected slots:
	void AddKeyedArchiveField();
	void RemKeyedArchiveField();
	void NewKeyedArchiveFieldReady(const DAVA::String &key, const DAVA::VariantType &value);
};

class KeyedArchiveItemWidget : public QWidget
{
	Q_OBJECT;

public:
	KeyedArchiveItemWidget(DAVA::KeyedArchive *arch, int defaultType = DAVA::VariantType::TYPE_STRING, QWidget *parent = NULL);
	~KeyedArchiveItemWidget();

signals:
	void ValueReady(const DAVA::String &key, const DAVA::VariantType &value);

protected:
	DAVA::KeyedArchive *arch;

	QLineEdit *keyWidget;
	QComboBox *valueWidget;
	QComboBox *presetWidget;
	QPushButton *defaultBtn;

	virtual void showEvent(QShowEvent * event);
	virtual void keyPressEvent(QKeyEvent *event);

protected slots:
	void OkKeyPressed();
};

#endif // __QT_PROPERTY_DATA_DAVA_KEYEDARCHIVE_H__
