#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Mesh.h>

#include <set>

namespace panda {

using types::Mesh;

class ModifierMesh_FindNeighbors : public PandaObject
{
public:
	PANDA_CLASS(ModifierMesh_FindNeighbors, PandaObject)

	ModifierMesh_FindNeighbors(PandaDocument *doc)
		: PandaObject(doc)
		, mesh(initData(&mesh, "mesh", "Mesh in which to search"))
		, polygons(initData(&polygons, "input", "Polygons indices to test"))
		, neighbors(initData(&neighbors, "neighbors", "Indices of the polygons, neighbors of the input"))
	{
		addInput(&mesh);
		addInput(&polygons);

		addOutput(&neighbors);
	}

	void update()
	{
		Mesh topo = mesh.getValue();

		const QVector<int>& polyIDs = polygons.getValue();

		Mesh::PolygonsIndicesList inputList;
		for(auto p : polyIDs)
		{
			if(p != Mesh::InvalidID)
				inputList.push_back(p);
		}
		Mesh::PolygonsIndicesList outputList = topo.getPolygonsAroundPolygons(inputList);

		auto output = neighbors.getAccessor();
		output.clear();
		for(auto p : outputList)
			output.push_back(p);

		cleanDirty();
	}

protected:
	Data< Mesh > mesh;
	Data< QVector<int> > polygons, neighbors;
};

int ModifierMesh_FindNeighborsClass = RegisterObject<ModifierMesh_FindNeighbors>("Modifier/Mesh/Find neighbors").setDescription("Find neighboring polygons to the input list");

//*************************************************************************//

class ModifierMesh_GetConnected : public PandaObject
{
public:
	PANDA_CLASS(ModifierMesh_GetConnected, PandaObject)

	ModifierMesh_GetConnected(PandaDocument *doc)
		: PandaObject(doc)
		, mesh(initData(&mesh, "mesh", "Mesh in which to search"))
		, polygons(initData(&polygons, "input", "Polygons indices to test"))
		, connected(initData(&connected, "connected", "Indices of the polygons connected to the input"))
	{
		addInput(&mesh);
		addInput(&polygons);

		addOutput(&connected);
	}

	void update()
	{
		Mesh topo = mesh.getValue();

		const QVector<int>& polyIDs = polygons.getValue();
		std::set<Mesh::PolygonID> outputSet;

		for(auto polyID : polyIDs)
		{
			if(polyID == Mesh::InvalidID)
				continue;
			Mesh::PolygonsIndicesList tmp = topo.getPolygonsConnectedToPolygon(polyID);
			outputSet.insert(tmp.begin(), tmp.end());
		}

		auto output = connected.getAccessor();
		output.clear();
		for(auto p : outputSet)
			output.push_back(p);

		cleanDirty();
	}

protected:
	Data< Mesh > mesh;
	Data< QVector<int> > polygons, connected;
};

int ModifierMesh_GetConnectedClass = RegisterObject<ModifierMesh_GetConnected>("Modifier/Mesh/Get connected").setDescription("Get connected polygons to the input list");

//*************************************************************************//

class ModifierMesh_SeparateDisconnected : public PandaObject
{
public:
	PANDA_CLASS(ModifierMesh_SeparateDisconnected, PandaObject)

	ModifierMesh_SeparateDisconnected(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "Input mesh"))
		, outputs(initData(&outputs, "output", "List of separated meshes"))
	{
		addInput(&input);

		addOutput(&outputs);
	}

	void update()
	{
		Mesh topo = input.getValue();

		auto topoList = outputs.getAccessor();
		topoList.clear();

		std::set<Mesh::PolygonID> polySet;
		for(int i=0, nb=topo.getNumberOfPolygons(); i<nb; ++i)
			polySet.insert(i);

		while(!polySet.empty())
		{
			Mesh::PolygonID polyID = *polySet.begin();
			Mesh newTopo;
			newTopo.addPoints(topo.getPoints());
			newTopo.addPolygon(topo.getPolygon(polyID));
			auto list = topo.getPolygonsConnectedToPolygon(polyID);

			for(auto i : list)
			{
				newTopo.addPolygon(topo.getPolygon(i));
				polySet.erase(i);
			}

			newTopo.removeUnusedPoints();
			topoList.push_back(newTopo);
		}

		cleanDirty();
	}

protected:
	Data< Mesh > input;
	Data< QVector<Mesh> > outputs;
};

int ModifierMesh_SeparateDisconnectedClass = RegisterObject<ModifierMesh_SeparateDisconnected>("Modifier/Mesh/Separate disconnected")
		.setDescription("Separate the disconnected parts of a mesh into many meshes");


} // namespace Panda
