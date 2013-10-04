#ifndef DATATRAITS_H
#define DATATRAITS_H

#include <panda/Data.h>
#include <panda/Animation.h>

#include <QRectF>
#include <QPointF>
#include <QVector>
#include <QStringList>
#include <QTextStream>
#include <QDomDocument>

namespace panda
{

template<class T> class Data;

//***************************************************************//

template<class T>
class data_trait
{
public:
	typedef Data<T> data_type;
	typedef T value_type;

	enum { is_single = 1 };
	enum { is_vector = 0 };
	enum { is_animation = 0 };

	static QString valueTypeName() { return ""; } // Override for each type
	static QString valueTypeNamePlural() { return valueTypeName() + "s"; }
	static QString description() { return valueTypeName(); }
	static int valueType() { return BaseData::getValueTypeOf<value_type>(); }
	static int fullType() { return BaseData::getFullTypeOfSingleValue(valueType()); }
	static bool isDisplayed() { return true; }
	static bool isPersistent() { return true; }
	static bool isNumerical() { return false; }
	static int size(const data_type& /*d*/) { return 1; }
	static void clear(data_type& d, int /*size*/, bool init) { if(init) d.setValue(T()); }
	static double getNumerical(const value_type& /*v*/, int /*index*/) { return 0; }
	static void setNumerical(value_type& /*v*/, double /*val*/, int /*index*/) { }
	static QTextStream& writeValue(QTextStream& stream, const value_type& v) { return stream << v; }
	static QTextStream& readValue(QTextStream& stream, value_type& v) { return stream >> v; }
	static void writeValue(QDomDocument&, QDomElement&, const value_type&) {}
	static void readValue(QDomElement&, value_type&) {}
	static void copyValue(data_type* data, const BaseData* parent)
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

		// Else we try a conversion
		if(data->isNumerical() && parent->isNumerical())
		{
			auto value = data->getAccessor();
			setNumerical(value.wref(), parent->getNumerical(0), 0);
		}
	}
};

template<class T>
class data_trait< QVector<T> >
{
public:
	typedef QVector<T> vector_type;
	typedef Data<vector_type> data_type;
	typedef T value_type;
	typedef data_trait<T> base_traits;

	enum { is_single = 0 };
	enum { is_vector = 1 };
	enum { is_animation = 0 };

	static QString valueTypeName() { return base_traits::valueTypeName(); }
	static QString valueTypeNamePlural() { return base_traits::valueTypeNamePlural(); }
	static QString description() { return valueTypeName() + "_vector"; }
	static int valueType() { return BaseData::getValueTypeOf<value_type>(); }
	static int fullType() { return BaseData::getFullTypeOfVector(valueType()); }
	static bool isDisplayed() { return base_traits::isDisplayed(); }
	static bool isPersistent() { return base_traits::isPersistent(); }
	static bool isNumerical() { return base_traits::isNumerical(); }
	static int size(const data_type& d) { return d.getValue().size(); }
	static void clear(data_type& d, int size, bool init)
	{
		auto v = d.getAccessor();
		if(init)
			v.clear();
		v.resize(size);
	}
	static double getNumerical(const vector_type& vec, int index)
	{
		if(index < 0 || index >= vec.size())
			return 0.0;
		return base_traits::getNumerical(vec[index], 0);
	}
	static void setNumerical(vector_type& vec, double val, int index)
	{
		if(index >= 0 && index < vec.size())
			base_traits::setNumerical(vec[index], val, 0);
	}
	static QTextStream& writeValue(QTextStream& stream, const vector_type& vec)
	{
		int size = vec.size();
		if(size)
		{
			base_traits::writeValue(stream, vec[0]);
			for(int i=1; i<size; ++i)
			{
				stream << " ";
				base_traits::writeValue(stream, vec[i]);
			}
		}
		return stream;
	}
	static QTextStream& readValue(QTextStream& stream, vector_type& vec)
	{
		vec.clear();
		T t = T();
		while(!stream.atEnd())
		{
			base_traits::readValue(stream, t);
			vec.push_back(t);
		}
		return stream;
	}
	static void writeValue(QDomDocument& doc, QDomElement& elem, const vector_type& vec)
	{
		for(auto& v : vec)
		{
			QDomElement node = doc.createElement("Value");
			base_traits::writeValue(doc, node, v);
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
			base_traits::readValue(e, t);
			vec.push_back(t);
			e = e.nextSiblingElement("Value");
		}
	}
	static void copyValue(data_type* data, const BaseData* parent)
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
				auto vec = data->getAccessor();
				vec = castedAnimationParent->getValue().getValues().toVector();
				return;
			}
		}
		else if(parent->isSingleValue())
		{
			// The parent is not a vector of T, but a single value of type T
			const Data<T>* castedSingleValueParent = dynamic_cast<const Data<T>*>(parent);
			if(castedSingleValueParent)
			{
				auto vec = data->getAccessor();
				vec.clear();
				vec.push_back(castedSingleValueParent->getValue());
				return;
			}
		}

		// Else we try a conversion
		if(data->isNumerical() && parent->isNumerical())
		{
			int size = parent->getSize();
			auto value = data->getAccessor();
			value.resize(size);
			for(int i=0; i<size; ++i)
				setNumerical(value.wref(), parent->getNumerical(i), i);
		}
	}
};

