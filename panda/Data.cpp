#include <panda/Data.h>

#include <panda/Animation.h>
#include <QRectF>
#include <QColor>
#include <QPointF>
#include <QVector>
#include <QImage>

#include <panda/Topology.h>

#include <panda/PandaObject.h>
#include <panda/DataTraits.h>

namespace panda
{

QString dataTypeToName(int type)
{
	switch(type)
	{
	case QMetaType::Int: 		return "integer";
	case QMetaType::Double:		return "real";
	case QMetaType::QColor:		return "color";
	case QMetaType::QPointF:	return "point";
	case QMetaType::QRectF:		return "rectangle";
	case QMetaType::QString:	return "text";
	case QMetaType::QImage:		return "image";
	default:
		if(type == qMetaTypeId<Topology>())
			return "topology";
		return "unknown";
	}
}

int dataNameToType(QString name)
{
	if(name == "integer")
		return QMetaType::Int;
	else if(name == "real")
		return QMetaType::Double;
	else if(name == "color")
		return QMetaType::QColor;
	else if(name == "point")
		return QMetaType::QPointF;
	else if(name == "rectangle")
		return QMetaType::QRectF;
	else if(name == "text")
		return QMetaType::QString;
	else if(name == "image")
		return QMetaType::QImage;
	else if(name == "topology")
		return qMetaTypeId<Topology>();
	return -1;
}

BaseData* createDataFromType(int type, const QString& name, const QString& help, PandaObject* owner)
{
    switch(type)
    {
    case QMetaType::Int:
        return new Data<int>(name, help, owner);
    case QMetaType::Double:
        return new Data<double>(name, help, owner);
    case QMetaType::QColor:
        return new Data<QColor>(name, help, owner);
    case QMetaType::QPointF:
        return new Data<QPointF>(name, help, owner);
    case QMetaType::QRectF:
        return new Data<QRectF>(name, help, owner);
    case QMetaType::QString:
        return new Data<QString>(name, help, owner);
    case QMetaType::QImage:
        return new Data<QImage>(name, help, owner);
    default:
		if(type == qMetaTypeId<Topology>())
			return new Data<Topology>(name, help, owner);
        return nullptr;
    }
}

BaseData* createVectorDataFromType(int type, const QString& name, const QString& help, PandaObject* owner)
{
    switch(type)
    {
    case QMetaType::Int:
        return new Data< QVector<int> >(name, help, owner);
    case QMetaType::Double:
        return new Data< QVector<double> >(name, help, owner);
    case QMetaType::QColor:
        return new Data< QVector<QColor> >(name, help, owner);
    case QMetaType::QPointF:
        return new Data< QVector<QPointF> >(name, help, owner);
    case QMetaType::QRectF:
        return new Data< QVector<QRectF> >(name, help, owner);
    case QMetaType::QString:
        return new Data< QVector<QString> >(name, help, owner);
    case QMetaType::QImage:
        return new Data< QVector<QImage> >(name, help, owner);
    default:
        return nullptr;
    }
}

BaseData* createAnimationDataFromType(int type, const QString& name, const QString& help, PandaObject* owner)
{
    switch(type)
    {
    case QMetaType::Double:
        return new Data< Animation<double> >(name, help, owner);
    case QMetaType::QColor:
        return new Data< Animation<QColor> >(name, help, owner);
    case QMetaType::QPointF:
        return new Data< Animation<QPointF> >(name, help, owner);
    default:
        return nullptr;
    }
}

BaseData* createDataFromFullType(int fullType, const QString& name, const QString& help, PandaObject* owner)
{
	int type = BaseData::getValueType(fullType);
	if(BaseData::isVector(fullType))
		return createVectorDataFromType(type, name, help, owner);
	if(BaseData::isAnimation(fullType))
		return createAnimationDataFromType(type, name, help, owner);

	return createDataFromType(type, name, help, owner);
}

//***************************************************************//

template<class T>
bool Data<T>::isSingleValue() const
{
	return data_trait<T>::is_single == 1;
}

template<class T>
bool Data<T>::isVector() const
{
	return data_trait<T>::is_vector == 1;
}

template<class T>
bool Data<T>::isAnimation() const
{
	return data_trait<T>::is_animation == 1;
}

template<class T>
int Data<T>::getValueType() const
{
	return data_trait<T>::valueType();
}

template<class T>
int Data<T>::getSize() const
{
	return data_trait<T>::size(*this);
}

template<class T>
void Data<T>::clear(int size, bool init)
{
	data_trait<T>::clear(*this, size, init);
}

template<class T>
QVariant Data<T>::getBaseValue(int index) const
{
	return data_trait<T>::getBaseValue(*this, index);
}

template<class T>
void Data<T>::fromBaseValue(QVariant val, int index)
{
	data_trait<T>::fromBaseValue(*this, val, index);
}

template<class T>
QString Data<T>::doToString() const
{
	return valueToString(value);
}

template<class T>
void Data<T>::fromString(const QString& text)
{
	beginEdit();
	value = valueFromString<T>(text);
	endEdit();
}

template<class T>
void Data<T>::copyValueFrom(const BaseData* parent)
{
	data_trait<T>::copyValue(this, parent);
	this->isValueSet = true;
}

template<class T>
void Data<T>::save(QDomDocument& doc, QDomElement& elem)
{
	data_trait<T>::writeValue(doc, elem, value);
}

template<class T>
void Data<T>::load(QDomElement& elem)
{
	data_trait<T>::readValue(elem, value);
}

template<class T>
helper::DataAccessor< Data<T> > Data<T>::getAccessor()
{
	return helper::DataAccessor< Data<T> >(*this);
}

//***************************************************************//

bool GenericData::validParent(const BaseData* parent) const
{
    if(allowedTypes.size() && !allowedTypes.contains(parent->getValueType()))
        return false;
    return true;
}

QString GenericData::getFullTypeName() const
{
    return QString("Accepting all types");
}

bool GenericNonVectorData::validParent(const BaseData* parent) const
{
    return parent->isSingleValue() && GenericData::validParent(parent);
}

QString GenericNonVectorData::getFullTypeName() const
{
    return QString("Accepting single values");
}

bool GenericVectorData::validParent(const BaseData* parent) const
{
	// TEST :  now accepting single values also, as the conversion is automatic
	return (parent->isVector() || parent->isSingleValue()) && GenericData::validParent(parent);
}

QString GenericVectorData::getFullTypeName() const
{
    return QString("Accepting lists");
}

bool GenericAnimationData::validParent(const BaseData* parent) const
{
    return parent->isAnimation() && GenericData::validParent(parent);
}

QString GenericAnimationData::getFullTypeName() const
{
    return QString("Accepting animations");
}

//***************************************************************//

template<class T>
QString Data<T>::getFullTypeName() const
{
	QString typeName = dataTypeToName(getValueType());

    if(isSingleValue())
        return QString("Single %1 value").arg(typeName);
    if(isVector())
        return QString("List of %1s").arg(typeName);
    if(isAnimation())
        return QString("Animation of %1s").arg(typeName);

    return QString();
}

//***************************************************************//

// TODO: we should not need this function after all...
template<class T>
void Data<T>::setCustomFlags()
{
	if(getValueType() == QMetaType::QImage)
	{
		setPersistent(false);
		setDisplayed(false);
	}
//	else if(getValueType() == qMetaTypeId<Topology>())
//		setDisplayed(false);
}

//***************************************************************//

template class Data<int>;
template class Data<double>;
template class Data<QColor>;
template class Data<QPointF>;
template class Data<QRectF>;
template class Data<QString>;
template class Data<QImage>;
template class Data<Topology>;

template class Data< QVector<int> >;
template class Data< QVector<double> >;
template class Data< QVector<QColor> >;
template class Data< QVector<QPointF> >;
template class Data< QVector<QRectF> >;
template class Data< QVector<QString> >;
template class Data< QVector<QImage> >;
template class Data< QVector<Topology> >;

template class Data< Animation<double> >;
template class Data< Animation<QColor> >;
template class Data< Animation<QPointF> >;

} // namespace panda
