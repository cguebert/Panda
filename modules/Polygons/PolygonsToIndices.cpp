#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/IntVector.h>
#include <panda/types/Path.h>

#include <algorithm>

namespace panda {

using types::IntVector;
using types::Path;
using types::Point;

class Polygon_CreateFromIndices : public PandaObject
{
public:
	PANDA_CLASS(Polygon_CreateFromIndices, PandaObject)

	Polygon_CreateFromIndices(PandaDocument *doc)
		: PandaObject(doc)
		, m_points(initData("points", "Input points"))
		, m_indices(initData("indices", "Input indices"))
		, m_output(initData("output", "Output paths"))
	{
		addInput(m_points);
		addInput(m_indices);
		addOutput(m_output);
	}

	void update()
	{
		const auto& points = m_points.getValue();
		const auto& indices = m_indices.getValue();
		auto outputAcc = m_output.getAccessor();
		auto& output = outputAcc.wref();

		output.clear();
		for (const auto& list : indices)
		{
			Path path;
			for (const auto& id : list.values)
				path.points.push_back(points[id]);
			output.push_back(path);
		}
	}

protected:
	Data< std::vector<Point> > m_points;
	Data< std::vector<IntVector> > m_indices;
	Data< std::vector<Path> > m_output;
};

int Polygon_CreateFromIndicesClass = RegisterObject<Polygon_CreateFromIndices>("Generator/Polygon/Polygons from indices")
	.setDescription("Create a polygon from points and a list of indices");

//****************************************************************************//

class Polygon_ToIndices : public PandaObject
{
public:
	PANDA_CLASS(Polygon_ToIndices, PandaObject)

		Polygon_ToIndices(PandaDocument *doc)
		: PandaObject(doc)
		, m_points(initData("points", "Output points"))
		, m_indices(initData("indices", "Output indices"))
		, m_input(initData("input", "Input paths"))
	{
		addInput(m_input);
		addOutput(m_points);
		addOutput(m_indices);
	}

	void update()
	{
		const auto& paths = m_input.getValue();
		auto pointsAcc = m_points.getAccessor();
		auto indicesAcc = m_indices.getAccessor();
		auto& points = pointsAcc.wref();
		auto& indices = indicesAcc.wref();

		points.clear();
		indices.clear();

		// Get all points
		for (const auto& path : paths)
		{
			for (const auto& pt : path.points)
				points.push_back(pt);
		}

		// Sort the points and remove duplicates
		std::sort(points.begin(), points.end());
		auto last = std::unique(points.begin(), points.end());
		points.erase(last, points.end());

		auto ptsBegin = points.begin(), ptsEnd = points.end();
		// For each point in a polygon, find the corresponding index by doing a binary search
		for (const auto& path : paths)
		{
			IntVector list;
			for (const auto& pt : path.points)
			{
				auto it = std::lower_bound(ptsBegin, ptsEnd, pt);
				list.values.push_back(std::distance(ptsBegin, it));
			}
			indices.push_back(list);
		}
	}

protected:
	Data< std::vector<Path> > m_input;
	Data< std::vector<Point> > m_points;
	Data< std::vector<IntVector> > m_indices;
};

int Polygon_ToIndicesClass = RegisterObject<Polygon_ToIndices>("Generator/Polygon/Polygons to indices")
	.setDescription("Convert a polygon to a list of points and a list of indices");

} // namespace Panda


