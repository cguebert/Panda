#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Topology.h>

#include <set>

namespace panda {

using types::Topology;

class ModifierTopology_FindNeighbors : public PandaObject
{
public:
	PANDA_CLASS(ModifierTopology_FindNeighbors, PandaObject)

	ModifierTopology_FindNeighbors(PandaDocument *doc)
		: PandaObject(doc)
		, topology(initData(&topology, "topology", "Topology in which to search"))
		, polygons(initData(&polygons, "input", "Polygons indices to test"))
		, neighbors(initData(&neighbors, "neighbors", "Indices of the polygons, neighbors of the input"))
	{
		addInput(&topology);
		addInput(&polygons);

		addOutput(&neighbors);
	}

	void update()
	{
		Topology topo = topology.getValue();

		const QVector<int>& polyIDs = polygons.getValue();

		Topology::PolygonsIndicesList inputList;
		for(auto p : polyIDs)
		{
			if(p != Topology::InvalidID)
				inputList.push_back(p);
		}
		Topology::PolygonsIndicesList outputList = topo.getPolygonsAroundPolygons(inputList);

		auto output = neighbors.getAccessor();
		output.clear();
		for(auto p : outputList)
			output.push_back(p);

		cleanDirty();
	}

protected:
	Data< Topology > topology;
	Data< QVector<int> > polygons, neighbors;
};

int ModifierTopology_FindNeighborsClass = RegisterObject<ModifierTopology_FindNeighbors>("Modifier/Topology/Find neighbors").setDescription("Find neighboring polygons to the input list");

//*************************************************************************//

class ModifierTopology_GetConnected : public PandaObject
{
public:
	PANDA_CLASS(ModifierTopology_GetConnected, PandaObject)

	ModifierTopology_GetConnected(PandaDocument *doc)
		: PandaObject(doc)
		, topology(initData(&topology, "topology", "Topology in which to search"))
		, polygons(initData(&polygons, "input", "Polygons indices to test"))
		, connected(initData(&connected, "connected", "Indices of the polygons connected to the input"))
	{
		addInput(&topology);
		addInput(&polygons);

		addOutput(&connected);
	}

	void update()
	{
		Topology topo = topology.getValue();

		const QVector<int>& polyIDs = polygons.getValue();
		std::set<Topology::PolygonID> outputSet;

		for(auto polyID : polyIDs)
		{
			if(polyID == Topology::InvalidID)
				continue;
			Topology::PolygonsIndicesList tmp = topo.getPolygonsConnectedToPolygon(polyID);
			outputSet.insert(tmp.begin(), tmp.end());
		}

		auto output = connected.getAccessor();
		output.clear();
		for(auto p : outputSet)
			output.push_back(p);

		cleanDirty();
	}

protected:
	Data< Topology > topology;
	Data< QVector<int> > polygons, connected;
};

int ModifierTopology_GetConnectedClass = RegisterObject<ModifierTopology_GetConnected>("Modifier/Topology/Get connected").setDescription("Get connected polygons to the input list");


} // namespace Panda
