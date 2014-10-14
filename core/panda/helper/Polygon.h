#ifndef HELPER_POLYGON_H
#define HELPER_POLYGON_H

#include <panda/types/Point.h>

#include <QVector>

namespace panda
{

namespace helper
{

typedef QVector<types::Point> Polygon;

PANDA_CORE_API PReal areaOfPolygon(const Polygon& poly);
PANDA_CORE_API types::Point centroidOfPolygon(const Polygon& poly);
PANDA_CORE_API bool polygonContainsPoint(const Polygon &poly, types::Point pt);
PANDA_CORE_API void reorientPolygon(Polygon& poly);

} // namespace helper

} // namespace panda

#endif // HELPER_POLYGON_H
