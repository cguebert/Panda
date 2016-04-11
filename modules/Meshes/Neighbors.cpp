#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/IntVector.h>
#include <panda/types/Mesh.h>

#include <set>

namespace panda {

using types::IntVector;
using types::Mesh;

class MeshInfo_PointsInEdges : public PandaObject
{
public:
	PANDA_CLASS(MeshInfo_PointsInEdges, PandaObject)

	MeshInfo_PointsInEdges(PandaDocument *doc)
		: PandaObject(doc)
		, m_mesh(initData("mesh", "Mesh to analyse"))
		, m_output(initData("output", "Indices of points in each edge"))
	{
		addInput(m_mesh);
		addOutput(m_output);
	}

	void update()
	{
		const Mesh& inMesh = m_mesh.getValue();
		auto acc = m_output.getAccessor();
		auto& output = acc.wref();
		output.clear();
		for (const auto& list : inMesh.getEdges())
		{
			IntVector vec;
			for (auto id : list)
				vec.values.push_back(id);
			output.push_back(vec);
		}
	}

protected:
	Data< Mesh > m_mesh;
	Data< std::vector<IntVector> > m_output;
};

int MeshInfo_PointsInEdgesClass = RegisterObject<MeshInfo_PointsInEdges>("Math/Mesh/Topology/Points in edges")
	.setDescription("Get the list of points in each edge of a mesh");

//****************************************************************************//

class MeshInfo_PointsInTriangles : public PandaObject
{
public:
	PANDA_CLASS(MeshInfo_PointsInTriangles, PandaObject)

	MeshInfo_PointsInTriangles(PandaDocument *doc)
		: PandaObject(doc)
		, m_mesh(initData("mesh", "Mesh to analyse"))
		, m_output(initData("output", "Indices of points in each triangle"))
	{
		addInput(m_mesh);
		addOutput(m_output);
	}

	void update()
	{
		const Mesh& inMesh = m_mesh.getValue();
		auto acc = m_output.getAccessor();
		auto& output = acc.wref();
		output.clear();
		for (const auto& list : inMesh.getTriangles())
		{
			IntVector vec;
			for (auto id : list)
				vec.values.push_back(id);
			output.push_back(vec);
		}
	}

protected:
	Data< Mesh > m_mesh;
	Data< std::vector<IntVector> > m_output;
};

int MeshInfo_PointsInTrianglesClass = RegisterObject<MeshInfo_PointsInTriangles>("Math/Mesh/Topology/Points in triangles")
	.setDescription("Get the list of points in each triangle of a mesh");

//****************************************************************************//

class MeshInfo_EdgesInTriangles : public PandaObject
{
public:
	PANDA_CLASS(MeshInfo_EdgesInTriangles, PandaObject)

	MeshInfo_EdgesInTriangles(PandaDocument *doc)
		: PandaObject(doc)
		, m_mesh(initData("mesh", "Mesh to analyse"))
		, m_output(initData("output", "Indices of edges in each triangle"))
	{
		addInput(m_mesh);
		addOutput(m_output);
	}

	void update()
	{
		const Mesh& inMesh = m_mesh.getValue();
		auto acc = m_output.getAccessor();
		auto& output = acc.wref();
		output.clear();
		for (const auto& list : inMesh.getEdgesInTriangleList())
		{
			IntVector vec;
			for (auto id : list)
				vec.values.push_back(id);
			output.push_back(vec);
		}
	}

protected:
	Data< Mesh > m_mesh;
	Data< std::vector<IntVector> > m_output;
};

int MeshInfo_EdgesInTrianglesClass = RegisterObject<MeshInfo_EdgesInTriangles>("Math/Mesh/Topology/Edges in triangles")
	.setDescription("Get the list of edges in each triangle of a mesh");

//****************************************************************************//

class MeshInfo_EdgesAroundPoints : public PandaObject
{
public:
	PANDA_CLASS(MeshInfo_EdgesAroundPoints, PandaObject)

	MeshInfo_EdgesAroundPoints(PandaDocument *doc)
		: PandaObject(doc)
		, m_mesh(initData("mesh", "Mesh to analyse"))
		, m_output(initData("output", "Indices of edges around each point"))
	{
		addInput(m_mesh);
		addOutput(m_output);
	}

	void update()
	{
		const Mesh& inMesh = m_mesh.getValue();
		auto acc = m_output.getAccessor();
		auto& output = acc.wref();
		output.clear();
		for (const auto& list : inMesh.getEdgesAroundPointList())
		{
			IntVector vec;
			for (auto id : list)
				vec.values.push_back(id);
			output.push_back(vec);
		}
	}

protected:
	Data< Mesh > m_mesh;
	Data< std::vector<IntVector> > m_output;
};

int MeshInfo_EdgesAroundPointsClass = RegisterObject<MeshInfo_EdgesAroundPoints>("Math/Mesh/Topology/Edges around points")
	.setDescription("Get the list of edges around each point of a mesh");

//****************************************************************************//

class MeshInfo_TrianglesAroundPoints : public PandaObject
{
public:
	PANDA_CLASS(MeshInfo_TrianglesAroundPoints, PandaObject)

	MeshInfo_TrianglesAroundPoints(PandaDocument *doc)
		: PandaObject(doc)
		, m_mesh(initData("mesh", "Mesh to analyse"))
		, m_output(initData("output", "Indices of triangles around each point"))
	{
		addInput(m_mesh);
		addOutput(m_output);
	}

