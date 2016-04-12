#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/IntVector.h>
#include <panda/types/Path.h>

#include <algorithm>
#include <array>
#include <deque>
#include <set>

namespace
{

	using IntVectorList = std::vector<panda::types::IntVector>;
	using Edge = std::array<int, 2>;
	using EdgeList = std::vector<Edge>;

	EdgeList computeEdges(const IntVectorList& polygons)
	{
		EdgeList edges;

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

	IntVectorList toIntVectorList(const EdgeList& edges)
	{
		IntVectorList output;
		for (const auto& edge : edges)
		{
			panda::types::IntVector vec;
			for (auto id : edge)
				vec.values.push_back(id);
			output.push_back(vec);
		}

		return output;
	}

	int getMaxValue(const IntVectorList& indices)
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

	IntVectorList computeListsAroundIndices(const IntVectorList& indices)
	{
		IntVectorList result;

		// First pass, get the maximum index value
		int maxId = getMaxValue(indices);
		if (maxId < 0)
			return result;

		// Second pass, add the index of each list to every of its indices
		result.resize(maxId + 1);
		int nbLists = indices.size();
		for (int i = 0; i < nbLists; ++i)
		{
			const auto& list = indices[i].values;
			int nb = list.size();
			for (int j = 0; j < nb - 1; ++j)
				result[list[j]].values.push_back(i);

			// The last point is often a copy of the first one
			if (nb && list.back() != list.front())
				result[list.back()].values.push_back(i);
		}

		return result;
	}

	IntVectorList computeEdgesInPolygons(const IntVectorList& polygons, const IntVectorList& edges)
	{
		IntVectorList output;

		// First pass, get the number of points
		int maxId = getMaxValue(edges);
		if (maxId < 0)
			return output;

		// Compute the list of edges around each point
		IntVectorList edgesAroundPoints;
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
			panda::types::IntVector vec;
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

		return output;
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
		m_output.setValue(computeListsAroundIndices(m_indices.getValue()));
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
		m_output.setValue(computeEdgesInPolygons(polygons, edges));
	}

protected:
	Data< std::vector<IntVector> > m_polygons, m_edges, m_output;	
};

int Polygon_EdgesInPolygonsClass = RegisterObject<Polygon_EdgesInPolygons>("Math/Polygon/Topology/Edges in polygons")
	.setDescription("Compute the indices of edges in each polygon");

//****************************************************************************//

class Polygon_FacesAroundFaces : public PandaObject
{
public:
	PANDA_CLASS(Polygon_FacesAroundFaces, PandaObject)

	Polygon_FacesAroundFaces(PandaDocument *doc)
		: PandaObject(doc)
		, m_polygons(initData("polygons", "Indices of points forming the polygons"))
		, m_output(initData("output", "Indices of the polygons around each polygon"))
		, m_shareEdge(initData(0, "share edge", "If true, two polygons must share an edge, else sharing a point is enough"))
	{
		addInput(m_polygons);
		addInput(m_shareEdge);
		addOutput(m_output);

		m_shareEdge.setWidget("checkbox");
	}

	void update()
	{
		const auto& polygons = m_polygons.getValue();
		auto acc = m_output.getAccessor();
		auto& output = acc.wref();
		output.clear();

		if (polygons.empty())
			return;
		int nbPoly = polygons.size();

		if (m_shareEdge.getValue() != 0)
		{
			auto edges = computeEdges(polygons);
			auto edgesInPolygons = computeEdgesInPolygons(polygons, toIntVectorList(edges));
			auto polysAroundEdges = computeListsAroundIndices(edgesInPolygons);

			for (int i = 0; i < nbPoly; ++i)
			{
				std::set<int> ids;
				for (auto edge : edgesInPolygons[i].values)
				{
					for (auto id : polysAroundEdges[edge].values)
					{
						if (id != i)
							ids.insert(id);
					}
				}

				std::vector<int> vec(ids.begin(), ids.end());
				output.emplace_back(vec);
			}
		}
		else
		{
			auto polysAroundPt = computeListsAroundIndices(polygons);
			for (int i = 0; i < nbPoly; ++i)
			{
				std::set<int> ids;
				for (auto pt : polygons[i].values)
				{
					for (auto id : polysAroundPt[pt].values)
					{
						if (id != i)
							ids.insert(id);
					}
				}

				std::vector<int> vec(ids.begin(), ids.end());
				output.emplace_back(vec);
			}
		}
	}

protected:
	Data< std::vector<IntVector> > m_polygons, m_output;
	Data< int > m_shareEdge;
};

int Polygon_FacesAroundFacesClass = RegisterObject<Polygon_FacesAroundFaces>("Math/Polygon/Topology/Faces around faces")
	.setDescription("Compute the indices of the polygons around each polygon");

//****************************************************************************//

class Polygon_Propagate : public PandaObject
{
public:
	PANDA_CLASS(Polygon_Propagate, PandaObject)

	Polygon_Propagate(PandaDocument *doc)
		: PandaObject(doc)
		, m_neighbors(initData("neighbors", "Indices of neighbors for each value"))
		, m_init(initData("init", "Initial values (we propagate from non zero values)"))
		, m_canPropagate(initData("can propagate", "For each value, non zero if we can propagate here"))
		, m_output(initData("output", "Values after propagation"))
	{
		addInput(m_neighbors);
		addInput(m_init);
		addInput(m_canPropagate);
		addOutput(m_output);
	}

	void update()
	{
		const auto& neighbors = m_neighbors.getValue();
		const auto& initValues = m_init.getValue();
		const auto& canPropagateVal = m_canPropagate.getValue();
		auto acc = m_output.getAccessor();
		auto& output = acc.wref();
		output.clear();

		int maxValue = getMaxValue(neighbors);
		if (maxValue < 0)
			return;
		int nb = maxValue + 1;
		if (initValues.size() != nb || canPropagateVal.size() != nb)
			return;

		std::vector<bool> closedList(nb, false), canPropagate(nb, false);
		std::deque<int> openList;
		output.assign(nb, 0);

		for (int i = 0; i < nb; ++i)
		{
			if (initValues[i] != 0)
			{
				closedList[i] = true;
				openList.push_back(i);
				output[i] = 1;
			}
			if (canPropagateVal[i] != 0)
				canPropagate[i] = true;
			else
				closedList[i] = true; // Do not process these points 
		}

		while (!openList.empty())
		{
			int id = openList.front();
			openList.pop_front();

			for (auto n : neighbors[id].values)
			{
				if (!closedList[n] && canPropagate[n])
				{
					closedList[n] = true;
					openList.push_back(n);
					output[n] = 1;
				}
			}
		}
	}

protected:
	Data< std::vector<IntVector> > m_neighbors;
	Data< std::vector<int> > m_init, m_canPropagate, m_output;
};

int Polygon_PropagateClass = RegisterObject<Polygon_Propagate>("Math/Polygon/Topology/Propagate")
	.setDescription("Propagate a value to neighbors");

} // namespace Panda


