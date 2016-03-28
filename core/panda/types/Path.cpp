#include <panda/types/Point.h>
#include <panda/types/Path.h>

#include <panda/data/DataFactory.h>

namespace 
{
	
using panda::types::Point;

// Copyright 2000 softSurfer, 2012 Dan Sunday
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

// http://geomalgorithms.com/a03-_inclusion.html

// isLeft(): tests if a point is Left|On|Right of an infinite line.
//    Input:  three points P0, P1, and P2
//    Return: >0 for P2 left of the line through P0 and P1
//            =0 for P2  on the line
//            <0 for P2  right of the line
//    See: Algorithm 1 "Area of Triangles and Polygons"
inline float isLeft( Point P0, Point P1, Point P2 )
{
	return ( (P1.x - P0.x) * (P2.y - P0.y)
			- (P2.x -  P0.x) * (P1.y - P0.y) );
}

// wn_PnPoly(): winding number test for a point in a polygon
//      Input:   P = a point,
//               V[] = vertex points of a polygon V[n+1] with V[n]=V[0]
//      Return:  wn = the winding number (=0 only when P is outside)
int wn_PnPoly( Point P, const Point* V, int n )
{
	int    wn = 0;    // the  winding number counter

	// loop through all edges of the polygon
	for (int i=0; i<n; i++) {   // edge from V[i] to  V[i+1]
		if (V[i].y <= P.y) {          // start y <= P.y
			if (V[i+1].y  > P.y)      // an upward crossing
				 if (isLeft( V[i], V[i+1], P) > 0)  // P left of  edge
					 ++wn;            // have  a valid up intersect
		}
		else {                        // start y > P.y (no test needed)
			if (V[i+1].y  <= P.y)     // a downward crossing
				 if (isLeft( V[i], V[i+1], P) < 0)  // P right of  edge
					 --wn;            // have  a valid down intersect
		}
	}
	return wn;
}

}

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
	return wn_PnPoly(pt, poly.points.data(), poly.points.size() - 1) != 0;
}

void reorientPolygon(Path& poly)
{
	auto& points = poly.points;
	std::reverse(points.begin(), points.end());
}

//****************************************************************************//

template<> PANDA_CORE_API std::string DataTrait<Path>::valueTypeName() { return "path"; }

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

