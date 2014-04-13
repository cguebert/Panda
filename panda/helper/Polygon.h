#ifndef HELPER_POLYGON_H
#define HELPER_POLYGON_H

#include <QVector>
#include <panda/types/Point.h>

namespace panda
{

namespace helper
{

typedef QVector<types::Point> Polygon;

PReal areaOfPolygon(const Polygon& poly);
types::Point centroidOfPolygon(const Polygon& poly);
bool polygonContainsPoint(const Polygon &poly, types::Point pt);
void reorientPolygon(Polygon& poly);

} // namespace helper

} // namespace panda

#endif // HELPER_POLYGON_H
