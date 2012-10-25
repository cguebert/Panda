#ifndef HELPER_POINTSGRID_H
#define HELPER_POINTSGRID_H

#include <QVector>
#include <QPointF>
#include <QRectF>

namespace panda
{

namespace helper
{

class PointsGrid
{
public:
	PointsGrid();
	void initGrid(QRectF area, double cellSize);
	void clear() { points.clear(); }
	void addPoint(const QPointF& point);

	// Return true if there is another point inside the disk a radius "distance" around "point"
	bool testNeighbor(const QPointF& point, double distance);

protected:
	int index(const QPointF& point);

	QRectF area;
	double cellSize;
	int width, height;
	QVector< QVector<QPointF> > points;
};

} // namespace helper

} // namespace panda

#endif // HELPER_POINTSGRID_H
