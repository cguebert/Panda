#ifndef TYPES_POLYGON_H
#define TYPES_POLYGON_H

#include <panda/core.h>
#include <panda/types/Path.h>

namespace panda
{

namespace types
{

class PANDA_CORE_API Polygon
{
public:
	void clear();

	bool operator==(const Polygon& p) const;
	bool operator!=(const Polygon& p) const;

	Path contour;
	QVector<Path> holes;
};

inline bool Polygon::operator==(const Polygon& p) const
{ return contour == p.contour && holes == p.holes; }

inline bool Polygon::operator!=(const Polygon& p) const
{ return contour != p.contour || holes != p.holes; }

PANDA_CORE_API PReal areaOfPolygon(const QVector<types::Point>& poly);
PANDA_CORE_API types::Point centroidOfPolygon(const QVector<types::Point>& poly);
PANDA_CORE_API bool polygonContainsPoint(const QVector<types::Point> &poly, types::Point pt);
PANDA_CORE_API void reorientPolygon(QVector<types::Point>& poly);

#ifndef PANDA_BUILD_CORE
extern template class PANDA_CORE_API Data< Polygon >;
extern template class PANDA_CORE_API Data< QVector<Polygon> >;
#endif

} // namespace types

} // namespace panda

#endif // TYPES_POLYGON_H
