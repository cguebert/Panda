#ifndef TYPES_ANIMATIONTRAITS_H
#define TYPES_ANIMATIONTRAITS_H

#include <panda/types/DataTraits.h>
#include <panda/types/Animation.h>

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

	static std::string valueTypeName() { return base_trait::valueTypeName(); }
	static std::string valueTypeNamePlural() { return base_trait::valueTypeNamePlural(); }
	static std::string typeName() { return valueTypeName() + "_animation"; }
	static std::string typeDescription() { return "animation of " + valueTypeNamePlural(); }
	static const std::type_info& typeInfo() { return typeid(animation_type); }
	static int valueTypeId() { return DataTypeId::getIdOf<value_type>(); }
	static int fullTypeId() { return DataTypeId::getFullTypeOfAnimation(valueTypeId()); }
	static int size(const animation_type& a) { return a.size(); }
	static void clear(animation_type& a, int /*size*/, bool /*init*/)
	{
		a.clear();
	}
	static const void* getVoidValue(const animation_type& anim, int index)
	{
		if(index < 0 || index >= anim.size())
			return nullptr;
		return &anim.getAtIndex(index);
	}
	static void* getVoidValue(animation_type& anim, int index)
	{
		if(index < 0 || index >= anim.size())
			return nullptr;
		return &anim.getAtIndex(index);
	}
	static void writeValue(XmlElement& elem, const animation_type& anim)
	{
		elem.setAttribute("extend", anim.getExtend());
		elem.setAttribute("interpolation", anim.getInterpolation());
		for(auto stop : anim.getStops())
		{
			auto node = elem.addChild("Value");
			node.setAttribute("key", stop.first);
			base_trait::writeValue(node, stop.second);
		}
	}
	static void readValue(XmlElement& elem, animation_type& anim)
	{
		anim.setExtend(elem.attribute("extend").toInt());
		anim.setInterpolation(elem.attribute("interpolation").toInt());
		anim.clear();
		T val = T();
		PReal key;
		auto e = elem.firstChild("Value");
		while(e)
		{
#ifdef PANDA_DOUBLE
			key = e.attribute("key").toDouble();
#else
			key = e.attribute("key").toFloat();
#endif
			base_trait::readValue(e, val);
			anim.add(key, val);
			e = e.nextSibling("Value");
		}
	}
};

} // namespace types

} // namespace panda

#endif // TYPES_ANIMATIONTRAITS_H
