#include "Qt/QtPropertyBrowser/Dava/qtDavaVectorPropertyManager.h"

// ------------------------------------------------------------------------------------------------------------------------------------------
// Vector4
// ------------------------------------------------------------------------------------------------------------------------------------------

void QtVector4PropertyManagerPrivate::slotDoubleChanged(QtProperty *property, double value)
{
    if (QtProperty *prop = m_xToProperty.value(property, 0)) {
        DAVA::Vector4 r = m_values[prop].val;
		r.x = value;
        q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_yToProperty.value(property, 0)) {
        DAVA::Vector4 r = m_values[prop].val;
		r.y = value;
        q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_zToProperty.value(property, 0)) {
		DAVA::Vector4 r = m_values[prop].val;
		r.z = value;
		q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_wToProperty.value(property, 0)) {
		DAVA::Vector4 r = m_values[prop].val;
		r.w = value;
		q_ptr->setValue(prop, r);
    }
}

void QtVector4PropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *pointProp = m_xToProperty.value(property, 0)) {
        m_propertyToX[pointProp] = 0;
        m_xToProperty.remove(property);
    } else if (QtProperty *pointProp = m_yToProperty.value(property, 0)) {
        m_propertyToY[pointProp] = 0;
        m_yToProperty.remove(property);
    } else if (QtProperty *pointProp = m_zToProperty.value(property, 0)) {
        m_propertyToZ[pointProp] = 0;
        m_zToProperty.remove(property);
    } else if (QtProperty *pointProp = m_wToProperty.value(property, 0)) {
        m_propertyToW[pointProp] = 0;
        m_wToProperty.remove(property);
    }
}


/*!
    Creates a manager with the given \a parent.
*/
QtVector4PropertyManager::QtVector4PropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtVector4PropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_doublePropertyManager = new QtDoublePropertyManager(this);
    connect(d_ptr->m_doublePropertyManager, SIGNAL(valueChanged(QtProperty *, double)), this, SLOT(slotDoubleChanged(QtProperty *, double)));
    connect(d_ptr->m_doublePropertyManager, SIGNAL(propertyDestroyed(QtProperty *)), this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtVector4PropertyManager::~QtVector4PropertyManager()
{
    clear();
    delete d_ptr;
}

QtDoublePropertyManager *QtVector4PropertyManager::subDoublePropertyManager() const
{
    return d_ptr->m_doublePropertyManager;
}

DAVA::Vector4 QtVector4PropertyManager::value(const QtProperty *property) const
{
	DAVA::Vector4 v;
	QtVector4PropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.find(property);

	if (it != d_ptr->m_values.constEnd())
	{
		v = it->val;
	}

	return v;
}

/*!
    \reimp
*/
QString QtVector4PropertyManager::valueText(const QtProperty *property) const
{
	char tmp[255];
	const QtVector4PropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);

	if (it == d_ptr->m_values.constEnd())
		return QString();

	const DAVA::Vector4 v = it.value().val;
	sprintf(tmp, "[%.3f, %.3f, %.3f, %.3f]", v.x, v.y, v.z, v.w);

	return QString(tmp);
}

void QtVector4PropertyManager::setValue(QtProperty *property, const DAVA::Vector4 &val)
{
    const QtVector4PropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtVector4PropertyManagerPrivate::Data data = it.value();

    if (data.val == val)
        return;

    data.val = val;

    it.value() = data;
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToX[property], val.x);
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToY[property], val.y);
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToZ[property], val.z);
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToW[property], val.w);

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtVector4PropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtVector4PropertyManagerPrivate::Data();

    QtProperty *xProp = d_ptr->m_doublePropertyManager->addProperty();
    xProp->setPropertyName(tr("X"));
    d_ptr->m_doublePropertyManager->setValue(xProp, 0);
    d_ptr->m_propertyToX[property] = xProp;
    d_ptr->m_xToProperty[xProp] = property;
    property->addSubProperty(xProp);

    QtProperty *yProp = d_ptr->m_doublePropertyManager->addProperty();
    yProp->setPropertyName(tr("Y"));
    d_ptr->m_doublePropertyManager->setValue(yProp, 0);
    d_ptr->m_propertyToY[property] = yProp;
    d_ptr->m_yToProperty[yProp] = property;
    property->addSubProperty(yProp);

	QtProperty *zProp = d_ptr->m_doublePropertyManager->addProperty();
	zProp->setPropertyName(tr("Z"));
	d_ptr->m_doublePropertyManager->setValue(zProp, 0);
	d_ptr->m_doublePropertyManager->setMinimum(zProp, 0);
	d_ptr->m_propertyToZ[property] = zProp;
	d_ptr->m_zToProperty[zProp] = property;
	property->addSubProperty(zProp);

    QtProperty *wProp = d_ptr->m_doublePropertyManager->addProperty();
    wProp->setPropertyName(tr("W"));
    d_ptr->m_doublePropertyManager->setValue(wProp, 0);
    d_ptr->m_doublePropertyManager->setMinimum(wProp, 0);
    d_ptr->m_propertyToW[property] = wProp;
    d_ptr->m_wToProperty[wProp] = property;
    property->addSubProperty(wProp);
}

