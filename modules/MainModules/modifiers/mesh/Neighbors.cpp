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
		, triangles(initData(&triangles, "input", "Triangles indices to test"))
		, neighbors(initData(&neighbors, "neighbors", "Indices of the triangles, neighbors of the input"))
	{
		addInput(&mesh);
		addInput(&triangles);

		addOutput(&neighbors);
	}

	void update()
	{
		Mesh inMesh = mesh.getValue();

		const QVector<int>& polyIDs = triangles.getValue();

		Mesh::TrianglesIndicesList inputList;
		for(auto p : polyIDs)
		{
			if(p != Mesh::InvalidID)
				inputList.push_back(p);
		}
		Mesh::TrianglesIndicesList outputList = inMesh.getTrianglesAroundTriangles(inputList);

		auto output = neighbors.getAccessor();
		output.clear();
		for(auto p : outputList)
			output.push_back(p);

		cleanDirty();
	}

protected:
	Data< Mesh > mesh;
	Data< QVector<int> > triangles, neighbors;
};

int ModifierMesh_FindNeighborsClass = RegisterObject<ModifierMesh_FindNeighbors>("Modifier/Mesh/Find neighbors").setDescription("Find neighboring triangles to the input list");

//****************************************************************************//

class ModifierMesh_GetConnected : public PandaObject
{
public:
	PANDA_CLASS(ModifierMesh_GetConnected, PandaObject)

	ModifierMesh_GetConnected(PandaDocument *doc)
		: PandaObject(doc)
		, mesh(initData(&mesh, "mesh", "Mesh in which to search"))
		, triangles(initData(&triangles, "input", "Triangles indices to test"))
		, connected(initData(&connected, "connected", "Indices of the triangles connected to the input"))
	{
		addInput(&mesh);
		addInput(&triangles);

		addOutput(&connected);
	}

	void update()
	{
		Mesh inMesh = mesh.getValue();

		const QVector<int>& triIDs = triangles.getValue();
		std::set<Mesh::TriangleID> outputSet;

		for(auto triID : triIDs)
		{
			if(triID == Mesh::InvalidID)
				continue;
			Mesh::TrianglesIndicesList tmp = inMesh.getTrianglesConnectedToTriangle(triID);
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
	Data< QVector<int> > triangles, connected;
};

int ModifierMesh_GetConnectedClass = RegisterObject<ModifierMesh_GetConnected>("Modifier/Mesh/Get connected").setDescription("Get connected triangles to the input list");

//****************************************************************************//

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
		Mesh inMesh = input.getValue();

		auto outMeshes = outputs.getAccessor();
		outMeshes.clear();

		std::set<Mesh::TriangleID> triSet;
		for(int i=0, nb=inMesh.getNumberOfTriangles(); i<nb; ++i)
			triSet.insert(i);

		while(!triSet.empty())
		{
			Mesh::TriangleID triID = *triSet.begin();
			Mesh newMesh;
			newMesh.addPoints(inMesh.getPoints());
			newMesh.addTriangle(inMesh.getTriangle(triID));
			auto list = inMesh.getTrianglesConnectedToTriangle(triID);

			for(auto i : list)
			{
				newMesh.addTriangle(inMesh.getTriangle(i));
				triSet.erase(i);
			}

			newMesh.removeUnusedPoints();
			outMeshes.push_back(newMesh);
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
