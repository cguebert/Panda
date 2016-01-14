#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Path.h>

#include <set>
#include <map>

namespace
{

struct PointCompare
{
	bool operator()(const panda::types::Point& p1, const panda::types::Point& p2)
	{
		return p1.x < p2.x || (p1.x == p2.x && p1.y < p2.y);
	}
};

}

namespace panda {

using types::Path;
using types::Point;

class ModifierPoints_ConnectSegments : public PandaObject
{
public:
	PANDA_CLASS(ModifierPoints_ConnectSegments, PandaObject)

	ModifierPoints_ConnectSegments(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "List of segments (pair of points)"))
		, m_output(initData("output", "Output path" ))
	{
		addInput(m_input);
		addOutput(m_output);
	}

	std::pair<int, int> make_edge(int a, int b)
	{
		if(a < b)
			return std::make_pair(a, b);
		else
			return std::make_pair(b, a);
	}

	void prepareSearchData(const std::vector<Point>& points)
	{
		m_usedEdges.clear();
		m_uniquePoints.clear();
		m_neighbours.clear();

		// Compute the "points around points" list
		std::map<Point, int, PointCompare> pointsMap;
		for(int i=0, nb=points.size()/2; i<nb; ++i)
		{
			const Point &pt1 = points[i*2], &pt2 = points[i*2+1];
			if(!pointsMap.count(pt1))
			{
				pointsMap[pt1] = m_uniquePoints.size();
				m_uniquePoints.push_back(pt1);
			}
			if(!pointsMap.count(pt2))
			{
				pointsMap[pt2] = m_uniquePoints.size();
				m_uniquePoints.push_back(pt2);
			}

			int i1 = pointsMap[pt1], i2 = pointsMap[pt2];
			m_neighbours[i1].push_back(i2);
			m_neighbours[i2].push_back(i1);
		}
	}

	bool usedEdge(int a, int b)
	{
		return m_usedEdges.find(make_edge(a, b)) != m_usedEdges.end();
	}

	PReal angleModulo(PReal angle) // Make sure angle is [0; 2*pi]
	{
		const PReal pi2 = 2 * (PReal)M_PI;
		while(angle < 0)
			angle += pi2;
		while(angle > pi2)
			angle -= pi2;
		return angle;
	}

	int selectNextPoint(int currentId, int prevId)
	{
		std::vector<int> candidates;
		for(auto ptId : m_neighbours[currentId])
			if(ptId != prevId && !usedEdge(currentId, ptId))
				candidates.push_back(ptId);

		if(candidates.size() == 1)
			return candidates.front();

		Point BA = m_uniquePoints[prevId] - m_uniquePoints[currentId];
		PReal prevAngle = (prevId != currentId ? -atan2(BA.y, BA.x) : M_PI);

		int best = -1;
		PReal bestAngle = 10; // > 3 * pi
		for(auto ptId : candidates)
		{
			// Compute the angle from the current segment to this one
			Point BC = m_uniquePoints[ptId] - m_uniquePoints[currentId];
			PReal angle = -atan2(BC.y, BC.x);
			PReal delta = angleModulo(prevAngle - angle);
			if(delta < bestAngle)
			{
				bestAngle = delta;
				best = ptId;
			}
		}

		return best;
	}

	int findTopLeftPoint(const std::set<int>& ptsId)
	{
		Point best = m_uniquePoints[*ptsId.begin()];
		int bestId = *ptsId.begin();
		for(const auto& ptId : ptsId)
		{
			Point pt = m_uniquePoints[ptId];
			if(pt. y < best.y || (pt.y == best.y && pt.x < best.x))
			{
				best = pt;
				bestId = ptId;
			}
		}

		return bestId;
	}

	void update()
	{
		const auto& input = m_input.getValue();

		auto output = m_output.getAccessor();
		output.clear();

		if(input.empty())
		{
			cleanDirty();
			return;
		}

		prepareSearchData(input);

		std::set<int> unusedIndices;
		for(int i=0, nb=m_uniquePoints.size(); i<nb; ++i)
			unusedIndices.insert(i);

		while(!unusedIndices.empty())
		{
			int start = findTopLeftPoint(unusedIndices);
			unusedIndices.erase(start);
			std::vector<int> resIndices;
			resIndices.push_back(start);

			int prev = start, current = start;
			bool found = true;
			while(found)
			{
				found = false;
				int best = selectNextPoint(current, prev);

				if(best != -1)
				{
					resIndices.push_back(best);
					unusedIndices.erase(best);
					m_usedEdges.insert(make_edge(current, best));
					prev = current;
					current = best;
					found = true;

					// Closed a loop
					if(best == start)
						break;
				}
			}

			if(resIndices.size() > 1)
			{
				Path path;
				for(auto p : resIndices)
					path.push_back(m_uniquePoints[p]);
				output.push_back(path);
			}
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > m_input;
	Data< std::vector<Path> > m_output;

	std::set<std::pair<int, int>> m_usedEdges;
	std::vector<Point> m_uniquePoints;
	std::map<int, std::vector<int> > m_neighbours;
};

int ModifierPoints_ConnectSegmentsClass = RegisterObject<ModifierPoints_ConnectSegments>("Modifier/Point/Connect segments")
		.setDescription("Create a continous line from a list of segments");

} // namespace Panda
