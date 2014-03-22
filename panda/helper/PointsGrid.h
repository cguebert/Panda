#ifndef HELPER_POINTSGRID_H
#define HELPER_POINTSGRID_H

#include <QList>
#include <QVector>
#include <panda/types/Rect.h>

namespace panda
{

namespace helper
{

class PointsGrid
{
public:
	PointsGrid();
	void initGrid(types::Rect area, double cellSize);
	void clear();
	void addPoint(const types::Point& point);
	void addPoints(const QVector<types::Point>& points);
	int removePoint(const types::Point& point);
	bool hasPoint(const types::Point& point);

	// Return true if there is another point inside the disk a radius "distance" around "point"
	bool testNeighbor(const types::Point& point, double distance);

	bool getNearest(const types::Point& point, double maxDist, types::Point& result);

protected:
	int cellIndex(const types::Point& point);

	typedef QList<types::Point> Cell;

	types::Rect area;
	double cellSize;
	int width, height;
	QVector<Cell> cells;
};

} // namespace helper

} // namespace panda

#endif // HELPER_POINTSGRID_H
