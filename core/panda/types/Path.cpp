#include <panda/types/Point.h>
#include <panda/types/Path.h>

#include <panda/data/DataFactory.h>

namespace panda
{

namespace types
{

Path Path::operator+(const Point& p) const
{
	const int nb = points.size();
	std::vector<Point> tmp(nb);
	for (int i = 0; i < nb; ++i)
		tmp[i] = points[i] + p;
	return tmp;
}

Path Path::operator-(const Point& p) const
{
	const int nb = points.size();
	std::vector<Point> tmp(nb);
	for (int i = 0; i < nb; ++i)
		tmp[i] = points[i] - p;
	return tmp;
}

Path Path::operator*(float v) const
{
	const int nb = points.size();
	std::vector<Point> tmp(nb);
	for (int i = 0; i < nb; ++i)
		tmp[i] = points[i] * v;
	return tmp;
}

Path Path::operator/(float v) const
{
	const int nb = points.size();
	std::vector<Point> tmp(nb);
	for (int i = 0; i < nb; ++i)
		tmp[i] = points[i] / v;
	return tmp;
}

Path operator*(float v, const Path& p)
{
	const int nb = p.points.size();
	std::vector<Point> tmp(nb);
	for (int i = 0; i < nb; ++i)
		tmp[i] = p.points[i] * v;
	return tmp;
}

Path Path::linearProduct(const Point& p) const
{
	const int nb = points.size();
	std::vector<Point> tmp(nb);
	for (int i = 0; i < nb; ++i)
		tmp[i] = points[i].linearProduct(p);
	return tmp;
}

Path Path::linearDivision(const Point& p) const
{
	const int nb = points.size();
	std::vector<Point> tmp(nb);
	for (int i = 0; i < nb; ++i)
		tmp[i] = points[i].linearDivision(p);
	return tmp;
}

Path Path::reversed() const
{
	auto tmp = points;
	std::reverse(tmp.begin(), tmp.end());
	return tmp;
}

void Path::reverse()
{
	std::reverse(points.begin(), points.end());
}

void rotate(Path& path, const Point& center, float angle)
{
	auto& points = path.points;
	int nb = points.size();
	float ca = cos(angle), sa = sin(angle);
	for (int i = 0; i < nb; ++i)
	{
		Point pt = points[i] - center;
		points[i] = center + Point(pt.x*ca-pt.y*sa, pt.x*sa+pt.y*ca);
	}
}

Path rotated(const Path& path, const Point& center, float angle)
{
	const auto& points = path.points;
	int nb = points.size();
	std::vector<Point> tmp(nb);
	float ca = cos(angle), sa = sin(angle);
	for (int i = 0; i < nb; ++i)
	{
		Point pt = points[i] - center;
		tmp[i] = center + Point(pt.x*ca-pt.y*sa, pt.x*sa+pt.y*ca);
	}
	return tmp;
}

//****************************************************************************//

float areaOfPolygon(const Path& poly)
{
	const auto& points = poly.points;
	int nbPts = points.size();
	float area = 0;
	for (int i = 0; i < nbPts; ++i)
	{
		Point p1 = points[i], p2 = points[(i+1) % nbPts];
		area += p1.cross(p2);
	}

	return area / 2;
}

types::Point centroidOfPolygon(const Path& poly)
{
	const auto& points = poly.points;
	int nbPts = points.size();
	Point pt;
	for (int i = 0; i < nbPts; ++i)
	{
		Point p1 = points[i], p2 = points[(i+1)%nbPts];
		pt += (p1 + p2) * p1.cross(p2);
	}

	return pt / (6 * areaOfPolygon(poly));
}

bool polygonContainsPoint(const Path &poly, types::Point pt)
{
	const auto& points = poly.points;
	int nb = points.size();
	for(int i1=0, i0=nb-1; i1<nb; i0=i1++)
	{
		const Point &p0 = points[i0], &p1 = points[i1];
		Point n = Point(p1.y - p0.y, p0.x - p1.x);
		Point d = pt - p0;

		if(n.dot(d) > 0)
			return false;
	}
	return true;
}

void reorientPolygon(Path& poly)
{
	auto& points = poly.points;
	std::reverse(points.begin(), points.end());
}

//****************************************************************************//

template<> PANDA_CORE_API std::string DataTrait<Path>::valueTypeName() { return "path"; }
template<> PANDA_CORE_API int DataTrait<Path>::size(const Path& v) { return v.points.size(); }
template<> PANDA_CORE_API void DataTrait<Path>::clear(Path& v, int size, bool init)
{
	if(init)
		v.points.clear();
	v.points.resize(size);
}

template<>
PANDA_CORE_API void DataTrait<Path>::writeValue(XmlElement& elem, const Path& path)
{
	auto pointTrait = DataTraitsList::getTraitOf<Point>();
	for(const auto& pt : path.points)
	{
		auto ptNode = elem.addChild("Point");
		pointTrait->writeValue(ptNode, &pt);
	}
}

template<>
PANDA_CORE_API void DataTrait<Path>::readValue(XmlElement& elem, Path& path)
{
	auto& points = path.points;
	points.clear();
	auto pointTrait = DataTraitsList::getTraitOf<Point>();

	auto ptNode = elem.firstChild("Point");
	while(ptNode)
	{
		Point pt;
		pointTrait->readValue(ptNode, &pt);
		points.push_back(pt);
		ptNode = ptNode.nextSibling("Point");
	}
}

template class PANDA_CORE_API Data< Path >;
template class PANDA_CORE_API Data< std::vector<Path> >;

int pathDataClass = RegisterData< Path >();
int pathVectorDataClass = RegisterData< std::vector<Path> >();

} // namespace types

} // namespace panda

void convertType(const panda::types::Path& from, std::vector<panda::types::Point>& to)
{ to = from.points; }
void convertType(const std::vector<panda::types::Point>& from, panda::types::Path& to)
{ to.points = from; }

panda::types::RegisterTypeConverter<panda::types::Path, std::vector<panda::types::Point> > PathPointsConverter;
panda::types::RegisterTypeConverter<std::vector<panda::types::Point>, panda::types::Path> PointsPathConverter;

