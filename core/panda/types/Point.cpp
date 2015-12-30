#include <panda/types/Point.h>

#include <panda/types/Animation.inl>
#include <panda/types/AnimationTraits.h>

#include <panda/DataFactory.h>

namespace panda
{

namespace types
{

void rotate(Point& point, const Point& center, PReal angle)
{
	PReal ca = cos(angle), sa = sin(angle);
	Point pt = point - center;
	point = center + Point(pt.x*ca-pt.y*sa, pt.x*sa+pt.y*ca);
}

Point rotated(const Point& point, const Point& center, PReal angle)
{
	PReal ca = cos(angle), sa = sin(angle);
	Point pt = point - center;
	return center + Point(pt.x*ca-pt.y*sa, pt.x*sa+pt.y*ca);
}

// Converts a coordinate from rectangular (Cartesian) coordinates to polar coordinates of the form (radius, theta)
Point toPolar(Point car)
{
	const PReal epsilon = (PReal)0.0000001;
	PReal theta;

	if(abs(car.x) < epsilon)
	{	// x == 0
		if(abs(car.y) < epsilon) theta = 0;
		else if(car.y > 0) theta = (PReal)M_PI / 2;
		else theta = ((PReal)M_PI * 3) / 2;
	}
	else if(car.x > 0)
	{
		if(car.y < 0) theta = atan(car.y / car.x) + 2 * (PReal)M_PI;
		else theta = atan(car.y / car.x);
	}
	else // car.x < 0
	{
		theta = (atan(car.y / car.x) + (PReal)M_PI);
	}

	return Point(car.norm(), theta);
}

// Converts a coordinate from polar coordinates of the form (radius, theta) to rectangular coordinates
Point fromPolar(Point pol)
{
	return Point(cos(pol.y) * pol.x, sin(pol.y) * pol.x);
}

//****************************************************************************//

template<> PANDA_CORE_API std::string DataTrait<Point>::valueTypeName() { return "point"; }

template<>
PANDA_CORE_API void DataTrait<Point>::writeValue(XmlElement& elem, const Point& v)
{ elem.setAttribute("x", v.x); elem.setAttribute("y", v.y); }

template<>
PANDA_CORE_API void DataTrait<Point>::readValue(XmlElement& elem, Point& v)
#ifdef PANDA_DOUBLE
{	v.x = elem.attribute("x").toDouble();
	v.y = elem.attribute("y").toDouble(); }
#else
{	v.x = elem.attribute("x").toFloat();
	v.y = elem.attribute("y").toFloat(); }
#endif

template class PANDA_CORE_API std::vector<Point>;

template class PANDA_CORE_API Data< Point >;
template class PANDA_CORE_API Data< std::vector<Point> >;

template class PANDA_CORE_API Animation<Point>;
template class PANDA_CORE_API Data< Animation<Point> >;

int pointDataClass = RegisterData< Point >();
int pointVectorDataClass = RegisterData< std::vector<Point> >();
int pointAnimationDataClass = RegisterData< Animation<Point> >();

} // namespace types

} // namespace panda
