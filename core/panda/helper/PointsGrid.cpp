#include <panda/helper/PointsGrid.h>
#include <panda/helper/algorithm.h>

#include <cmath>

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

void PointsGrid::initGrid(Rect newArea, float newCellSize)
{
	m_cells.clear();
	m_area = newArea;
	m_cellSize = newCellSize;

	m_width = static_cast<int>(ceil(m_area.width() / m_cellSize));
	m_height = static_cast<int>(ceil(m_area.height() / m_cellSize));

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

void PointsGrid::addPoints(const std::vector<Point>& points)
{
	for(const auto& pt : points)
		addPoint(pt);
}

int PointsGrid::removePoint(const Point& point)
{
	return helper::removeAll(m_cells[cellIndex(point)], point);
}

bool PointsGrid::hasPoint(const Point& point)
{
	return helper::contains(m_cells[cellIndex(point)], point);
}

bool PointsGrid::testNeighbor(const Point& point, float distance)
{
	int minX = static_cast<int>(floor(std::max<float>(0, (point.x - distance - m_area.left()) / m_cellSize)));
	int maxX = static_cast<int>(ceil(std::min<float>(m_width - 1.f, (point.x + distance - m_area.left()) / m_cellSize)));
	int minY = static_cast<int>(floor(std::max<float>(0, (point.y - distance - m_area.top()) / m_cellSize)));
	int maxY = static_cast<int>(ceil(std::min<float>(m_height - 1.f, (point.y + distance - m_area.top()) / m_cellSize)));

	float dist2 = distance*distance;
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

bool PointsGrid::getNearest(const Point& point, float maxDist, Point& result)
{
	int minX = static_cast<int>(floor(std::max<float>(0, (point.x - maxDist - m_area.left()) / m_cellSize)));
	int maxX = static_cast<int>(ceil(std::min<float>(m_width - 1.f, (point.x + maxDist - m_area.left()) / m_cellSize)));
	int minY = static_cast<int>(floor(std::max<float>(0, (point.y - maxDist - m_area.top()) / m_cellSize)));
	int maxY = static_cast<int>(ceil(std::min<float>(m_height - 1.f, (point.y + maxDist - m_area.top()) / m_cellSize)));

	float minDist = maxDist*maxDist;
	bool found = false;
	for(int y=minY; y<=maxY; y++)
	{
		for(int x=minX; x<=maxX; x++)
		{
			const Cell& cell = m_cells[y * m_width + x];
			for(const auto& pt : cell)
			{
				float d = (pt - point).norm2();
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
	float x = helper::bound(m_area.left(), point.x, m_area.right());
	float y = helper::bound(m_area.top(), point.y, m_area.bottom());

	int gx = static_cast<int>(floor((x - m_area.left()) / m_cellSize));
	int gy = static_cast<int>(floor((y - m_area.top()) / m_cellSize));

	gx = helper::bound(0, gx, m_width - 1);
	gy = helper::bound(0, gy, m_height - 1);
	return gy * m_width + gx;
}

} // namespace helper

} // namespace panda
