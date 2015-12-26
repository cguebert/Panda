#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Mesh.h>
#include <panda/types/Path.h>

namespace panda {

using types::Mesh;
using types::Path;
using types::Point;

class GeneratorMesh_OnePolygon : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_OnePolygon, PandaObject)

	GeneratorMesh_OnePolygon(PandaDocument *doc)
		: PandaObject(doc)
		, m_points(initData("points", "List of points forming the polygon"))
		, m_mesh(initData("mesh", "Mesh created from the list of points"))
	{
		addInput(m_points);

		addOutput(m_mesh);
	}

	void update()
	{
		const std::vector<Path>& paths = m_points.getValue();
		int nbPaths = paths.size();

		auto outMesh = m_mesh.getAccessor();
		outMesh.clear();
		outMesh.resize(nbPaths);

		for(int i=0; i<nbPaths; ++i)
		{
			const Path& path = paths[i];
			int nbPts = path.size();
			if(nbPts < 3)
				continue;

			Mesh mesh;
			mesh.addPoints(path);

			if(nbPts == 3)
			{
				mesh.addTriangle(0, 1, 2);
			}
			else
			{
				Point center = types::centroidOfPolygon(path);
				Mesh::PointID centerId = mesh.addPoint(center);
				for(int j=0; j<nbPts; ++j)
				{
					int k = (j+1) % nbPts;
					mesh.addTriangle(j, k, centerId);
				}
			}

			outMesh[i] = std::move(mesh);
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Path> > m_points;
	Data< std::vector<Mesh> > m_mesh;
};

int GeneratorMesh_OnePolygonClass = RegisterObject<GeneratorMesh_OnePolygon>("Generator/Mesh/Create one polygon").setName("Points to polygon").setDescription("Create a mesh (of one polygon) using a list of points");

//****************************************************************************//

class GeneratorMesh_Wireframe : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_Wireframe, PandaObject)

	GeneratorMesh_Wireframe(PandaDocument *doc)
		: PandaObject(doc)
		, m_mesh(initData("mesh", "Mesh from which to extract the edges"))
		, m_points(initData("points", "Pairs of points forming the edges"))
	{
		addInput(m_mesh);

		addOutput(m_points);
	}

	void update()
	{
		Mesh inMesh = m_mesh.getValue();
		const std::vector<Point>& meshPts = inMesh.getPoints();
		auto pts = m_points.getAccessor();

		pts.clear();

		if(!inMesh.hasEdges())
			inMesh.createEdgeList();
		int nbEdges = inMesh.nbEdges();
		for(int i=0; i<nbEdges; ++i)
		{
			const Mesh::Edge& e = inMesh.getEdge(i);
			pts.push_back(meshPts[e[0]]);
			pts.push_back(meshPts[e[1]]);
		}

		cleanDirty();
	}

protected:
	Data<Mesh> m_mesh;
	Data< std::vector<Point> > m_points;
};

int GeneratorMesh_WireframeClass = RegisterObject<GeneratorMesh_Wireframe>("Generator/Mesh/Wireframe").setDescription("Extract the edges from a mesh");

//****************************************************************************//

class GeneratorMesh_Vertices : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_Vertices, PandaObject)

	GeneratorMesh_Vertices(PandaDocument *doc)
		: PandaObject(doc)
		, m_mesh(initData("mesh", "Mesh from which to extract the vertices"))
		, m_points(initData("points", "Points used by the mesh"))
	{
		addInput(m_mesh);

		addOutput(m_points);
	}

	void update()
	{
		const Mesh& inMesh = m_mesh.getValue();

		auto pts = m_points.getAccessor();
		pts = inMesh.getPoints();

		cleanDirty();
	}

protected:
	Data<Mesh> m_mesh;
	Data< std::vector<Point> > m_points;
};

int GeneratorMesh_VerticesClass = RegisterObject<GeneratorMesh_Vertices>("Generator/Mesh/Vertices").setDescription("Extract the vertices of a mesh");

//****************************************************************************//

class GeneratorMesh_ExtractTriangles : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_ExtractTriangles, PandaObject)

	GeneratorMesh_ExtractTriangles(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input mesh"))
		, m_output(initData("output", "Output mesh"))
		, m_triangles(initData("triangles", "Indices of the triangles to extract"))
	{
		addInput(m_input);
		addInput(m_triangles);

		addOutput(m_output);
	}

	void update()
	{
		const Mesh& inMesh = m_input.getValue();
		const std::vector<int>& triId = m_triangles.getValue();

		auto outMesh = m_output.getAccessor();

		outMesh->clear();
		outMesh->addPoints(inMesh.getPoints());

		int nbTri = inMesh.nbTriangles();

		for(int i : triId)
		{
			if(i != Mesh::InvalidID && i < nbTri)
				outMesh->addTriangle(inMesh.getTriangle(i));
		}

		outMesh->removeUnusedPoints();

		cleanDirty();
	}

protected:
	Data<Mesh> m_input, m_output;
	Data< std::vector<int> > m_triangles;
};

