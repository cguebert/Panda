#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Mesh.h>
#include <panda/types/Polygon.h>
#include <panda/helper/algorithm.h>

#include <set>
#include <map>
#include <iostream>

namespace panda {

using types::Mesh;
using types::Path;
using types::Point;
using types::Polygon;

class Polygon_CreateFromMesh : public PandaObject
{
public:
	PANDA_CLASS(Polygon_CreateFromMesh, PandaObject)

	using PointsList = std::vector<Point>;
	using IdSet = std::set<int>;
	using Edge = std::pair<int, int>;
	using EdgeSet = std::set<Edge>;
	using Neighbours =  std::map<int, std::vector<int>>;

	Polygon_CreateFromMesh(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input mesh"))
		, m_output(initData("output", "Output polygon" ))
	{
		addInput(m_input);
		addOutput(m_output);
	}

	Edge make_edge(int a, int b)
	{
		if(a < b)
			return std::make_pair(a, b);
		else
			return std::make_pair(b, a);
	}

	int findTopLeftPoint(const PointsList& points, const IdSet& ptsId)
	{
		Point best = points[*ptsId.begin()];
		int bestId = *ptsId.begin();
		for(const auto& ptId : ptsId)
		{
			Point pt = points[ptId];
			if(pt. y < best.y || (pt.y == best.y && pt.x < best.x))
			{
				best = pt;
				bestId = ptId;
			}
		}

		return bestId;
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

	int selectNextPoint(const PointsList& points, const Neighbours& neighbours, const EdgeSet& usedEdges, int currentId, int prevId)
	{
		std::vector<int> candidates;
		for(auto ptId : helper::valueOrDefault(neighbours, currentId))
			if(ptId != prevId && usedEdges.find(make_edge(currentId, ptId)) == usedEdges.end())
				candidates.push_back(ptId);

		if(candidates.size() == 1)
			return candidates.front();

		Point BA = points[prevId] - points[currentId];
		PReal prevAngle = (prevId != currentId ? -atan2(BA.y, BA.x) : static_cast<PReal>(M_PI));

		int best = -1;
		PReal bestAngle = 10; // > 3 * pi
		for(auto ptId : candidates)
		{
			// Compute the angle from the current segment to this one
			Point BC = points[ptId] - points[currentId];
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

	std::vector<Path> doSimpleSearch(const PointsList& points, const Neighbours& neighbours, IdSet& unusedPts)
	{
		std::vector<Path> paths; // Before we can separate the contour from the holes
		while(!unusedPts.empty())
		{
			int start = *unusedPts.begin();
			unusedPts.erase(start);
			std::vector<int> ptsId;
			ptsId.push_back(start);

			int prev = start, current = start;
			bool found = true;
			while(found)
			{
				found = false;
				for(auto ptId : helper::valueOrDefault(neighbours, current))
				{
					if(ptId != prev && unusedPts.find(ptId) != unusedPts.end())
					{
						ptsId.push_back(ptId);
						unusedPts.erase(ptId);
						prev = current;
						current = ptId;
						found = true;
						break;
					}
				}

				// Can we close the loop ?
				if (!found && helper::contains(helper::valueOrDefault(neighbours, current), start))
					ptsId.push_back(start);
			}

			if(ptsId.size() > 1)
			{
				Path path;
				for(auto p : ptsId)
					path.push_back(points[p]);
				paths.push_back(path);
			}
		}

		return paths;
	}

	std::vector<Path> doComplexSearch(const PointsList& points, const Neighbours& neighbours, IdSet& unusedPts)
	{
		std::vector<Path> paths;
		EdgeSet usedEdges;
		while(!unusedPts.empty())
		{
			int start = findTopLeftPoint(points, unusedPts);
			unusedPts.erase(start);
			std::vector<int> ptsId;
			ptsId.push_back(start);

			int prev = start, current = start;
			bool found = true;
			while(found)
			{
				found = false;
				int best = selectNextPoint(points, neighbours, usedEdges, current, prev);

				if(best != -1)
				{
					ptsId.push_back(best);
					unusedPts.erase(best);
					usedEdges.insert(make_edge(current, best));
					prev = current;
					current = best;
					found = true;

					// Closed a loop
					if(best == start)
						break;
				}
			}

			if(ptsId.size() > 1)
			{
				Path path;
				for(auto p : ptsId)
					path.push_back(points[p]);
				paths.push_back(path);
			}
		}

		return paths;
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

		for(Mesh mesh : input) // Take a non-const copy
		{
			const auto& points = mesh.getPoints();
			std::set<int> unusedPts;
			Neighbours neighbours;
			auto borderEdges = mesh.getEdgesOnBorder(); // This is not const
			for(auto edgeId : borderEdges)
			{
				const auto& edge = mesh.getEdge(edgeId);
				unusedPts.insert(edge[0]);
				unusedPts.insert(edge[1]);
				neighbours[edge[0]].push_back(edge[1]);
				neighbours[edge[1]].push_back(edge[0]);
			}

			bool simpleSearch = true;
			for(const auto& n : neighbours)
			{
				if(n.second.size() > 2)
				{
					simpleSearch = false;
					break;
				}
			}

			std::vector<Path> tempPaths = simpleSearch ?
						doSimpleSearch(points, neighbours, unusedPts) :
						doComplexSearch(points, neighbours, unusedPts);

			Polygon poly;
			if(tempPaths.size() == 1)
			{
				poly.contour = std::move(tempPaths.front());
			}
			else
			{	// The path with the biggest area is the contour
				int contourId = 0;
				PReal maxArea = 0;
				for(int i=0, nb=tempPaths.size(); i<nb; ++i)
				{
					auto area = fabs(types::areaOfPolygon(tempPaths[i]));
					if(area > maxArea)
					{
						contourId = i;
						maxArea = area;
					}
				}

				for(int i=0, nb=tempPaths.size(); i<nb; ++i)
				{
					if(i == contourId)
						poly.contour = std::move(tempPaths[i]);
					else
						poly.holes.push_back(std::move(tempPaths[i]));
				}
			}

			output.push_back(poly);
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Mesh> > m_input;
	Data< std::vector<Polygon> > m_output;
};

int Polygon_CreateFromMeshClass = RegisterObject<Polygon_CreateFromMesh>("Modifier/Mesh/Mesh to polygon")
		.setDescription("Compute the polygon corresponding to the input mesh");

} // namespace Panda
