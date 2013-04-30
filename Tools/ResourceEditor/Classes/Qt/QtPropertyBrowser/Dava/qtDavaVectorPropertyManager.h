#ifndef QTDAVAVECTORPROPERTYMANAGER_H
#define QTDAVAVECTORPROPERTYMANAGER_H

#include "Qt/QtPropertyBrowser/qtpropertybrowser.h"
#include "Qt/QtPropertyBrowser/qtpropertymanager.h"
#include "Base/BaseMath.h"
#include "Math/Vector.h"

#include <QMap>

class QtVector4PropertyManager;

class QtVector4PropertyManagerPrivate
{
	QtVector4PropertyManager *q_ptr;
	Q_DECLARE_PUBLIC(QtVector4PropertyManager)
public:

	void slotDoubleChanged(QtProperty *property, double value);
	void slotPropertyDestroyed(QtProperty *property);

	struct Data
	{
		Data() : val(0, 0, 0, 0)
		{}

		DAVA::Vector4 val;
	};

	typedef QMap<const QtProperty *, Data> PropertyValueMap;
	PropertyValueMap m_values;
	QtDoublePropertyManager *m_doublePropertyManager;

	QMap<const QtProperty *, QtProperty *> m_propertyToX;
	QMap<const QtProperty *, QtProperty *> m_propertyToY;
	QMap<const QtProperty *, QtProperty *> m_propertyToZ;
	QMap<const QtProperty *, QtProperty *> m_propertyToW;

	QMap<const QtProperty *, QtProperty *> m_xToProperty;
	QMap<const QtProperty *, QtProperty *> m_yToProperty;
	QMap<const QtProperty *, QtProperty *> m_zToProperty;
	QMap<const QtProperty *, QtProperty *> m_wToProperty;
};

class QT_QTPROPERTYBROWSER_EXPORT QtVector4PropertyManager : public QtAbstractPropertyManager
{
	Q_OBJECT
public:
	QtVector4PropertyManager(QObject *parent = 0);
	~QtVector4PropertyManager();

	QtDoublePropertyManager *subDoublePropertyManager() const;
	DAVA::Vector4 value(const QtProperty *property) const;

public Q_SLOTS:
	void setValue(QtProperty *property, const DAVA::Vector4 &val);

Q_SIGNALS:
	void valueChanged(QtProperty *property, const DAVA::Vector4 &val);

protected:
	QString valueText(const QtProperty *property) const;
	virtual void initializeProperty(QtProperty *property);
	virtual void uninitializeProperty(QtProperty *property);

private:
	QtVector4PropertyManagerPrivate *d_ptr;
	Q_DECLARE_PRIVATE(QtVector4PropertyManager)
	Q_DISABLE_COPY(QtVector4PropertyManager)
	Q_PRIVATE_SLOT(d_func(), void slotDoubleChanged(QtProperty *, double))
	Q_PRIVATE_SLOT(d_func(), void slotPropertyDestroyed(QtProperty *))
};

class QtVector3PropertyManager;

class QtVector3PropertyManagerPrivate
{
	QtVector3PropertyManager *q_ptr;
	Q_DECLARE_PUBLIC(QtVector3PropertyManager)
public:

	void slotDoubleChanged(QtProperty *property, double value);
	void slotPropertyDestroyed(QtProperty *property);

	struct Data
	{
		Data() : val(0, 0, 0)
		{}

		DAVA::Vector3 val;
	};

	typedef QMap<const QtProperty *, Data> PropertyValueMap;
	PropertyValueMap m_values;
	QtDoublePropertyManager *m_doublePropertyManager;

	QMap<const QtProperty *, QtProperty *> m_propertyToX;
	QMap<const QtProperty *, QtProperty *> m_propertyToY;
	QMap<const QtProperty *, QtProperty *> m_propertyToZ;

	QMap<const QtProperty *, QtProperty *> m_xToProperty;
	QMap<const QtProperty *, QtProperty *> m_yToProperty;
	QMap<const QtProperty *, QtProperty *> m_zToProperty;
};

class QT_QTPROPERTYBROWSER_EXPORT QtVector3PropertyManager : public QtAbstractPropertyManager
{
	Q_OBJECT
public:
	QtVector3PropertyManager(QObject *parent = 0);
	~QtVector3PropertyManager();

	QtDoublePropertyManager *subDoublePropertyManager() const;
	DAVA::Vector3 value(const QtProperty *property) const;

	public Q_SLOTS:
		void setValue(QtProperty *property, const DAVA::Vector3 &val);

Q_SIGNALS:
		void valueChanged(QtProperty *property, const DAVA::Vector3 &val);

protected:
	QString valueText(const QtProperty *property) const;
	virtual void initializeProperty(QtProperty *property);
	virtual void uninitializeProperty(QtProperty *property);

private:
	QtVector3PropertyManagerPrivate *d_ptr;
	Q_DECLARE_PRIVATE(QtVector3PropertyManager)
	Q_DISABLE_COPY(QtVector3PropertyManager)
	Q_PRIVATE_SLOT(d_func(), void slotDoubleChanged(QtProperty *, double))
	Q_PRIVATE_SLOT(d_func(), void slotPropertyDestroyed(QtProperty *))
};

class QtVector2PropertyManager;

class QtVector2PropertyManagerPrivate
{
	QtVector2PropertyManager *q_ptr;
	Q_DECLARE_PUBLIC(QtVector2PropertyManager)
public:

	void slotDoubleChanged(QtProperty *property, double value);
	void slotPropertyDestroyed(QtProperty *property);

	struct Data
	{
		Data() : val(0, 0)
		{}

		DAVA::Vector2 val;
	};

	typedef QMap<const QtProperty *, Data> PropertyValueMap;
	PropertyValueMap m_values;
	QtDoublePropertyManager *m_doublePropertyManager;

	QMap<const QtProperty *, QtProperty *> m_propertyToX;
	QMap<const QtProperty *, QtProperty *> m_propertyToY;

	QMap<const QtProperty *, QtProperty *> m_xToProperty;
	QMap<const QtProperty *, QtProperty *> m_yToProperty;
};

class QT_QTPROPERTYBROWSER_EXPORT QtVector2PropertyManager : public QtAbstractPropertyManager
{
	Q_OBJECT
public:
	QtVector2PropertyManager(QObject *parent = 0);
	~QtVector2PropertyManager();

	QtDoublePropertyManager *subDoublePropertyManager() const;
	DAVA::Vector2 value(const QtProperty *property) const;

	public Q_SLOTS:
		void setValue(QtProperty *property, const DAVA::Vector2 &val);

Q_SIGNALS:
		void valueChanged(QtProperty *property, const DAVA::Vector2 &val);

protected:
	QString valueText(const QtProperty *property) const;
	virtual void initializeProperty(QtProperty *property);
	virtual void uninitializeProperty(QtProperty *property);

private:
	QtVector2PropertyManagerPrivate *d_ptr;
	Q_DECLARE_PRIVATE(QtVector2PropertyManager)
		Q_DISABLE_COPY(QtVector2PropertyManager)
		Q_PRIVATE_SLOT(d_func(), void slotDoubleChanged(QtProperty *, double))
		Q_PRIVATE_SLOT(d_func(), void slotPropertyDestroyed(QtProperty *))
};

#endif 