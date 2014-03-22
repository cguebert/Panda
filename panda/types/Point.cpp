#include <panda/types/Point.h>
#include <panda/types/Path.h>

#include <panda/types/Animation.inl>
#include <panda/types/AnimationTraits.h>

#include <panda/DataFactory.h>
#include <panda/Data.inl>

namespace panda
{

namespace types
{

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

//*************************************************************************//

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


template class Data< Point >;
template class Data< QVector<Point> >;

template class Animation<Point>;
template class Data< Animation<Point> >;

int pointDataClass = RegisterData< Point >();
int pointVectorDataClass = RegisterData< QVector<Point> >();
int pointAnimationDataClass = RegisterData< Animation<Point> >();

//*************************************************************************//

Path& Path::operator=(const QVector<Point> &v)
{
	QVector<Point>::operator=(v);
	return *this;
}

template<> QString DataTrait<Path>::valueTypeName() { return "path"; }
template<> int DataTrait<Path>::size(const Path& v) { return v.size(); }
template<> void DataTrait<Path>::clear(Path& v, int size, bool init)
{
	if(init)
		v.clear();
	v.resize(size);
}

template<>
void DataTrait<Path>::writeValue(QDomDocument& doc, QDomElement& elem, const Path& path)
{
	for(const auto& pt : path)
	{
		QDomElement ptNode = doc.createElement("Point");
		ptNode.setAttribute("x", pt.x);
		ptNode.setAttribute("y", pt.y);
		elem.appendChild(ptNode);
	}
}

template<>
void DataTrait<Path>::readValue(QDomElement& elem, Path& path)
{
	path.clear();

	QDomElement ptNode = elem.firstChildElement("Point");
	while(!ptNode.isNull())
	{
#ifdef PANDA_DOUBLE
		double x = ptNode.attribute("x").toDouble();
		double y = ptNode.attribute("y").toDouble();
#else
		double x = ptNode.attribute("x").toFloat();
		double y = ptNode.attribute("y").toFloat();
#endif

		path.push_back(Point(x, y));
		ptNode = ptNode.nextSiblingElement("Point");
	}
}

template class Data< Path >;
template class Data< QVector<Path> >;

int pathDataClass = RegisterData< Path >();
int pathVectorDataClass = RegisterData< QVector<Path> >();

} // namespace types

} // namespace panda

void convertType(const panda::types::Path& from, QVector<panda::types::Point>& to)
{ to = static_cast< QVector<panda::types::Point> >(from); }
void convertType(const QVector<panda::types::Point>& from, panda::types::Path& to)
{ to = from; }

panda::types::RegisterTypeConverter<panda::types::Path, QVector<panda::types::Point> > PathPointsConverter;
panda::types::RegisterTypeConverter<QVector<panda::types::Point>, panda::types::Path> PointsPathConverter;
