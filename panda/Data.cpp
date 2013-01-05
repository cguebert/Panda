#include <panda/Data.h>

#include <panda/Animation.h>
#include <QRectF>
#include <QColor>
#include <QPointF>
#include <QVector>
#include <QImage>

#include <panda/Topology.h>

#include <panda/PandaObject.h>

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
        return NULL;
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
        return NULL;
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
        return NULL;
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

template<>
QTextStream& writeValue<QColor>(QTextStream& stream, const QColor& v)
{ return stream << QString("#%1").arg(v.rgba(), 8, 16, QChar('0')).toUpper(); }

template<>
QTextStream& writeValue<QPointF>(QTextStream& stream, const QPointF& v)
{ return stream << v.x() << " " << v.y(); }

template<>
QTextStream& writeValue<QRectF>(QTextStream& stream, const QRectF& v)
{ return stream << v.left() << " " << v.top() << " " << v.right() << " " << v.bottom(); }

template<>
QTextStream& writeValue(QTextStream& stream, const QVector<QString>& v)
{
    int size = v.size();
    if(size)
    {
        writeValue(stream, v[0]);
        for(int i=1; i<size; ++i)
        {
            stream << "\n";
            writeValue(stream, v[i]);
        }
    }
    return stream;
}

template<class T>
QTextStream& writeValue(QTextStream& stream, const QVector<T>& v)
{
    int size = v.size();
    if(size)
    {
        writeValue(stream, v[0]);
        for(int i=1; i<size; ++i)
        {
            stream << " ";
            writeValue(stream, v[i]);
        }
    }
    return stream;
}

template<class T>
QTextStream& writeValue(QTextStream& stream, const Animation<T>& v)
{
    int size = v.size();
    if(size)
    {
        typename Animation<T>::Iterator iter = v.getIterator();
        while(iter.hasNext())
        {
            iter.next();
            stream << iter.key() << " ";
            writeValue(stream, iter.value());
            stream << " ";
        }
    }
    return stream;
}

template<>
QTextStream& writeValue(QTextStream& stream, const QImage&)
{ return stream; }	// Use a SaveImage object instead

template<class T>
QTextStream& writeValue(QTextStream& stream, const T& v)
{ return stream << v; }

template<class T>
QString Data<T>::doToString() const
{
    QString tempString;
    QTextStream stream(&tempString, QIODevice::WriteOnly);
    writeValue(stream, value);
    return tempString;
}

//***************************************************************//

template<>
QTextStream& readValue<QColor>(QTextStream& stream, QColor& v)
{
    QString temp;
    stream >> temp;
    if(temp.startsWith('#'))
        temp = temp.mid(1);
    v.setRgba(temp.toUInt(NULL, 16));
    return stream;
}

template<>
QTextStream& readValue<QRectF>(QTextStream& stream, QRectF& v)
{
    double l, t, r, b;
    stream >> l >> t >> r >> b;
    v = QRectF(l, t, r-l, b-t).normalized();
    return stream;
}

template<>
QTextStream& readValue<QPointF>(QTextStream& stream, QPointF& v)
{ return stream >> v.rx() >> v.ry(); }

template<>
QTextStream& readValue(QTextStream& stream, QVector<QString>& v)
{
    v.clear();
    while(!stream.atEnd())
    {
        QString t = stream.readLine();
        if(!t.isEmpty())
            v.append(t);
    }
    return stream;
}

template<class T>
QTextStream& readValue(QTextStream& stream, QVector<T>& v)
{
    v.clear();
    T t = T();
    while(!stream.atEnd())
    {
        readValue(stream, t);
        v.append(t);
    }
    return stream;
}

template<class T>
QTextStream& readValue(QTextStream& stream, Animation<T>& v)
{
    v.clear();
    T val = T();
    double key;
    while(!stream.atEnd())
    {
        stream >> key;
        readValue(stream, val);
        v.add(key, val);
    }
    return stream;
}

template<>
QTextStream& readValue(QTextStream& stream, QImage&)
{ return stream; } // Not saving images (save it as a separate file and use a LoadImage object)

template<>
QTextStream& readValue(QTextStream& stream, QString& v)
{ v = stream.readLine(); return stream; }

