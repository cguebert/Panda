#include <panda/object/ObjectFactory.h>

#include <panda/types/Mesh.h>

namespace flags
{
	enum
	{
		Edges = 1,
		Triangles = 1 << 1,
		EdgesInTriangle = 1 << 2,
		EdgesAroundPoint = 1 << 3,
		TrianglesAroundPoint = 1 << 4,
		TrianglesAroundEdge = 1 << 5,
		BorderElements = 1 << 6
	};
}

namespace panda {

using types::Mesh;

class MeshMath_PrepareTopology : public PandaObject
{
public:
	PANDA_CLASS(MeshMath_PrepareTopology, PandaObject)

		MeshMath_PrepareTopology(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Mesh to analyse"))
		, m_output(initData("output", "Prepared mesh"))
		, m_flags(initData(127, "flags", "Choose what to compute"))
	{
		addInput(m_input);
		addInput(m_flags);
		addOutput(m_output);

		m_flags.setWidget("flags");
		m_flags.setWidgetData("Edges;Triangles;Edges in triangle;Edges around point;Triangles around point;Triangles around edge;Border elements");
	}

	void update()
	{
		const auto& inputs = m_input.getValue();
		auto acc = m_output.getAccessor();
		acc.clear();
		auto& outMeshes = acc.wref();

		int flagsVal = m_flags.getValue();
		bool computeEdges = (flagsVal & flags::Edges) != 0;
		bool computeTriangles = (flagsVal & flags::Triangles) != 0;
		bool computeEdgesInTriangle = (flagsVal & flags::EdgesInTriangle) != 0;
		bool computeEdgesAroundPoint = (flagsVal & flags::EdgesAroundPoint) != 0;
		bool computeTrianglesAroundPoint = (flagsVal & flags::TrianglesAroundPoint) != 0;
		bool computeTrianglesAroundEdge = (flagsVal & flags::TrianglesAroundEdge) != 0;
		bool computeBorderElements = (flagsVal & flags::BorderElements) != 0;

		for (auto mesh : inputs) // We want a copy as we will modify it eventually
		{
			if (!mesh.hasPoints())
			{
				outMeshes.push_back(mesh);
				continue;
			}

			if (computeEdges && !mesh.hasEdges() && mesh.hasTriangles())
				mesh.createEdgeList();
			else if (computeTriangles && mesh.hasEdges() && !mesh.hasTriangles())
				mesh.createTriangles();

			if (computeEdgesInTriangle && !mesh.hasEdgesInTriangle())
				mesh.createEdgesInTriangleList();
			if (computeEdgesAroundPoint && !mesh.hasEdgesAroundPoint())
				mesh.createEdgesAroundPointList();

			if (computeTrianglesAroundPoint && !mesh.hasTrianglesAroundPoint())
				mesh.createTrianglesAroundPointList();
			if (computeTrianglesAroundEdge && !mesh.hasTrianglesAroundEdge())
				mesh.createTrianglesAroundEdgeList();

			if (computeBorderElements && mesh.hasBorderElementsLists())
				mesh.createElementsOnBorder();

			outMeshes.push_back(std::move(mesh));
		}
	}

protected:
	Data<std::vector<Mesh>> m_input, m_output;
	Data<int> m_flags;
};

int MeshMath_PrepareTopologyClass = RegisterObject<MeshMath_PrepareTopology>("Math/Mesh/Topology/Prepare Topology")
	.setDescription("Compute the topology information for the input mesh, so that it is only done once");

//****************************************************************************//

class MeshMath_NumberOfPrimitives : public PandaObject
{
public:
	PANDA_CLASS(MeshMath_NumberOfPrimitives, PandaObject)

	MeshMath_NumberOfPrimitives(PandaDocument *doc)
		: PandaObject(doc)
		, m_mesh(initData("mesh", "Mesh to analyse"))
		, m_nbPoints(initData("nb points", "Number of points in the mesh"))
		, m_nbEdges(initData("nb edges", "Number of edges in the mesh"))
		, m_nbTriangles(initData("nb triangles", "Number of triangles in the mesh"))
	{
		addInput(m_mesh);

		addOutput(m_nbPoints);
		addOutput(m_nbEdges);
		addOutput(m_nbTriangles);
	}

	void update()
	{
		const auto& inputs = m_mesh.getValue();

		auto accPoints = m_nbPoints.getAccessor();
		auto accEdges = m_nbEdges.getAccessor();
		auto accTriangles = m_nbTriangles.getAccessor();

		auto& outPoints = accPoints.wref();		
		auto& outEdges = accEdges.wref();
		auto& outTriangles = accTriangles.wref();

		outPoints.clear();
		outEdges.clear();
		outTriangles.clear();

		for (const auto& mesh : inputs)
		{
			outPoints.push_back(mesh.nbPoints());
			outEdges.push_back(mesh.nbEdges());
			outTriangles.push_back(mesh.nbTriangles());
		}
	}

protected:
	Data<std::vector<Mesh>> m_mesh;
	Data<std::vector<int>> m_nbPoints, m_nbEdges, m_nbTriangles;
};

int MeshMath_NumberOfPrimitivesClass = RegisterObject<MeshMath_NumberOfPrimitives>("Math/Mesh/Number of primitives")
		.setDescription("Compute the number of points, edges and triangles in the mesh");

//****************************************************************************//

class MeshMath_Area : public PandaObject
{
public:
	PANDA_CLASS(MeshMath_Area, PandaObject)

	MeshMath_Area(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("mesh", "Mesh to analyse"))
		, m_output(initData("area", "Area of the mesh"))
	{
		addInput(m_input);
		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto output = m_output.getAccessor();

		int nb = input.size();
		output.resize(nb);
		for(int i=0; i<nb; ++i)
		{
			const auto& mesh = input[i];
			float area = 0;
			for(int j=0, nbT=mesh.nbTriangles(); j<nbT; ++j)
				area += mesh.areaOfTriangle(mesh.getTriangle(j));
			output[i] = fabs(area);
		}
	}

protected:
	Data< std::vector<Mesh> > m_input;
	Data< std::vector<float> > m_output;
};

int MeshMath_AreaClass = RegisterObject<MeshMath_Area>("Math/Mesh/Area")
						 .setName("Area of mesh")
						 .setDescription("Compute the area of a mesh");


} // namespace Panda