template<class T>
class data_trait< Animation<T> >
{
public:
	typedef Animation<T> animation_type;
	typedef Data<animation_type> data_type;
	typedef T value_type;
	typedef data_trait<T> base_traits;

	enum { is_single = 0 };
	enum { is_vector = 0 };
	enum { is_animation = 1 };

	static QString valueTypeName() { return base_traits::valueTypeName(); }
	static QString valueTypeNamePlural() { return base_traits::valueTypeNamePlural(); }
	static QString description() { return valueTypeName() + "_animation"; }
	static int valueType() { return BaseData::getValueTypeOf<value_type>(); }
	static int fullType() { return BaseData::getFullTypeOfAnimation(valueType()); }
	static bool isDisplayed() { return base_traits::isDisplayed(); }
	static bool isPersistent() { return base_traits::isPersistent(); }
	static bool isNumerical() { return base_traits::isNumerical(); }
	static int size(const data_type& d) { return d.getValue().size(); }
	static void clear(data_type& d, int /*size*/, bool /*init*/)
	{
		d.getAccessor().clear();
	}
	static double getNumerical(const animation_type& /*anim*/, int /*index*/) { return 0.0; }
	static void setNumerical(animation_type& /*anim*/, double /*val*/, int /*index*/) { }
	static QTextStream& writeValue(QTextStream& stream, const animation_type& anim)
	{
		int size = anim.size();
		if(size)
		{
			typename animation_type::Iterator iter = anim.getIterator();
			while(iter.hasNext())
			{
				iter.next();
				stream << iter.key() << " ";
				base_traits::writeValue(stream, iter.value());
				stream << " ";
			}
		}
		return stream;
	}
	static QTextStream& readValue(QTextStream& stream, animation_type& anim)
	{
		anim.clear();
		T val = T();
		double key;
		while(!stream.atEnd())
		{
			stream >> key;
			base_traits::readValue(stream, val);
			anim.add(key, val);
		}
		return stream;
	}
	static void writeValue(QDomDocument& doc, QDomElement& elem, const animation_type& anim)
	{
		typename animation_type::Iterator iter = anim.getIterator();
		while(iter.hasNext())
		{
			iter.next();
			QDomElement node = doc.createElement("Value");
			node.setAttribute("key", iter.key());
			base_traits::writeValue(doc, node, iter.value());
			elem.appendChild(node);
		}
	}
	static void readValue(QDomElement& elem, animation_type& anim)
	{
		anim.clear();
		T val = T();
		double key;
		QDomElement e = elem.firstChildElement("Value");
		while(!e.isNull())
		{
			key = e.attribute("key").toDouble();
			base_traits::readValue(e, val);
			anim.add(key, val);
			e = e.nextSiblingElement("Value");
		}
	}
	static void copyValue(data_type* data, const BaseData* parent)
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
		data->getAccessor().clear();
	}
};