template<class T>
QTextStream& readValue(QTextStream& stream, T& v)
{ return stream >> v; }

template<class T>
void Data<T>::fromString(const QString& text)
{
    QString copy;
    if(getValueType() != QMetaType::QString)
        copy = text.simplified();
	else
		copy = text;
    QTextStream stream(&copy, QIODevice::ReadOnly);
    beginEdit();
    readValue(stream, value);
    endEdit();
}

//***************************************************************//

template<class T>
bool dataIsSingleValue(const Data<T>*);

template<class T>
bool dataIsSingleValue(const Data< QVector<T> >*) { return false; }

template<class T>
bool dataIsSingleValue(const Data< Animation<T> >*) { return false; }

template<class T>
bool dataIsSingleValue(const Data<T>*) { return true; }

template<class T>
bool Data<T>::isSingleValue() const { return dataIsSingleValue(this); }

//***************************************************************//

template<class T>
bool dataIsVectorType(const Data<T>*);

template<class T>
bool dataIsVectorType(const Data< QVector<T> >*) { return true; }

template<class T>
bool dataIsVectorType(const Data< Animation<T> >*) { return false; }

template<class T>
bool dataIsVectorType(const Data<T>*) { return false; }

template<class T>
bool Data<T>::isVector() const { return dataIsVectorType(this); }

//***************************************************************//

template<class T>
bool dataIsAnimation(const Data<T>*);

template<class T>
bool dataIsAnimation(const Data< QVector<T> >*) { return false; }

template<class T>
bool dataIsAnimation(const Data< Animation<T> >*) { return true; }

template<class T>
bool dataIsAnimation(const Data<T>*) { return false; }

template<class T>
bool Data<T>::isAnimation() const { return dataIsAnimation(this); }

//***************************************************************//

template<class T>
int getDataValueType(const Data<T>*);

template<class T>
int getDataValueType(const Data< QVector<T> >*) { return BaseData::getValueTypeOf<T>(); }

template<class T>
int getDataValueType(const Data< Animation<T> >*) { return BaseData::getValueTypeOf<T>(); }

template<class T>
int getDataValueType(const Data<T>*) { return BaseData::getValueTypeOf<T>(); }

template<class T>
int Data<T>::getValueType() const { return getDataValueType(this); }

//***************************************************************//

template<class T>
int getDataSize(const Data<T>*);

template<class T>
int getDataSize(const Data< QVector<T> >* data) { return data->getValue().size(); }

template<class T>
int getDataSize(const Data< Animation<T> >* data) { return data->getValue().size(); }

template<class T>
int getDataSize(const Data<T>*) { return 1; }

template<class T>
int Data<T>::getSize() const { return getDataSize(this); }

//***************************************************************//

template<class T>
void clearData(Data<T>*, int size, bool init);

template<class T>
void clearData(Data< QVector<T> >* data, int size, bool init)
{
    QVector<T>& vec = *data->beginEdit();
    if(init)
        vec.clear();
    vec.resize(size);
    data->endEdit();
}

template<class T>
void clearData(Data< Animation<T> >* data, int /*size*/, bool /*init*/)
{
    data->beginEdit()->clear();
    data->endEdit();
}

template<class T>
void clearData(Data<T>* data, int /*size*/, bool init)
{
    if(init)
        data->setValue(T());
}

template<class T>
void Data<T>::clear(int size, bool init)
{
    clearData(this, size, init);
}

//***************************************************************//

template<class T>
QVariant getDataBaseValue(const Data<T>*, int);

template<class T>
QVariant getDataBaseValue(const Data< QVector<T> >* data, int index)
{
    QVariant temp;
    if(index < 0 || index >= data->getValue().size())
        temp.setValue(T());
    else
        temp.setValue(data->getValue()[index]);
    return temp;
}

template<class T>
QVariant getDataBaseValue(const Data< Animation<T> >* data, int index)
{
    QVariant temp;
    if(index < 0 || index >= data->getValue().size())
        temp.setValue(T());
    else
        temp.setValue(data->getValue().getValueAtIndexConst(index));
    return temp;
}

template<class T>
QVariant getDataBaseValue(const Data<T>* data, int)
{
    QVariant temp;
    temp.setValue(data->getValue());
    return temp;
}

