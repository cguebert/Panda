#include <panda/types/Rect.h>

#include <panda/data/DataFactory.h>

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

Rect Rect::operator|(const Point &p) const
{
	if(empty())
		return Rect(p.x, p.y, p.x + 1, p.y + 1);

	return Rect(std::min(x1, p.x), std::min(y1, p.y),
				std::max(x2, p.x), std::max(y2, p.y));
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
template<> PANDA_CORE_API unsigned int DataTrait<Rect>::typeColor() { return 0xD09A39; }

template<>
PANDA_CORE_API void DataTrait<Rect>::writeValue(XmlElement& elem, const Rect& v)
{	elem.setAttribute("l", v.left());
	elem.setAttribute("t", v.top());
	elem.setAttribute("r", v.right());
	elem.setAttribute("b", v.bottom()); }

template<>
PANDA_CORE_API void DataTrait<Rect>::readValue(const XmlElement& elem, Rect& v)
{
	v.setLeft(  elem.attribute("l").toFloat());
	v.setTop(   elem.attribute("t").toFloat());
	v.setRight( elem.attribute("r").toFloat());
	v.setBottom(elem.attribute("b").toFloat());
}

template class PANDA_CORE_API Data< Rect >;
template class PANDA_CORE_API Data< std::vector<Rect> >;

int rectDataClass = RegisterData< Rect >();
int rectVectorDataClass = RegisterData< std::vector<Rect> >();

} // namespace types

} // namespace panda