//***************************************************************//

template<> QString data_trait<int>::valueTypeName() { return "integer"; }
template<> QString data_trait<double>::valueTypeName() { return "real"; }
template<> QString data_trait<QColor>::valueTypeName() { return "color"; }
template<> QString data_trait<QPointF>::valueTypeName() { return "point"; }
template<> QString data_trait<QRectF>::valueTypeName() { return "rectangle"; }
template<> QString data_trait<QString>::valueTypeName() { return "text"; }
template<> QString data_trait<QImage>::valueTypeName() { return "image"; }

template<> bool data_trait<QImage>::isDisplayed() { return false; }
template<> bool data_trait<QImage>::isPersistent() { return false; }

template<> bool data_trait<int>::isNumerical() { return true; }
template<> bool data_trait<double>::isNumerical() { return true; }

//***************************************************************//
// Overrides for get/setNumerical

template<>
double data_trait<int>::getNumerical(const value_type& v, int index)
{
	if(index == 0)
		return v;
	return 0;
}

template<>
void data_trait<int>::setNumerical(value_type& v, double val, int index)
{
	if(index == 0)
		v = static_cast<int>(val);
}

template<>
double data_trait<double>::getNumerical(const value_type& v, int index)
{
	if(index == 0)
		return v;
	return 0;
}

template<>
void data_trait<double>::setNumerical(value_type& v, double val, int index)
{
	if(index == 0)
		v = val;
}

template<>
double data_trait< Animation<double> >::getNumerical(const animation_type& anim, int index)
{
	if(index < 0 || index >= anim.size())
		return 0.0;
	return anim.getValueAtIndexConst(index);
}

template<>
void data_trait< Animation<double> >::setNumerical(animation_type& anim, double val, int index)
{
	if(index >= 0 && index < anim.size())
		anim.getValueAtIndex(index) = val;
}

//***************************************************************//
// Overrides for writeValue

template<>
QTextStream& data_trait<QColor>::writeValue(QTextStream& stream, const QColor& v)
{ return stream << QString("#%1").arg(v.rgba(), 8, 16, QChar('0')).toUpper(); }

template<>
QTextStream& data_trait<QPointF>::writeValue(QTextStream& stream, const QPointF& v)
{ return stream << v.x() << " " << v.y(); }

template<>
QTextStream& data_trait<QRectF>::writeValue(QTextStream& stream, const QRectF& v)
{ return stream << v.left() << " " << v.top() << " " << v.right() << " " << v.bottom(); }

template<>
QTextStream& data_trait< QVector<QString> >::writeValue(QTextStream& stream, const QVector<QString>& v)
{
	if(v.empty())
		return stream;
	for(int i=0, nb=v.size()-1; i<nb; ++i)
		stream << v[i] << endl;
	stream << v.last();
	return stream;
}

template<>
QTextStream& data_trait<QImage>::writeValue(QTextStream& stream, const QImage&)
{ return stream; } // Use a SaveImage object instead

//***************************************************************//
// Overrides for readValue

template<>
QTextStream& data_trait<QColor>::readValue(QTextStream& stream, QColor& v)
{
	QString temp;
	stream >> temp;
	if(temp.startsWith('#'))
		temp = temp.mid(1);
	v.setRgba(temp.toUInt(nullptr, 16));
	return stream;
}

template<>
QTextStream& data_trait<QRectF>::readValue(QTextStream& stream, QRectF& v)
{
	double l, t, r, b;
	stream >> l >> t >> r >> b;
	v = QRectF(l, t, r-l, b-t).normalized();
	return stream;
}

