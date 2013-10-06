#ifndef ANIMATIONTRAITS_H
#define ANIMATIONTRAITS_H

#include <panda/types/DataTraits.h>
#include <panda/Data.h>
#include <panda/types/Animation.h>

#include <QPointF>
#include <QVector>
#include <QDomDocument>

namespace panda
{

namespace types
{

template<class T>
class DataTrait< Animation<T> >
{
public:
	typedef Animation<T> animation_type;
	typedef T base_type;
	typedef T value_type;
	typedef DataTrait<base_type> base_trait;

	enum { is_single = 0 };
	enum { is_vector = 0 };
	enum { is_animation = 1 };
	static bool isDisplayed() { return base_trait::isDisplayed(); }
	static bool isPersistent() { return base_trait::isPersistent(); }
	static bool isNumerical() { return base_trait::isNumerical(); }

	static QString valueTypeName() { return base_trait::valueTypeName(); }
	static QString valueTypeNamePlural() { return base_trait::valueTypeNamePlural(); }
	static QString description() { return valueTypeName() + "_animation"; }
	static int valueTypeId() { return DataTypeId::getIdOf<value_type>(); }
	static int fullTypeId() { return DataTypeId::getFullTypeOfAnimation(valueTypeId()); }
	static int size(const animation_type& a) { return a.size(); }
	static void clear(animation_type& a, int /*size*/, bool /*init*/)
	{
		a.clear();
	}
	static double getNumerical(const animation_type& /*anim*/, int /*index*/) { return 0.0; }
	static void setNumerical(animation_type& /*anim*/, double /*val*/, int /*index*/) { }
	static void writeValue(QDomDocument& doc, QDomElement& elem, const animation_type& anim)
	{
		for(auto stop : anim.getStops())
		{
			QDomElement node = doc.createElement("Value");
			node.setAttribute("key", stop.first);
			base_trait::writeValue(doc, node, stop.second);
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
			base_trait::readValue(e, val);
			anim.add(key, val);
			e = e.nextSiblingElement("Value");
		}
	}
};

//***************************************************************//

template<>
double DataTrait< Animation<double> >::getNumerical(const animation_type& anim, int index)
{
	if(index < 0 || index >= anim.size())
		return 0.0;
	return anim.getAtIndex(index);
}

template<>
void DataTrait< Animation<double> >::setNumerical(animation_type& anim, double val, int index)
{
	if(index >= 0 && index < anim.size())
		anim.getAtIndex(index) = val;
}

} // namespace types

} // namespace panda

#endif // ANIMATIONTRAITS_H
