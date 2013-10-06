#ifndef DATATRAITS_H
#define DATATRAITS_H

#include <panda/types/DataTypeId.h>

#include <QColor>
#include <QRectF>
#include <QPointF>
#include <QVector>
#include <QDomDocument>

namespace panda
{

namespace types
{

class AbstractDataTrait
{
public:
	virtual const AbstractDataTrait* baseTypeTrait() const = 0;
	virtual const AbstractDataTrait* valueTypeTrait() const = 0;

	virtual bool isSingleValue() const = 0;
	virtual bool isVector() const = 0;
	virtual bool isAnimation() const = 0;
	virtual bool isDisplayed() const = 0;
	virtual bool isPersistent() const = 0;
	virtual bool isNumerical() const = 0;

	virtual QString valueTypeName() const = 0;
	virtual QString valueTypeNamePlural() const = 0;
	virtual QString description() const = 0;
	virtual int valueTypeId() const = 0;
	virtual int fullTypeId() const = 0;

	virtual int size(const void* value) const = 0;
	virtual void clear(void* value, int size, bool init) const = 0;
	virtual double getNumerical(const void* value, int index) const = 0;
	virtual void setNumerical(void* value, double num, int index) const = 0;

	virtual void writeValue(QDomDocument& doc, QDomElement& elem, const void* value) const = 0;
	virtual void readValue(QDomElement& elem, void* value) const = 0;
};

//***************************************************************//

template<class T>
class DataTrait
{
public:
	typedef T base_type;
	typedef T value_type;

	enum { is_single = 1 };
	enum { is_vector = 0 };
	enum { is_animation = 0 };
	static bool isDisplayed() { return true; }
	static bool isPersistent() { return true; }
	static bool isNumerical() { return false; }

	static QString valueTypeName() { return ""; } // Override for each type
	static QString valueTypeNamePlural() { return valueTypeName() + "s"; }
	static QString description() { return valueTypeName(); }
	static int valueTypeId() { return DataTypeId::getIdOf<value_type>(); }
	static int fullTypeId() { return DataTypeId::getFullTypeOfSingleValue(valueTypeId()); }
	static int size(const value_type& /*v*/) { return 1; }
	static void clear(value_type& v, int /*size*/, bool init) { if(init) v = T(); }
	static double getNumerical(const value_type& /*v*/, int /*index*/) { return 0; }
	static void setNumerical(value_type& /*v*/, double /*val*/, int /*index*/) { }
	static void writeValue(QDomDocument&, QDomElement&, const value_type&) {}
	static void readValue(QDomElement&, value_type&) {}
};

//***************************************************************//

template<class T>
class VirtualDataTrait : public AbstractDataTrait
{
public:
	typedef T value_type;
	typedef DataTrait<value_type> value_trait;

	static VirtualDataTrait* get() { static VirtualDataTrait<value_type> trait; return &trait; }

	virtual const AbstractDataTrait* baseTypeTrait() const
	{ return VirtualDataTrait<value_trait::base_type>::get(); }
	virtual const AbstractDataTrait* valueTypeTrait() const
	{ return VirtualDataTrait<value_trait::value_type>::get(); }

	virtual bool isSingleValue() const	{ return value_trait::is_single; }
	virtual bool isVector() const		{ return value_trait::is_vector; }
	virtual bool isAnimation() const	{ return value_trait::is_animation; }
	virtual bool isDisplayed() const	{ return value_trait::isDisplayed(); }
	virtual bool isPersistent() const	{ return value_trait::isPersistent(); }
	virtual bool isNumerical() const	{ return value_trait::isNumerical(); }

	virtual QString valueTypeName() const { return value_trait::valueTypeName(); }
	virtual QString valueTypeNamePlural() const { return value_trait::valueTypeNamePlural(); }
	virtual QString description() const { return value_trait::description(); }
	virtual int valueTypeId() const { return value_trait::valueTypeId(); }
	virtual int fullTypeId() const { return value_trait::fullTypeId(); }

	virtual int size(const void* value) const
	{ return value_trait::size(*static_cast<const value_type*>(value)); }
	virtual void clear(void* value, int size, bool init) const
	{ return value_trait::clear(*static_cast<value_type*>(value), size, init); }
	virtual double getNumerical(const void* value, int index) const
	{ return value_trait::getNumerical(*static_cast<const value_type*>(value), index); }
	virtual void setNumerical(void* value, double num, int index) const
	{ return value_trait::setNumerical(*static_cast<value_type*>(value), num, index); }

	virtual void writeValue(QDomDocument& doc, QDomElement& elem, const void* value) const
	{ return value_trait::writeValue(doc, elem, *static_cast<const value_type*>(value)); }
	virtual void readValue(QDomElement& elem, void* value) const
	{ return value_trait::readValue(elem, *static_cast<value_type*>(value)); }
};

//***************************************************************//


template<class T>
class DataTrait< QVector<T> >
{
public:
	typedef QVector<T> vector_type;
	typedef T base_type;
	typedef T value_type;
	typedef DataTrait<base_type> base_trait;

	enum { is_single = 0 };
	enum { is_vector = 1 };
	enum { is_animation = 0 };
	static bool isDisplayed() { return base_trait::isDisplayed(); }
	static bool isPersistent() { return base_trait::isPersistent(); }
	static bool isNumerical() { return base_trait::isNumerical(); }

