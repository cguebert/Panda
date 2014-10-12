#include <panda/helper/PointsGrid.h>
#include <cmath>
#include <qglobal.h>

namespace panda
{

namespace helper
{

using types::Point;
using types::Rect;

PointsGrid::PointsGrid()
	: cellSize(10)
	, width(10)
	, height(10)
{
}

void PointsGrid::initGrid(Rect newArea, PReal newCellSize)
{
	cells.clear();
	area = newArea;
	cellSize = newCellSize;

	width = ceil(area.width() / cellSize);
	height = ceil(area.height() / cellSize);

	cells.resize(width * height);
}

void PointsGrid::clear()
{
	int nb = cells.size();
	for(int i=0; i<nb; ++i)
		cells[i].clear();
}

void PointsGrid::addPoint(const Point& point)
{
	cells[cellIndex(point)].push_back(point);
}

void PointsGrid::addPoints(const QVector<Point>& points)
{
	int nb = points.size();
	for(int i=0; i<nb; ++i)
		addPoint(points[i]);
}

int PointsGrid::removePoint(const Point& point)
{
	return cells[cellIndex(point)].removeAll(point);
}

bool PointsGrid::hasPoint(const Point& point)
{
	return cells[cellIndex(point)].contains(point);
}

bool PointsGrid::testNeighbor(const Point& point, PReal distance)
{
	int minX = floor(qMax<PReal>(0.0, (point.x - distance - area.left()) / cellSize));
	int maxX = ceil(qMin<PReal>(width - 1.0, (point.x + distance - area.left()) / cellSize));
	int minY = floor(qMax<PReal>(0.0, (point.y - distance - area.top()) / cellSize));
	int maxY = ceil(qMin<PReal>(height - 1.0, (point.y + distance - area.top()) / cellSize));

	PReal dist2 = distance*distance;
	for(int y=minY; y<=maxY; y++)
	{
		for(int x=minX; x<=maxX; x++)
		{
			const Cell& cell = cells[y * width + x];
			for(Cell::const_iterator iter=cell.begin(); iter!=cell.end(); ++iter)
				if((*iter - point).norm2() < dist2)
					return true;
		}
	}

	return false;
}

bool PointsGrid::getNearest(const Point& point, PReal maxDist, Point& result)
{
	int minX = floor(qMax<PReal>(0.0, (point.x - maxDist - area.left()) / cellSize));
	int maxX = ceil(qMin<PReal>(width - 1.0, (point.x + maxDist - area.left()) / cellSize));
	int minY = floor(qMax<PReal>(0.0, (point.y - maxDist - area.top()) / cellSize));
	int maxY = ceil(qMin<PReal>(height - 1.0, (point.y + maxDist - area.top()) / cellSize));

	PReal minDist = maxDist*maxDist;
	bool found = false;
	for(int y=minY; y<=maxY; y++)
	{
		for(int x=minX; x<=maxX; x++)
		{
			const Cell& cell = cells[y * width + x];
			for(Cell::const_iterator iter=cell.begin(); iter!=cell.end(); ++iter)
			{
				PReal d = (*iter - point).norm2();
				if(d < minDist)
				{
					result = *iter;
					minDist = d;
					found = true;
				}
			}
		}
	}

	return found;
}

int PointsGrid::cellIndex(const Point& point)
{
	PReal x = qBound(area.left(), point.x, area.right());
	PReal y = qBound(area.top(), point.y, area.bottom());

	int gx = floor((x - area.left()) / cellSize);
	int gy = floor((y - area.top()) / cellSize);
	return gy * width + gx;
}

} // namespace helper

} // namespace panda