template<class T>
QVariant Data<T>::getBaseValue(int index) const { return getDataBaseValue(this, index); }

//***************************************************************//

template<class T>
void setDataFromBaseValue(Data<T>*, QVariant val, int);

template<class T>
void setDataFromBaseValue(Data< QVector<T> >* data, QVariant val, int index)
{
    QVector<T>& vec = *data->beginEdit();
    if(vec.size() <= index)
        vec.resize(index+1);
    vec[index] = val.value<T>();
    data->endEdit();
}

template<class T>
void setDataFromBaseValue(Data< Animation<T> >* data, QVariant val, int index)
{
    Animation<T>& vec = *data->beginEdit();
    vec.getValueAtIndex(index) = val.value<T>();
    data->endEdit();
}

template<class T>
void setDataFromBaseValue(Data<T>* data, QVariant val, int)
{
    data->setValue(val.value<T>());
}

template<class T>
void Data<T>::fromBaseValue(QVariant val, int index) { setDataFromBaseValue(this, val, index); }

//***************************************************************//

template<class T>
void copyValue(Data<T>* data, const BaseData* parent);

template<class T>
void copyValue(Data< QVector<T> >* data, const BaseData* parent)
{
    // First we try without conversion
    if(parent->isVector())
    {
        // Same type (both vectors)
        const Data< QVector<T> >* castedParent = dynamic_cast<const Data< QVector<T> >*>(parent);
        if(castedParent)
        {
            data->setValue(castedParent->getValue());
            return;
        }
    }
    else if(parent->isAnimation())
    {
        // The parent is not a vector of T, but an animation of type T
        const Data< Animation<T> >* castedAnimationParent = dynamic_cast<const Data< Animation<T> >*>(parent);
        if(castedAnimationParent)
        {
            QVector<T>& vec = *data->beginEdit();
            vec = castedAnimationParent->getValue().getValues().toVector();
            data->endEdit();
            return;
        }
    }
    else if(parent->isSingleValue())
    {
        // The parent is not a vector of T, but a single value of type T
        const Data<T>* castedSingleValueParent = dynamic_cast<const Data<T>*>(parent);
        if(castedSingleValueParent)
        {
            QVector<T>& vec = *data->beginEdit();
            vec.clear();
            vec.append(castedSingleValueParent->getValue());
            data->endEdit();
            return;
        }
    }

    // Else we use QVariant for a conversion
    QVector<T>& value = *data->beginEdit();
    value.clear();
    int size = parent->getSize();
    for(int i=0; i<size; ++i)
        value.append(parent->getBaseValue(i).value<T>());
    data->endEdit();
}

template<class T>
void copyValue(Data< Animation<T> >* data, const BaseData* parent)
{
    // Without conversion
    if(parent->isAnimation())
    {
        // Same type (both animations)
        const Data< Animation<T> >* castedAnimationParent = dynamic_cast<const Data< Animation<T> >*>(parent);
        if(castedAnimationParent)
        {
            data->setValue(castedAnimationParent->getValue());
            return;
        }
    }

    // Not accepting conversions from non-animation datas
    Animation<T>& value = *data->beginEdit();
    value.clear();
    data->endEdit();
}

template<class T>
void copyValue(Data<T>* data, const BaseData* parent)
{
    // First we try without conversion
    if(parent->isVector())
    {
        // The parent is a vector of T
        const Data< QVector<T> >* castedVectorParent = dynamic_cast<const Data< QVector<T> >*>(parent);
        if(castedVectorParent)
        {
            if(castedVectorParent->getValue().size())
                data->setValue(castedVectorParent->getValue()[0]);
            else
                data->setValue(T());
            return;
        }
    }
    else if(parent->isSingleValue())
    {
        // Same type
        const Data<T>* castedParent = dynamic_cast<const Data<T>*>(parent);
        if(castedParent)
        {
            data->setValue(castedParent->getValue());
            return;
        }
    }

    // Else we use QVariant for a conversion
    data->setValue(parent->getBaseValue(0).value<T>());
}

template<class T>
void Data<T>::copyValueFrom(const BaseData* parent) { copyValue(this, parent); this->isValueSet = true; }

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
