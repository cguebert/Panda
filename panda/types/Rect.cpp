#include <panda/types/Rect.h>

#include <panda/DataFactory.h>
#include <panda/Data.inl>

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

//***************************************************************//

template<> QString DataTrait<Rect>::valueTypeName() { return "rectangle"; }

template<>
void DataTrait<Rect>::writeValue(QDomDocument&, QDomElement& elem, const Rect& v)
{	elem.setAttribute("l", v.left());
	elem.setAttribute("t", v.top());
	elem.setAttribute("r", v.right());
	elem.setAttribute("b", v.bottom()); }

template<>
void DataTrait<Rect>::readValue(QDomElement& elem, Rect& v)
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
template class Data< QVector<Rect> >;

int rectDataClass = RegisterData< Rect >();
int rectVectorDataClass = RegisterData< QVector<Rect> >();

} // namespace types

} // namespace panda
