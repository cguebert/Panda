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
	: m_cellSize(10)
	, m_width(10)
	, m_height(10)
{
}

void PointsGrid::initGrid(Rect newArea, PReal newCellSize)
{
	m_cells.clear();
	m_area = newArea;
	m_cellSize = newCellSize;

	m_width = ceil(m_area.width() / m_cellSize);
	m_height = ceil(m_area.height() / m_cellSize);

	m_cells.resize(m_width * m_height);
}

void PointsGrid::clear()
{
	for (auto& c : m_cells)
		c.clear();
}

void PointsGrid::addPoint(const Point& point)
{
	m_cells[cellIndex(point)].push_back(point);
}

void PointsGrid::addPoints(const QVector<Point>& points)
{
	for(const auto& pt : points)
		addPoint(pt);
}

int PointsGrid::removePoint(const Point& point)
{
	return m_cells[cellIndex(point)].removeAll(point);
}

bool PointsGrid::hasPoint(const Point& point)
{
	return m_cells[cellIndex(point)].contains(point);
}

bool PointsGrid::testNeighbor(const Point& point, PReal distance)
{
	int minX = floor(qMax<PReal>(0.0, (point.x - distance - m_area.left()) / m_cellSize));
	int maxX = ceil(qMin<PReal>(m_width - 1.0, (point.x + distance - m_area.left()) / m_cellSize));
	int minY = floor(qMax<PReal>(0.0, (point.y - distance - m_area.top()) / m_cellSize));
	int maxY = ceil(qMin<PReal>(m_height - 1.0, (point.y + distance - m_area.top()) / m_cellSize));

	PReal dist2 = distance*distance;
	for(int y=minY; y<=maxY; y++)
	{
		for(int x=minX; x<=maxX; x++)
		{
			const Cell& cell = m_cells[y * m_width + x];
			for(const auto& pt : cell)
				if((pt - point).norm2() < dist2)
					return true;
		}
	}

	return false;
}

bool PointsGrid::getNearest(const Point& point, PReal maxDist, Point& result)
{
	int minX = floor(qMax<PReal>(0.0, (point.x - maxDist - m_area.left()) / m_cellSize));
	int maxX = ceil(qMin<PReal>(m_width - 1.0, (point.x + maxDist - m_area.left()) / m_cellSize));
	int minY = floor(qMax<PReal>(0.0, (point.y - maxDist - m_area.top()) / m_cellSize));
	int maxY = ceil(qMin<PReal>(m_height - 1.0, (point.y + maxDist - m_area.top()) / m_cellSize));

	PReal minDist = maxDist*maxDist;
	bool found = false;
	for(int y=minY; y<=maxY; y++)
	{
		for(int x=minX; x<=maxX; x++)
		{
			const Cell& cell = m_cells[y * m_width + x];
			for(const auto& pt : cell)
			{
				PReal d = (pt - point).norm2();
				if(d < minDist)
				{
					result = pt;
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
	PReal x = qBound(m_area.left(), point.x, m_area.right());
	PReal y = qBound(m_area.top(), point.y, m_area.bottom());

	int gx = floor((x - m_area.left()) / m_cellSize);
	int gy = floor((y - m_area.top()) / m_cellSize);

	gx = qBound(0, gx, m_width - 1);
	gy = qBound(0, gy, m_height - 1);
	return gy * m_width + gx;
}

} // namespace helper

} // namespace panda