/*!
    \reimp
*/
void QtVector4PropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *xProp = d_ptr->m_propertyToX[property];
    if (xProp) {
        d_ptr->m_xToProperty.remove(xProp);
        delete xProp;
    }
    d_ptr->m_propertyToX.remove(property);

    QtProperty *yProp = d_ptr->m_propertyToY[property];
    if (yProp) {
        d_ptr->m_yToProperty.remove(yProp);
        delete yProp;
    }
    d_ptr->m_propertyToY.remove(property);

    QtProperty *zProp = d_ptr->m_propertyToZ[property];
    if (zProp) {
        d_ptr->m_zToProperty.remove(zProp);
        delete zProp;
    }
    d_ptr->m_propertyToZ.remove(property);
	
	QtProperty *wProp = d_ptr->m_propertyToW[property];
	if (wProp) {
		d_ptr->m_wToProperty.remove(wProp);
		delete wProp;
	}
	d_ptr->m_propertyToW.remove(property);

    d_ptr->m_values.remove(property);
}




// ------------------------------------------------------------------------------------------------------------------------------------------
// Vector3
// ------------------------------------------------------------------------------------------------------------------------------------------


void QtVector3PropertyManagerPrivate::slotDoubleChanged(QtProperty *property, double value)
{
    if (QtProperty *prop = m_xToProperty.value(property, 0)) {
        DAVA::Vector3 r = m_values[prop].val;
		r.x = value;
        q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_yToProperty.value(property, 0)) {
        DAVA::Vector3 r = m_values[prop].val;
		r.y = value;
        q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_zToProperty.value(property, 0)) {
		DAVA::Vector3 r = m_values[prop].val;
		r.z = value;
		q_ptr->setValue(prop, r);
	}
}

void QtVector3PropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *pointProp = m_xToProperty.value(property, 0)) {
        m_propertyToX[pointProp] = 0;
        m_xToProperty.remove(property);
    } else if (QtProperty *pointProp = m_yToProperty.value(property, 0)) {
        m_propertyToY[pointProp] = 0;
        m_yToProperty.remove(property);
    } else if (QtProperty *pointProp = m_zToProperty.value(property, 0)) {
        m_propertyToZ[pointProp] = 0;
        m_zToProperty.remove(property);
	}
}


/*!
    Creates a manager with the given \a parent.
*/
QtVector3PropertyManager::QtVector3PropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtVector3PropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_doublePropertyManager = new QtDoublePropertyManager(this);
    connect(d_ptr->m_doublePropertyManager, SIGNAL(valueChanged(QtProperty *, double)), this, SLOT(slotDoubleChanged(QtProperty *, double)));
    connect(d_ptr->m_doublePropertyManager, SIGNAL(propertyDestroyed(QtProperty *)), this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtVector3PropertyManager::~QtVector3PropertyManager()
{
    clear();
    delete d_ptr;
}

QtDoublePropertyManager *QtVector3PropertyManager::subDoublePropertyManager() const
{
    return d_ptr->m_doublePropertyManager;
}

DAVA::Vector3 QtVector3PropertyManager::value(const QtProperty *property) const
{
	DAVA::Vector3 v;
	QtVector3PropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.find(property);

	if (it != d_ptr->m_values.constEnd())
	{
		v = it->val;
	}

	return v;
}

/*!
    \reimp
*/
QString QtVector3PropertyManager::valueText(const QtProperty *property) const
{
	char tmp[255];
	const QtVector3PropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);

	if (it == d_ptr->m_values.constEnd())
		return QString();

	const DAVA::Vector3 v = it.value().val;
	sprintf(tmp, "[%.3f, %.3f, %.3f]", v.x, v.y, v.z);

	return QString(tmp);
}

void QtVector3PropertyManager::setValue(QtProperty *property, const DAVA::Vector3 &val)
{
    const QtVector3PropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtVector3PropertyManagerPrivate::Data data = it.value();

    if (data.val == val)
        return;

    data.val = val;

    it.value() = data;
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToX[property], val.x);
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToY[property], val.y);
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToZ[property], val.z);

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtVector3PropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtVector3PropertyManagerPrivate::Data();

    QtProperty *xProp = d_ptr->m_doublePropertyManager->addProperty();
    xProp->setPropertyName(tr("X"));
    d_ptr->m_doublePropertyManager->setValue(xProp, 0);
    d_ptr->m_propertyToX[property] = xProp;
    d_ptr->m_xToProperty[xProp] = property;
    property->addSubProperty(xProp);

    QtProperty *yProp = d_ptr->m_doublePropertyManager->addProperty();
    yProp->setPropertyName(tr("Y"));
    d_ptr->m_doublePropertyManager->setValue(yProp, 0);
    d_ptr->m_propertyToY[property] = yProp;
    d_ptr->m_yToProperty[yProp] = property;
    property->addSubProperty(yProp);

	QtProperty *zProp = d_ptr->m_doublePropertyManager->addProperty();
	zProp->setPropertyName(tr("Z"));
	d_ptr->m_doublePropertyManager->setValue(zProp, 0);
	d_ptr->m_doublePropertyManager->setMinimum(zProp, 0);
	d_ptr->m_propertyToZ[property] = zProp;
	d_ptr->m_zToProperty[zProp] = property;
	property->addSubProperty(zProp);
}

