#include "PointsGrid.h"
#include <math.h>
#include <qglobal.h>

namespace panda
{

namespace helper
{

PointsGrid::PointsGrid()
	: cellSize(10)
	, width(10)
	, height(10)
{
}

void PointsGrid::initGrid(QRectF newArea, double newCellSize)
{
	points.clear();
	area = newArea;
	cellSize = newCellSize;

	width = ceil(area.width() / cellSize);
	height = ceil(area.height() / cellSize);

	points.resize(width * height);
}

void PointsGrid::addPoint(const QPointF& point)
{
	points[index(point)].append(point);
}

double distance2(const QPointF& ptA, const QPointF& ptB)
{
	double dx = ptA.x()-ptB.x();
	double dy = ptA.y()-ptB.y();
	return dx*dx+dy*dy;
}

bool PointsGrid::testNeighbor(const QPointF& point, double distance)
{
	int minX = floor(qMax(0.0, (point.x() - distance - area.left()) / cellSize));
	int maxX = ceil(qMin(width - 1.0, (point.x() + distance - area.left()) / cellSize));
	int minY = floor(qMax(0.0, (point.y() - distance - area.top()) / cellSize));
	int maxY = ceil(qMin(height - 1.0, (point.y() + distance - area.top()) / cellSize));

	double dist2 = distance*distance;
	for(int y=minY; y<=maxY; y++)
	{
		for(int x=minX; x<=maxX; x++)
		{
			QVector<QPointF>& cell = points[y * width + x];
			QVector<QPointF>::iterator iter;
			for(iter=cell.begin(); iter!=cell.end(); ++iter)
				if(distance2(*iter, point) < dist2)
					return true;
		}
	}

	return false;
}

int PointsGrid::index(const QPointF& point)
{
	double x = qBound(area.left(), point.x(), area.right());
	double y = qBound(area.top(), point.y(), area.bottom());

	int gx = floor((x - area.left()) / cellSize);
	int gy = floor((y - area.top()) / cellSize);
	return gy * width + gx;
}

} // namespace helper

} // namespace panda