template<>
QTextStream& data_trait<QPointF>::readValue(QTextStream& stream, QPointF& v)
{ return stream >> v.rx() >> v.ry(); }

template<>
QTextStream& data_trait< QVector<QString> >::readValue(QTextStream& stream, QVector<QString>& v)
{
	v.clear();
	while(!stream.atEnd())
	{
		QString t = stream.readLine();
		v.push_back(t);
	}
	return stream;
}

template<>
QTextStream& data_trait<QImage>::readValue(QTextStream& stream, QImage&)
{ return stream; } // Not saving images (save it as a separate file and use a LoadImage object)

template<>
QTextStream& data_trait<QString>::readValue(QTextStream& stream, QString& v)
{ v = stream.readLine(); return stream; }

//***************************************************************//
// Overrides for writeValue xml

template<>
void data_trait<int>::writeValue(QDomDocument&, QDomElement& elem, const int& v)
{ elem.setAttribute("int", v); }

template<>
void data_trait<double>::writeValue(QDomDocument&, QDomElement& elem, const double& v)
{ elem.setAttribute("double", v); }

template<>
void data_trait<QColor>::writeValue(QDomDocument&, QDomElement& elem, const QColor& v)
{	elem.setAttribute("r", v.red());
	elem.setAttribute("g", v.green());
	elem.setAttribute("b", v.blue());
	elem.setAttribute("a", v.alpha()); }

template<>
void data_trait<QPointF>::writeValue(QDomDocument&, QDomElement& elem, const QPointF& v)
{ elem.setAttribute("x", v.x()); elem.setAttribute("y", v.y()); }

template<>
void data_trait<QRectF>::writeValue(QDomDocument&, QDomElement& elem, const QRectF& v)
{	elem.setAttribute("l", v.left());
	elem.setAttribute("t", v.top());
	elem.setAttribute("r", v.right());
	elem.setAttribute("b", v.bottom()); }

template<>
void data_trait<QString>::writeValue(QDomDocument& doc, QDomElement& elem, const QString& v)
{
	QDomText node = doc.createTextNode(v);
	elem.appendChild(node);
}

//***************************************************************//
// Overrides for readValue xml

template<>
void data_trait<int>::readValue(QDomElement& elem, int& v)
{ v = elem.attribute("int").toInt(); }

template<>
void data_trait<double>::readValue(QDomElement& elem, double& v)
{ v = elem.attribute("double").toDouble(); }

template<>
void data_trait<QColor>::readValue(QDomElement& elem, QColor& v)
{	v.setRed(  elem.attribute("r").toInt());
	v.setGreen(elem.attribute("g").toInt());
	v.setBlue( elem.attribute("b").toInt());
	v.setAlpha(elem.attribute("a").toInt()); }

template<>
void data_trait<QPointF>::readValue(QDomElement& elem, QPointF& v)
{	v.setX(elem.attribute("x").toDouble());
	v.setY(elem.attribute("y").toDouble()); }

template<>
void data_trait<QRectF>::readValue(QDomElement& elem, QRectF& v)
{	v.setLeft(  elem.attribute("l").toDouble());
	v.setTop(   elem.attribute("t").toDouble());
	v.setRight( elem.attribute("r").toDouble());
	v.setBottom(elem.attribute("b").toDouble());
}

template<>
void data_trait<QString>::readValue(QDomElement& elem, QString& v)
{
	v = elem.text();
}

//***************************************************************//

template<class T>
T valueFromString(const QString& text)
{
	T val = T();
	QString copy = text;
	QTextStream stream(&copy, QIODevice::ReadOnly);
	data_trait<T>::readValue(stream, val);
	return val;
}

template<class T>
QString valueToString(const T& val)
{
	QString tempString;
	QTextStream stream(&tempString, QIODevice::WriteOnly);
	data_trait<T>::writeValue(stream, val);
	return tempString;
}

} // namespace panda

#endif // DATATRAITS_H
