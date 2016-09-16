#include <panda/object/ObjectFactory.h>
#include <panda/types/IntVector.h>
#include <panda/types/Path.h>

#include <algorithm>
#include <tuple>

namespace
{

	struct SimplifiedPoint
	{
		SimplifiedPoint() = default;
		SimplifiedPoint(const panda::types::Point& pt)
			: intPoint{static_cast<int>(round(pt.x)), static_cast<int>(round(pt.y))}
			, originalPoint(pt)
		{ }

		std::tuple<int, int> intPoint;
		panda::types::Point originalPoint;
	};

	bool simplifiedCompare(const SimplifiedPoint& lhs, const SimplifiedPoint& rhs)
	{ return lhs.intPoint == rhs.intPoint; }

	bool simplifiedLesser(const SimplifiedPoint& lhs, const SimplifiedPoint& rhs)
	{ return lhs.intPoint < rhs.intPoint; }
}

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

		if (points.empty())
			return;

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

		std::vector<SimplifiedPoint> simplifiedPoints;
		// Get all points

		for (const auto& path : paths)
		{
			for (const auto& pt : path.points)
				simplifiedPoints.emplace_back(pt);
		}

		// Sort the points and remove duplicates
		std::sort(simplifiedPoints.begin(), simplifiedPoints.end(), simplifiedLesser);
		auto last = std::unique(simplifiedPoints.begin(), simplifiedPoints.end(), simplifiedCompare);
		simplifiedPoints.erase(last, simplifiedPoints.end());

		// Copy to the outputs points
		for (const auto& pt : simplifiedPoints)
			points.push_back(pt.originalPoint);

		const auto ptsBegin = simplifiedPoints.begin(), ptsEnd = simplifiedPoints.end();
		// For each point in a polygon, find the corresponding index by doing a binary search
		for (const auto& path : paths)
		{
			IntVector list;
			for (const auto& pt : path.points)
			{
				auto it = std::lower_bound(ptsBegin, ptsEnd, SimplifiedPoint(pt), simplifiedLesser);
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

//****************************************************************************//

class Polygon_IndicesCloseLoop : public PandaObject
{
public:
	PANDA_CLASS(Polygon_IndicesCloseLoop, PandaObject)

	Polygon_IndicesCloseLoop(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input indices"))
		, m_output(initData("output", "Output indices"))
	{
		addInput(m_input);
		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto outputAcc = m_output.getAccessor();
		auto& output = outputAcc.wref();

		output = input;

		if (input.empty())
			return;

		for (auto& list : output)
		{
			auto& values = list.values;
			if (values.size() > 1 && values.front() != values.back())
				values.push_back(values.front());
		}
	}

protected:
	Data< std::vector<IntVector> > m_input, m_output;
};

int Polygon_IndicesCloseLoopClass = RegisterObject<Polygon_IndicesCloseLoop>("Generator/Polygon/Close loop")
	.setDescription("Ensure the last index of a list is the same as the first");

//****************************************************************************//

class Polygon_IndicesOpenLoop : public PandaObject
{
public:
	PANDA_CLASS(Polygon_IndicesOpenLoop, PandaObject)

	Polygon_IndicesOpenLoop(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input indices"))
		, m_output(initData("output", "Output indices"))
	{
		addInput(m_input);
		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto outputAcc = m_output.getAccessor();
		auto& output = outputAcc.wref();

		output = input;

		if (input.empty())
			return;

		for (auto& list : output)
		{
			auto& values = list.values;
			if (values.size() > 1 && values.front() == values.back())
				values.pop_back();
		}
	}

protected:
	Data< std::vector<IntVector> > m_input, m_output;
};

int Polygon_IndicesOpenLoopClass = RegisterObject<Polygon_IndicesOpenLoop>("Generator/Polygon/Open loop")
	.setDescription("Remove the last index of a list if it is the same as the first");

} // namespace Panda


