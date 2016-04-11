#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/IntVector.h>
#include <panda/types/Path.h>

#include <algorithm>
#include <array>
#include <set>

namespace
{

	using Edge = std::array<int, 2>;
	std::vector<Edge> computeEdges(const std::vector<panda::types::IntVector>& polygons)
	{
		std::vector<Edge> edges;

		std::set<Edge> edgeSet;
		for(const auto& poly : polygons)
		{
			const auto& indices = poly.values;
			int nb = indices.size();
			for (int i = 0; i < nb; ++i)
			{
				const int p1 = indices[i];
				const int p2 = indices[(i+1) % nb];

				if (p1 == p2)
					continue;

				const Edge e = ((p1 < p2) ? Edge{p1, p2} : Edge{p2, p1});

				if (!edgeSet.count(e))
				{
					edges.push_back(e);
					edgeSet.insert(e);
				}
			}
		}

		return edges;
	}

	int getMaxValue(const std::vector<panda::types::IntVector>& indices)
	{
		int maxId = -1;
		for (const auto& list : indices)
		{
			for (const auto& id : list.values)
			{
				if (id > maxId)
					maxId = id;
			}
		}

		return maxId;
	}

}

namespace panda {

using types::IntVector;
using types::Path;
using types::Point;

class Polygon_CreateEdgesList : public PandaObject
{
public:
	PANDA_CLASS(Polygon_CreateEdgesList, PandaObject)

	Polygon_CreateEdgesList(PandaDocument *doc)
		: PandaObject(doc)
		, m_indices(initData("indices", "Indices of points forming the polygons"))
		, m_output(initData("ouput", "Indices of points in each edge"))
	{
		addInput(m_indices);
		addOutput(m_output);
	}

	void update()
	{
		const auto& indices = m_indices.getValue();
		auto acc = m_output.getAccessor();
		auto& output = acc.wref();
		output.clear();

		auto edges = computeEdges(indices);
		for (const auto& edge : edges)
		{
			IntVector vec;
			for (auto id : edge)
				vec.values.push_back(id);
			output.push_back(vec);
		}
	}

protected:
	Data< std::vector<IntVector> > m_indices, m_output;	
};

int Polygon_CreateEdgesListClass = RegisterObject<Polygon_CreateEdgesList>("Math/Polygon/Topology/Create edges")
	.setDescription("Compute the indices of points in each edge");

//****************************************************************************//

class Polygon_FacesAroundPoints : public PandaObject
{
public:
	PANDA_CLASS(Polygon_FacesAroundPoints, PandaObject)

	Polygon_FacesAroundPoints(PandaDocument *doc)
		: PandaObject(doc)
		, m_indices(initData("indices", "Lists of indices"))
		, m_output(initData("ouput", "Indices of the lists containing each value"))
	{
		addInput(m_indices);
		addOutput(m_output);
	}

	void update()
	{
		const auto& indices = m_indices.getValue();
		auto acc = m_output.getAccessor();
		auto& output = acc.wref();
		output.clear();

		// First pass, get the number of points
		int maxId = getMaxValue(indices);
		if (maxId < 0)
			return;

		// Second pass, add the index of each polygon to every of its points
		output.resize(maxId + 1);
		int nbPoly = indices.size();
		for (int i = 0; i < nbPoly; ++i)
		{
			const auto& poly = indices[i].values;
			int nb = poly.size();
			for (int j = 0; j < nb - 1; ++j)
				output[poly[j]].values.push_back(i);

			// The last point is often a copy of the first one
			if (nb && poly.back() != poly.front())
				output[poly.back()].values.push_back(i);
		}
	}

protected:
	Data< std::vector<IntVector> > m_indices, m_output;	
};

int Polygon_FacesAroundPointsClass = RegisterObject<Polygon_FacesAroundPoints>("Math/Polygon/Topology/Lists around index")
	.setDescription("Compute the indices of the lists containing each index");

//****************************************************************************//

class Polygon_PointsAroundPoints : public PandaObject
{
public:
	PANDA_CLASS(Polygon_PointsAroundPoints, PandaObject)

	Polygon_PointsAroundPoints(PandaDocument *doc)
		: PandaObject(doc)
		, m_indices(initData("indices", "Indices of points forming the polygons"))
		, m_output(initData("output", "Indices of points around each point"))
	{
		addInput(m_indices);
		addOutput(m_output);
	}

	void update()
	{
		const auto& indices = m_indices.getValue();
		auto acc = m_output.getAccessor();
		auto& output = acc.wref();
		output.clear();

		// First pass, get the number of points
		int maxId = getMaxValue(indices);
		if (maxId < 0)
			return;

		// Compute the list of edges
		auto edges = computeEdges(indices);

		// Second pass, add the index of each edge to every of its points
		output.resize(maxId + 1);
		for (const auto& edge : edges)
		{
			output[edge[0]].values.push_back(edge[1]);
			output[edge[1]].values.push_back(edge[0]);
		}
	}

protected:
	Data< std::vector<IntVector> > m_indices, m_output;	
};

int Polygon_PointsAroundPointsClass = RegisterObject<Polygon_PointsAroundPoints>("Math/Polygon/Topology/Points around points")
	.setDescription("Compute the list of points around each point in polygons");

//****************************************************************************//

class Polygon_EdgesInPolygons : public PandaObject
{
public:
	PANDA_CLASS(Polygon_EdgesInPolygons, PandaObject)

	Polygon_EdgesInPolygons(PandaDocument *doc)
		: PandaObject(doc)
		, m_polygons(initData("polygons", "Indices of points forming the polygons"))
		, m_edges(initData("edges", "Indices of points forming the edges"))
		, m_output(initData("output", "Indices of edges in each polygon"))
	{
		addInput(m_polygons);
		addInput(m_edges);
		addOutput(m_output);
	}

	void update()
	{
		const auto& polygons = m_polygons.getValue();
		const auto& edges = m_edges.getValue();
		auto acc = m_output.getAccessor();
		auto& output = acc.wref();
		output.clear();

		// First pass, get the number of points
		int maxId = getMaxValue(edges);
		if (maxId < 0)
			return;

		// Compute the list of edges around each point
		std::vector<IntVector> edgesAroundPoints;
		edgesAroundPoints.resize(maxId + 1);
		int nbEdges = edges.size();
		for (int i = 0; i < nbEdges; ++i)
		{
			const auto& edge = edges[i];
			const auto& pts = edge.values;
			if (pts.size() == 2)
			{
				edgesAroundPoints[pts[0]].values.push_back(i);
				edgesAroundPoints[pts[1]].values.push_back(i);
			}
		}

		// Second pass, find the edges in each polygon
		for (const auto& poly : polygons)
		{
			IntVector vec;
			const auto& pts = poly.values;
			int nbPts = pts.size();
			for (int i = 0; i < nbPts; ++i)
			{
				int a = pts[i], b = pts[(i + 1) % nbPts];
				if (a == b)
					continue;
				for (auto edgeId : edgesAroundPoints[a].values)
				{
					const auto& edgePts = edges[edgeId].values;
					if ((edgePts[0] == a && edgePts[1] == b) || (edgePts[0] == b && edgePts[1] == a))
					{
						vec.values.push_back(edgeId);
						break;
					}
				}
			}
			output.push_back(vec);
		}
	}

protected:
	Data< std::vector<IntVector> > m_polygons, m_edges, m_output;	
};

int Polygon_EdgesInPolygonsClass = RegisterObject<Polygon_EdgesInPolygons>("Math/Polygon/Topology/Edges in polygons")
	.setDescription("Compute the indices of edges in each polygon");

//****************************************************************************//

} // namespace Panda