/*!
    \reimp
*/
void QtVector3PropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *xProp = d_ptr->m_propertyToX[property];
    if (xProp) {
        d_ptr->m_xToProperty.remove(xProp);
        delete xProp;
    }
    d_ptr->m_propertyToX.remove(property);

    QtProperty *yProp = d_ptr->m_propertyToY[property];
    if (yProp) {
        d_ptr->m_yToProperty.remove(yProp);
        delete yProp;
    }
    d_ptr->m_propertyToY.remove(property);

    QtProperty *zProp = d_ptr->m_propertyToZ[property];
    if (zProp) {
        d_ptr->m_zToProperty.remove(zProp);
        delete zProp;
    }
    d_ptr->m_propertyToZ.remove(property);

    d_ptr->m_values.remove(property);
}



// ------------------------------------------------------------------------------------------------------------------------------------------
// Vector2
// ------------------------------------------------------------------------------------------------------------------------------------------


void QtVector2PropertyManagerPrivate::slotDoubleChanged(QtProperty *property, double value)
{
    if (QtProperty *prop = m_xToProperty.value(property, 0)) {
        DAVA::Vector2 r = m_values[prop].val;
		r.x = value;
        q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_yToProperty.value(property, 0)) {
        DAVA::Vector2 r = m_values[prop].val;
		r.y = value;
        q_ptr->setValue(prop, r);
    }
}

void QtVector2PropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *pointProp = m_xToProperty.value(property, 0)) {
        m_propertyToX[pointProp] = 0;
        m_xToProperty.remove(property);
    } else if (QtProperty *pointProp = m_yToProperty.value(property, 0)) {
        m_propertyToY[pointProp] = 0;
        m_yToProperty.remove(property);
	}
}


/*!
    Creates a manager with the given \a parent.
*/
QtVector2PropertyManager::QtVector2PropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtVector2PropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_doublePropertyManager = new QtDoublePropertyManager(this);
    connect(d_ptr->m_doublePropertyManager, SIGNAL(valueChanged(QtProperty *, double)), this, SLOT(slotDoubleChanged(QtProperty *, double)));
    connect(d_ptr->m_doublePropertyManager, SIGNAL(propertyDestroyed(QtProperty *)), this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtVector2PropertyManager::~QtVector2PropertyManager()
{
    clear();
    delete d_ptr;
}

QtDoublePropertyManager *QtVector2PropertyManager::subDoublePropertyManager() const
{
    return d_ptr->m_doublePropertyManager;
}

DAVA::Vector2 QtVector2PropertyManager::value(const QtProperty *property) const
{
	DAVA::Vector2 v;
	QtVector2PropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.find(property);

	if (it != d_ptr->m_values.constEnd())
	{
		v = it->val;
	}

	return v;
}

/*!
    \reimp
*/
QString QtVector2PropertyManager::valueText(const QtProperty *property) const
{
	char tmp[255];
	const QtVector2PropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);

	if (it == d_ptr->m_values.constEnd())
		return QString();

	const DAVA::Vector2 v = it.value().val;
	sprintf(tmp, "[%.3f, %.3f]", v.x, v.y);

	return QString(tmp);
}

void QtVector2PropertyManager::setValue(QtProperty *property, const DAVA::Vector2 &val)
{
    const QtVector2PropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtVector2PropertyManagerPrivate::Data data = it.value();

    if (data.val == val)
        return;

    data.val = val;

    it.value() = data;
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToX[property], val.x);
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToY[property], val.y);

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtVector2PropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtVector2PropertyManagerPrivate::Data();

    QtProperty *xProp = d_ptr->m_doublePropertyManager->addProperty();
    xProp->setPropertyName(tr("X"));
    d_ptr->m_doublePropertyManager->setValue(xProp, 0);
    d_ptr->m_propertyToX[property] = xProp;
    d_ptr->m_xToProperty[xProp] = property;
    property->addSubProperty(xProp);

    QtProperty *yProp = d_ptr->m_doublePropertyManager->addProperty();
    yProp->setPropertyName(tr("Y"));
    d_ptr->m_doublePropertyManager->setValue(yProp, 0);
    d_ptr->m_propertyToY[property] = yProp;
    d_ptr->m_yToProperty[yProp] = property;
    property->addSubProperty(yProp);
}

/*!
    \reimp
*/
void QtVector2PropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *xProp = d_ptr->m_propertyToX[property];
    if (xProp) {
        d_ptr->m_xToProperty.remove(xProp);
        delete xProp;
    }
    d_ptr->m_propertyToX.remove(property);

    QtProperty *yProp = d_ptr->m_propertyToY[property];
    if (yProp) {
        d_ptr->m_yToProperty.remove(yProp);
        delete yProp;
    }
    d_ptr->m_propertyToY.remove(property);

    d_ptr->m_values.remove(property);
}
