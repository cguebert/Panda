#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
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
		, m_mesh(initData("mesh", "Mesh in which to search"))
		, m_triangles(initData("input", "Triangles indices to test"))
		, m_testEdges(initData(true, "share edges", "If true, meshes must shared edges (not only vertices)"))
		, m_neighbors(initData("neighbors", "Indices of the triangles, neighbors of the input"))
	{
		addInput(m_mesh);
		addInput(m_triangles);
		addInput(m_testEdges);
		m_testEdges.setWidget("checkbox");

		addOutput(m_neighbors);
	}

	void update()
	{
		Mesh inMesh = m_mesh.getValue();

		const std::vector<int>& polyIDs = m_triangles.getValue();

		Mesh::TrianglesIndicesList inputList;
		for(auto p : polyIDs)
		{
			if(p != Mesh::InvalidID)
				inputList.push_back(p);
		}
		Mesh::TrianglesIndicesList outputList = inMesh.getTrianglesAroundTriangles(inputList, m_testEdges.getValue());

		auto output = m_neighbors.getAccessor();
		output.clear();
		for(auto p : outputList)
			output.push_back(p);

		cleanDirty();
	}

protected:
	Data< Mesh > m_mesh;
	Data< int > m_testEdges;
	Data< std::vector<int> > m_triangles, m_neighbors;
};

int ModifierMesh_FindNeighborsClass = RegisterObject<ModifierMesh_FindNeighbors>("Modifier/Mesh/Find neighbors").setDescription("Find neighboring triangles to the input list");

//****************************************************************************//

class ModifierMesh_GetConnected : public PandaObject
{
public:
	PANDA_CLASS(ModifierMesh_GetConnected, PandaObject)

	ModifierMesh_GetConnected(PandaDocument *doc)
		: PandaObject(doc)
		, m_mesh(initData("mesh", "Mesh in which to search"))
		, m_triangles(initData("input", "Triangles indices to test"))
		, m_testEdges(initData(true, "share edges", "If true, meshes must share edges (not only vertices)"))
		, m_connected(initData("connected", "Indices of the triangles connected to the input"))
	{
		addInput(m_mesh);
		addInput(m_triangles);
		addInput(m_testEdges);
		m_testEdges.setWidget("checkbox");

		addOutput(m_connected);
	}

	void update()
	{
		Mesh inMesh = m_mesh.getValue();

		const std::vector<int>& triIDs = m_triangles.getValue();
		std::set<Mesh::TriangleID> outputSet;

		for(auto triID : triIDs)
		{
			if(triID == Mesh::InvalidID)
				continue;
			Mesh::TrianglesIndicesList tmp = inMesh.getTrianglesConnectedToTriangle(triID, m_testEdges.getValue());
			outputSet.insert(tmp.begin(), tmp.end());
		}

		auto output = m_connected.getAccessor();
		output.clear();
		for(auto p : outputSet)
			output.push_back(p);

		cleanDirty();
	}

protected:
	Data< Mesh > m_mesh;
	Data< int > m_testEdges;
	Data< std::vector<int> > m_triangles, m_connected;
};

int ModifierMesh_GetConnectedClass = RegisterObject<ModifierMesh_GetConnected>("Modifier/Mesh/Get connected").setDescription("Get connected triangles to the input list");

//****************************************************************************//

class ModifierMesh_SeparateDisconnected : public PandaObject
{
public:
	PANDA_CLASS(ModifierMesh_SeparateDisconnected, PandaObject)

	ModifierMesh_SeparateDisconnected(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input mesh"))
		, m_testEdges(initData(true, "share edges", "If true, meshes must share edges (not only vertices)"))
		, m_output(initData("output", "List of separated meshes"))
	{
		addInput(m_input);
		addInput(m_testEdges);
		m_testEdges.setWidget("checkbox");

		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();

		auto output = m_output.getAccessor();
		output.clear();

		bool testEdges = m_testEdges.getValue();

		for(auto mesh : input)
		{
			std::set<Mesh::TriangleID> triSet;
			for(int i=0, nb=mesh.nbTriangles(); i<nb; ++i)
				triSet.insert(i);

			while(!triSet.empty())
			{
				Mesh::TriangleID triID = *triSet.begin();
				Mesh newMesh;
				newMesh.addPoints(mesh.getPoints());
				auto list = mesh.getTrianglesConnectedToTriangle(triID, testEdges);

				for(auto i : list)
				{
					newMesh.addTriangle(mesh.getTriangle(i));
					triSet.erase(i);
				}

				newMesh.removeUnusedPoints();
				output.push_back(newMesh);
			}
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Mesh> > m_input;
	Data< int > m_testEdges;
	Data< std::vector<Mesh> > m_output;
};

int ModifierMesh_SeparateDisconnectedClass = RegisterObject<ModifierMesh_SeparateDisconnected>("Modifier/Mesh/Separate disconnected")
		.setDescription("Separate the disconnected parts of a mesh into many meshes");


} // namespace Panda