	static QString valueTypeName() { return base_trait::valueTypeName(); }
	static QString valueTypeNamePlural() { return base_trait::valueTypeNamePlural(); }
	static QString description() { return valueTypeName() + "_vector"; }
	static int valueTypeId() { return DataTypeId::getIdOf<value_type>(); }
	static int fullTypeId() { return DataTypeId::getFullTypeOfVector(valueTypeId()); }
	static int size(const vector_type& v) { return v.size(); }
	static void clear(vector_type& v, int size, bool init)
	{
		if(init)
			v.clear();
		v.resize(size);
	}
	static double getNumerical(const vector_type& vec, int index)
	{
		if(index < 0 || index >= vec.size())
			return 0.0;
		return base_trait::getNumerical(vec[index], 0);
	}
	static void setNumerical(vector_type& vec, double val, int index)
	{
		if(index >= 0 && index < vec.size())
			base_trait::setNumerical(vec[index], val, 0);
	}
	static void writeValue(QDomDocument& doc, QDomElement& elem, const vector_type& vec)
	{
		for(auto& v : vec)
		{
			QDomElement node = doc.createElement("Value");
			base_trait::writeValue(doc, node, v);
			elem.appendChild(node);
		}
	}
	static void readValue(QDomElement& elem, vector_type& vec)
	{
		vec.clear();
		T t = T();
		QDomElement e = elem.firstChildElement("Value");
		while(!e.isNull())
		{
			base_trait::readValue(e, t);
			vec.push_back(t);
			e = e.nextSiblingElement("Value");
		}
	}
};

//***************************************************************//

template<> QString DataTrait<int>::valueTypeName() { return "integer"; }
template<> QString DataTrait<double>::valueTypeName() { return "real"; }
template<> QString DataTrait<QColor>::valueTypeName() { return "color"; }
template<> QString DataTrait<QPointF>::valueTypeName() { return "point"; }
template<> QString DataTrait<QRectF>::valueTypeName() { return "rectangle"; }
template<> QString DataTrait<QString>::valueTypeName() { return "text"; }
template<> QString DataTrait<QImage>::valueTypeName() { return "image"; }

template<> bool DataTrait<QImage>::isDisplayed() { return false; }
template<> bool DataTrait<QImage>::isPersistent() { return false; }

template<> bool DataTrait<int>::isNumerical() { return true; }
template<> bool DataTrait<double>::isNumerical() { return true; }

//***************************************************************//
// Overrides for get/setNumerical

template<>
double DataTrait<int>::getNumerical(const value_type& v, int index)
{
	if(index == 0)
		return v;
	return 0;
}

template<>
void DataTrait<int>::setNumerical(value_type& v, double val, int index)
{
	if(index == 0)
		v = static_cast<int>(val);
}

template<>
double DataTrait<double>::getNumerical(const value_type& v, int index)
{
	if(index == 0)
		return v;
	return 0;
}

template<>
void DataTrait<double>::setNumerical(value_type& v, double val, int index)
{
	if(index == 0)
		v = val;
}

//***************************************************************//
// Overrides for writeValue xml

template<>
void DataTrait<int>::writeValue(QDomDocument&, QDomElement& elem, const int& v)
{ elem.setAttribute("int", v); }

template<>
void DataTrait<double>::writeValue(QDomDocument&, QDomElement& elem, const double& v)
{ elem.setAttribute("double", v); }

template<>
void DataTrait<QColor>::writeValue(QDomDocument&, QDomElement& elem, const QColor& v)
{	elem.setAttribute("r", v.red());
	elem.setAttribute("g", v.green());
	elem.setAttribute("b", v.blue());
	elem.setAttribute("a", v.alpha()); }

template<>
void DataTrait<QPointF>::writeValue(QDomDocument&, QDomElement& elem, const QPointF& v)
{ elem.setAttribute("x", v.x()); elem.setAttribute("y", v.y()); }

template<>
void DataTrait<QRectF>::writeValue(QDomDocument&, QDomElement& elem, const QRectF& v)
{	elem.setAttribute("l", v.left());
	elem.setAttribute("t", v.top());
	elem.setAttribute("r", v.right());
	elem.setAttribute("b", v.bottom()); }

template<>
void DataTrait<QString>::writeValue(QDomDocument& doc, QDomElement& elem, const QString& v)
{
	QDomText node = doc.createTextNode(v);
	elem.appendChild(node);
}

//***************************************************************//
// Overrides for readValue xml

template<>
void DataTrait<int>::readValue(QDomElement& elem, int& v)
{ v = elem.attribute("int").toInt(); }

template<>
void DataTrait<double>::readValue(QDomElement& elem, double& v)
{ v = elem.attribute("double").toDouble(); }

template<>
void DataTrait<QColor>::readValue(QDomElement& elem, QColor& v)
{	v.setRed(  elem.attribute("r").toInt());
	v.setGreen(elem.attribute("g").toInt());
	v.setBlue( elem.attribute("b").toInt());
	v.setAlpha(elem.attribute("a").toInt()); }

template<>
void DataTrait<QPointF>::readValue(QDomElement& elem, QPointF& v)
{	v.setX(elem.attribute("x").toDouble());
	v.setY(elem.attribute("y").toDouble()); }

template<>
void DataTrait<QRectF>::readValue(QDomElement& elem, QRectF& v)
{	v.setLeft(  elem.attribute("l").toDouble());
	v.setTop(   elem.attribute("t").toDouble());
	v.setRight( elem.attribute("r").toDouble());
	v.setBottom(elem.attribute("b").toDouble());
}

template<>
void DataTrait<QString>::readValue(QDomElement& elem, QString& v)
{
	v = elem.text();
}

} // namespace types

} // namespace panda

#endif // DATATRAITS_H
