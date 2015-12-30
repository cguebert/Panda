#include <panda/types/Rect.h>

#include <panda/DataFactory.h>
#include <panda/Data.h>

#include <algorithm>

namespace panda
{

namespace types
{

Rect Rect::operator|(const Rect &r) const
{
	if(empty())
		return r;

	if(r.empty())
		return *this;

	return Rect(std::min(x1, r.x1), std::min(y1, r.y1),
				std::max(x2, r.x2), std::max(y2, r.y2));
}

Rect Rect::operator&(const Rect &r) const
{
	if(x1 >= r.x2 || r.x1 >= x2 || y1 >= r.y2 || r.y1 >= y2)
		return Rect();

	return Rect(std::max(x1, r.x1), std::max(y1, r.y1),
				std::min(x2, r.x2), std::min(y2, r.y2));
}

bool Rect::intersects(const Rect& r) const
{
	if(empty() || r.empty())
		return false;

	if(x1 > r.x2 || r.x1 > x2 || y1 > r.y2 || r.y1 > y2)
		return false;

	return true;
}

bool Rect::contains(const Rect& r) const
{
	if(empty() || r.empty())
		return false;

	if(x1 < r.x1 || x2 > r.x2 || y1 < r.y1 || y2 > r.y2)
		return false;

	return true;
}

//****************************************************************************//

template<> PANDA_CORE_API std::string DataTrait<Rect>::valueTypeName() { return "rectangle"; }

template<>
PANDA_CORE_API void DataTrait<Rect>::writeValue(XmlElement& elem, const Rect& v)
{	elem.setAttribute("l", v.left());
	elem.setAttribute("t", v.top());
	elem.setAttribute("r", v.right());
	elem.setAttribute("b", v.bottom()); }

template<>
PANDA_CORE_API void DataTrait<Rect>::readValue(XmlElement& elem, Rect& v)
{
#ifdef PANDA_DOUBLE
	v.setLeft(  elem.attribute("l").toDouble());
	v.setTop(   elem.attribute("t").toDouble());
	v.setRight( elem.attribute("r").toDouble());
	v.setBottom(elem.attribute("b").toDouble());
#else
	v.setLeft(  elem.attribute("l").toFloat());
	v.setTop(   elem.attribute("t").toFloat());
	v.setRight( elem.attribute("r").toFloat());
	v.setBottom(elem.attribute("b").toFloat());
#endif
}

template class Data< Rect >;
template class Data< std::vector<Rect> >;

int rectDataClass = RegisterData< Rect >();
int rectVectorDataClass = RegisterData< std::vector<Rect> >();

} // namespace types

} // namespace panda
