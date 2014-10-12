#include <panda/helper/Polygon.h>

namespace panda
{

namespace helper
{

using types::Point;

PReal areaOfPolygon(const Polygon& poly)
{
	int nbPts = poly.size();
	PReal area = 0;
	for(int i=0; i<nbPts; ++i)
	{
		Point p1 = poly[i], p2 = poly[(i+1)%nbPts];
		area += p1.cross(p2);
	}

	return area / 2;
}

types::Point centroidOfPolygon(const Polygon& poly)
{
	int nbPts = poly.size();
	Point pt;
	for(int i=0; i<nbPts; ++i)
	{
		Point p1 = poly[i], p2 = poly[(i+1)%nbPts];
		pt += (p1 + p2) * p1.cross(p2);
	}

	return pt / (6 * areaOfPolygon(poly));
}

bool polygonContainsPoint(const Polygon &poly, types::Point pt)
{
	int nb = poly.size();
	for(int i1=0, i0=nb-1; i1<nb; i0=i1++)
	{
		const Point &p0 = poly[i0], &p1 = poly[i1];
		Point n = Point(p1.y - p0.y, p0.x - p1.x);
		Point d = pt - p0;

		if(n.dot(d) > 0)
			return false;
	}
	return true;
}

void reorientPolygon(Polygon& poly)
{
	std::reverse(poly.begin(), poly.end());
}

} // namespace helper

} // namespace panda
