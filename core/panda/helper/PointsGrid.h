#ifndef HELPER_POINTSGRID_H
#define HELPER_POINTSGRID_H

#include <panda/core.h>
#include <panda/types/Rect.h>

#include <vector>

namespace panda
{

namespace helper
{

class PANDA_CORE_API PointsGrid
{
public:
	PointsGrid();
	void initGrid(types::Rect area, PReal cellSize);
	void clear();
	void addPoint(const types::Point& point);
	void addPoints(const std::vector<types::Point>& points);
	int removePoint(const types::Point& point);
	bool hasPoint(const types::Point& point);

	// Return true if there is another point inside the disk a radius "distance" around "point"
	bool testNeighbor(const types::Point& point, PReal distance);

	bool getNearest(const types::Point& point, PReal maxDist, types::Point& result);

protected:
	int cellIndex(const types::Point& point);

	typedef std::vector<types::Point> Cell;

	types::Rect m_area;
	PReal m_cellSize;
	int m_width, m_height;
	std::vector<Cell> m_cells;
};

} // namespace helper

} // namespace panda

#endif // HELPER_POINTSGRID_H
