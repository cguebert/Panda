#include <panda/types/Point.h>

#include <panda/types/Animation.inl>
#include <panda/DataFactory.h>
#include <panda/Data.inl>

namespace panda
{

namespace types
{

template<> QString DataTrait<Point>::valueTypeName() { return "point"; }

template<>
void DataTrait<Point>::writeValue(QDomDocument&, QDomElement& elem, const Point& v)
{ elem.setAttribute("x", v.x); elem.setAttribute("y", v.y); }

template<>
void DataTrait<Point>::readValue(QDomElement& elem, Point& v)
#ifdef PANDA_DOUBLE
{	v.x = elem.attribute("x").toDouble();
	v;y = elem.attribute("y").toDouble(); }
#else
{	v.x = elem.attribute("x").toFloat();
	v.y = elem.attribute("y").toFloat(); }
#endif

template class Animation<Point>;

template class Data< Point >;
template class Data< QVector<Point> >;
template class Data< Animation<Point> >;

int pointDataClass = RegisterData< Point >();
int pointVectorDataClass = RegisterData< QVector<Point> >();
int pointAnimationDataClass = RegisterData< Animation<Point> >();

} // namespace types

} // namespace panda