	void update()
	{
		const Mesh& inMesh = m_mesh.getValue();
		auto acc = m_output.getAccessor();
		auto& output = acc.wref();
		output.clear();
		for (const auto& list : inMesh.getTrianglesAroundPointList())
		{
			IntVector vec;
			for (auto id : list)
				vec.values.push_back(id);
			output.push_back(vec);
		}
	}

protected:
	Data< Mesh > m_mesh;
	Data< std::vector<IntVector> > m_output;
};

int MeshInfo_TrianglesAroundPointsClass = RegisterObject<MeshInfo_TrianglesAroundPoints>("Math/Mesh/Topology/Triangles around points")
	.setDescription("Get the list of triangles around each point of a mesh");

//****************************************************************************//

class MeshInfo_TrianglesAroundEdges : public PandaObject
{
public:
	PANDA_CLASS(MeshInfo_TrianglesAroundEdges, PandaObject)

	MeshInfo_TrianglesAroundEdges(PandaDocument *doc)
		: PandaObject(doc)
		, m_mesh(initData("mesh", "Mesh to analyse"))
		, m_output(initData("output", "Indices of triangles around each edge"))
	{
		addInput(m_mesh);
		addOutput(m_output);
	}

	void update()
	{
		const Mesh& inMesh = m_mesh.getValue();
		auto acc = m_output.getAccessor();
		auto& output = acc.wref();
		output.clear();
		for (const auto& list : inMesh.getTrianglesAroundEdgeList())
		{
			IntVector vec;
			for (auto id : list)
				vec.values.push_back(id);
			output.push_back(vec);
		}
	}

protected:
	Data< Mesh > m_mesh;
	Data< std::vector<IntVector> > m_output;
};

int MeshInfo_TrianglesAroundEdgesClass = RegisterObject<MeshInfo_TrianglesAroundEdges>("Math/Mesh/Topology/Triangles around edges")
	.setDescription("Get the list of triangles around each edge of a mesh");

//****************************************************************************//

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
		int nbTriangles = inMesh.nbTriangles();
		for(auto p : polyIDs)
		{
			if(p != Mesh::InvalidID && p >= 0 && p < nbTriangles)
				inputList.push_back(p);
		}
		Mesh::TrianglesIndicesList outputList = inMesh.getTrianglesAroundTriangles(inputList, m_testEdges.getValue() != 0);

		auto output = m_neighbors.getAccessor();
		output.clear();
		for(auto p : outputList)
			output.push_back(p);
	}

protected:
	Data< Mesh > m_mesh;
	Data< int > m_testEdges;
	Data< std::vector<int> > m_triangles, m_neighbors;
};

int ModifierMesh_FindNeighborsClass = RegisterObject<ModifierMesh_FindNeighbors>("Modifier/Mesh/Topology/Neighbor triangles")
	.setDescription("Find neighboring triangles to the input triangles");

//****************************************************************************//

class ModifierMesh_FindNeighborPoints : public PandaObject
{
public:
	PANDA_CLASS(ModifierMesh_FindNeighborPoints, PandaObject)

		ModifierMesh_FindNeighborPoints(PandaDocument *doc)
		: PandaObject(doc)
		, m_mesh(initData("mesh", "Mesh in which to search"))
		, m_points(initData("input", "Points indices to test"))
		, m_neighbors(initData("neighbors", "Indices of the points, neighbors of the input"))
	{
		addInput(m_mesh);
		addInput(m_points);

		addOutput(m_neighbors);
	}

	void update()
	{
		Mesh inMesh = m_mesh.getValue();

		const std::vector<int>& ptsIDs = m_points.getValue();

		Mesh::PointsIndicesList inputList;
		int nbPoints = inMesh.nbPoints();
		for (auto p : ptsIDs)
		{
			if (p != Mesh::InvalidID && p >= 0 && p < nbPoints)
				inputList.push_back(p);
		}
		Mesh::PointsIndicesList outputList = inMesh.getPointsAroundPoints(inputList);

		auto output = m_neighbors.getAccessor();
		output.clear();
		for (auto p : outputList)
			output.push_back(p);
	}

protected:
	Data< Mesh > m_mesh;
	Data< std::vector<int> > m_points, m_neighbors;
};

int ModifierMesh_FindNeighborPointsClass = RegisterObject<ModifierMesh_FindNeighborPoints>("Modifier/Mesh/Topology/Neighbor points")
	.setDescription("Find points sharing an edge with the input points");

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
			Mesh::TrianglesIndicesList tmp = inMesh.getTrianglesConnectedToTriangle(triID, m_testEdges.getValue() != 0);
			outputSet.insert(tmp.begin(), tmp.end());
		}

		auto output = m_connected.getAccessor();
		output.clear();
		for(auto p : outputSet)
			output.push_back(p);
	}

protected:
	Data< Mesh > m_mesh;
	Data< int > m_testEdges;
	Data< std::vector<int> > m_triangles, m_connected;
};

int ModifierMesh_GetConnectedClass = RegisterObject<ModifierMesh_GetConnected>("Modifier/Mesh/Topology/Get connected").setDescription("Get connected triangles to the input list");

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

		bool testEdges = m_testEdges.getValue() != 0;

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
	}

protected:
	Data< std::vector<Mesh> > m_input;
	Data< int > m_testEdges;
	Data< std::vector<Mesh> > m_output;
};

int ModifierMesh_SeparateDisconnectedClass = RegisterObject<ModifierMesh_SeparateDisconnected>("Modifier/Mesh/Topology/Separate disconnected")
		.setDescription("Separate the disconnected parts of a mesh into many meshes");


} // namespace Panda
