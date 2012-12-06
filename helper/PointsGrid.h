#ifndef HELPER_POINTSGRID_H
#define HELPER_POINTSGRID_H

#include <QList>
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
	void clear();
	void addPoint(const QPointF& point);
	void addPoints(const QVector<QPointF>& points);
	void removePoint(const QPointF& point);
	bool hasPoint(const QPointF& point);

	// Return true if there is another point inside the disk a radius "distance" around "point"
	bool testNeighbor(const QPointF& point, double distance);

	bool getNearest(const QPointF& point, double maxDist, QPointF& result);

protected:
	int index(const QPointF& point);

	typedef QList<QPointF> Cell;

	QRectF area;
	double cellSize;
	int width, height;
	QVector<Cell> cells;
};

} // namespace helper

} // namespace panda

#endif // HELPER_POINTSGRID_H