int GeneratorMesh_ExtractTrianglesClass = RegisterObject<GeneratorMesh_ExtractTriangles>("Generator/Mesh/Extract triangles")
		.setDescription("Extract some triangles from a mesh");

//****************************************************************************//

class GeneratorMesh_BorderElements : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_BorderElements, PandaObject)

	GeneratorMesh_BorderElements(PandaDocument *doc)
		: PandaObject(doc)
		, m_mesh(initData("mesh", "Input mesh"))
		, m_points(initData("points", "Indices of the points on the border"))
		, m_edges(initData("edges", "Indices of the edges on the border"))
		, m_triangles(initData("triangles", "Indices of the triangles on the border"))
	{
		addInput(m_mesh);

		addOutput(m_points);
		addOutput(m_edges);
		addOutput(m_triangles);
	}

	std::vector<int> toIntVector(const std::vector<unsigned int>& in)
	{
		std::vector<int> tmp;
		tmp.reserve(in.size());
		for(auto v : in)
			tmp.push_back(v);
		return tmp;
	}

	void update()
	{
		Mesh inMesh = m_mesh.getValue();

		auto outPoints = m_points.getAccessor();
		auto outEdges = m_edges.getAccessor();
		auto outTriangles = m_triangles.getAccessor();

		outPoints.wref() = toIntVector(inMesh.getPointsOnBorder());
		outEdges.wref() = toIntVector(inMesh.getEdgesOnBorder());
		outTriangles.wref() = toIntVector(inMesh.getTrianglesOnBorder());

		cleanDirty();
	}

protected:
	Data<Mesh> m_mesh;
	Data< std::vector<int> > m_points, m_edges, m_triangles;
};

int GeneratorMesh_BorderElementsClass = RegisterObject<GeneratorMesh_BorderElements>("Generator/Mesh/Border elements")
		.setDescription("Get the indices of the elements on the border");

//****************************************************************************//

class GeneratorMesh_ExtractEdges : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_ExtractEdges, PandaObject)

	GeneratorMesh_ExtractEdges(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input mesh"))
		, m_output(initData("output", "Points pairs forming the extracted edges"))
		, m_edges(initData("edges", "Indices of the edges to extract"))
	{
		addInput(m_input);
		addInput(m_edges);

		addOutput(m_output);
	}

	void update()
	{
		Mesh inMesh = m_input.getValue();
		const std::vector<int>& edgesId = m_edges.getValue();

		auto outPts = m_output.getAccessor();

		outPts.clear();

		if(!inMesh.hasEdges())
			inMesh.createEdgeList();
		int nbEdges = inMesh.nbEdges();

		for(int i : edgesId)
		{
			if(i != Mesh::InvalidID && i < nbEdges)
			{
				Mesh::Edge e = inMesh.getEdge(i);
				outPts.push_back(inMesh.getPoint(e[0]));
				outPts.push_back(inMesh.getPoint(e[1]));
			}
		}

		cleanDirty();
	}

protected:
	Data<Mesh> m_input;
	Data< std::vector<Point> > m_output;
	Data< std::vector<int> > m_edges;
};

int GeneratorMesh_ExtractEdgesClass = RegisterObject<GeneratorMesh_ExtractEdges>("Generator/Mesh/Extract edges")
		.setDescription("Extract some edges from a mesh");

//****************************************************************************//

class ModifierMesh_MoveVertices : public PandaObject
{
public:
	PANDA_CLASS(ModifierMesh_MoveVertices, PandaObject)

	ModifierMesh_MoveVertices(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input mesh"))
		, m_output(initData("output", "Output mesh"))
		, m_points(initData("vertices", "New position of the vertices"))
	{
		addInput(m_input);
		addInput(m_points);

		addOutput(m_output);
	}

	void update()
	{
		const Mesh& input = m_input.getValue();
		const std::vector<Point>& points = m_points.getValue();
		auto output = m_output.getAccessor();

		output.wref() = input;
		int nb = std::min(input.nbPoints(), static_cast<int>(points.size()));
		for(int i=0; i<nb; ++i)
			output.wref().getPoint(i) = points[i];

		cleanDirty();
	}

protected:
	Data<Mesh> m_input, m_output;
	Data< std::vector<Point> > m_points;
};

int ModifierMesh_MoveVerticesClass = RegisterObject<ModifierMesh_MoveVertices>("Modifier/Mesh/Move vertices").setDescription("Move the vertices of a mesh");

} // namespace Panda
