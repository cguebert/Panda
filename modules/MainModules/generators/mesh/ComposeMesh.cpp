#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Mesh.h>
#include <panda/types/Polygon.h>

namespace panda {

using types::Mesh;
using types::Point;

class GeneratorMesh_OnePolygon : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_OnePolygon, PandaObject)

	GeneratorMesh_OnePolygon(PandaDocument *doc)
		: PandaObject(doc)
		, points(initData(&points, "points", "List of points forming the polygon"))
		, mesh(initData(&mesh, "mesh", "Mesh created from the list of points"))
	{
		addInput(points);

		addOutput(mesh);
	}

	void update()
	{
		const QVector<Point>& pts = points.getValue();
		int nbPts = pts.size();
		auto outMesh = mesh.getAccessor();

		outMesh->clear();

		if(nbPts < 3)
		{
			cleanDirty();
			return;
		}

		outMesh->addPoints(pts);

		if(nbPts == 3)
		{
			outMesh->addTriangle(0, 1, 2);
		}
		else
		{
			Point center = types::centroidOfPolygon(pts);
			Mesh::PointID centerId = outMesh->addPoint(center);
			for(int i=0; i<nbPts; ++i)
			{
				int j = (i+1) % nbPts;
				outMesh->addTriangle(i, j, centerId);
			}
		}

		cleanDirty();
	}

protected:
	Data< QVector<Point> > points;
	Data<Mesh> mesh;
};

int GeneratorMesh_OnePolygonClass = RegisterObject<GeneratorMesh_OnePolygon>("Generator/Mesh/Create one polygon").setName("Points to polygon").setDescription("Create a mesh (of one polygon) using a list of points");

//****************************************************************************//

class GeneratorMesh_Wireframe : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_Wireframe, PandaObject)

	GeneratorMesh_Wireframe(PandaDocument *doc)
		: PandaObject(doc)
		, mesh(initData(&mesh, "mesh", "Mesh from which to extract the edges"))
		, points(initData(&points, "points", "Pairs of points forming the edges"))
	{
		addInput(mesh);

		addOutput(points);
	}

	void update()
	{
		Mesh inMesh = mesh.getValue();
		const QVector<Point>& meshPts = inMesh.getPoints();
		auto pts = points.getAccessor();

		pts.clear();

		if(!inMesh.hasEdges())
			inMesh.createEdgeList();
		int nbEdges = inMesh.getNumberOfEdges();
		for(int i=0; i<nbEdges; ++i)
		{
			const Mesh::Edge& e = inMesh.getEdge(i);
			pts.push_back(meshPts[e[0]]);
			pts.push_back(meshPts[e[1]]);
		}

		cleanDirty();
	}

protected:
	Data<Mesh> mesh;
	Data< QVector<Point> > points;
};

int GeneratorMesh_WireframeClass = RegisterObject<GeneratorMesh_Wireframe>("Generator/Mesh/Wireframe").setDescription("Extract the edges from a mesh");

//****************************************************************************//

class GeneratorMesh_Vertices : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_Vertices, PandaObject)

	GeneratorMesh_Vertices(PandaDocument *doc)
		: PandaObject(doc)
		, mesh(initData(&mesh, "mesh", "Mesh from which to extract the vertices"))
		, points(initData(&points, "points", "Points used by the mesh"))
	{
		addInput(mesh);

		addOutput(points);
	}

	void update()
	{
		const Mesh& inMesh = mesh.getValue();

		auto pts = points.getAccessor();
		pts = inMesh.getPoints();

		cleanDirty();
	}

protected:
	Data<Mesh> mesh;
	Data< QVector<Point> > points;
};

int GeneratorMesh_VerticesClass = RegisterObject<GeneratorMesh_Vertices>("Generator/Mesh/Vertices").setDescription("Extract the vertices of a mesh");

//****************************************************************************//

class GeneratorMesh_ExtractTriangles : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_ExtractTriangles, PandaObject)

	GeneratorMesh_ExtractTriangles(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "Input mesh"))
		, output(initData(&output, "output", "Output mesh"))
		, triangles(initData(&triangles, "triangles", "Indices of the triangles to extract"))
	{
		addInput(input);
		addInput(triangles);

		addOutput(output);
	}

	void update()
	{
		const Mesh& inMesh = input.getValue();
		const QVector<int>& triId = triangles.getValue();

		auto outMesh = output.getAccessor();

		outMesh->clear();
		outMesh->addPoints(inMesh.getPoints());

		int nbTri = inMesh.getNumberOfTriangles();

		for(int i : triId)
		{
			if(i != Mesh::InvalidID && i < nbTri)
				outMesh->addTriangle(inMesh.getTriangle(i));
		}

		outMesh->removeUnusedPoints();

		cleanDirty();
	}

protected:
	Data<Mesh> input, output;
	Data< QVector<int> > triangles;
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
		, mesh(initData(&mesh, "mesh", "Input mesh"))
		, points(initData(&points, "points", "Indices of the points on the border"))
		, edges(initData(&edges, "edges", "Indices of the edges on the border"))
		, triangles(initData(&triangles, "triangles", "Indices of the triangles on the border"))
	{
		addInput(mesh);

		addOutput(points);
		addOutput(edges);
		addOutput(triangles);
	}

	QVector<int> toIntVector(const QVector<unsigned int>& in)
	{
		QVector<int> tmp;
		tmp.reserve(in.size());
		for(auto v : in)
			tmp.push_back(v);
		return tmp;
	}

	void update()
	{
		Mesh inMesh = mesh.getValue();

		auto outPoints = points.getAccessor();
		auto outEdges = edges.getAccessor();
		auto outTriangles = triangles.getAccessor();

		outPoints.wref() = toIntVector(inMesh.getPointsOnBorder());
		outEdges.wref() = toIntVector(inMesh.getEdgesOnBorder());
		outTriangles.wref() = toIntVector(inMesh.getTrianglesOnBorder());

		cleanDirty();
	}

protected:
	Data<Mesh> mesh;
	Data< QVector<int> > points, edges, triangles;
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
		, input(initData(&input, "input", "Input mesh"))
		, output(initData(&output, "output", "Points pairs forming the extracted edges"))
		, edges(initData(&edges, "edges", "Indices of the edges to extract"))
	{
		addInput(input);
		addInput(edges);

		addOutput(output);
	}

	void update()
	{
		Mesh inMesh = input.getValue();
		const QVector<int>& edgesId = edges.getValue();

		auto outPts = output.getAccessor();

		outPts.clear();

		if(!inMesh.hasEdges())
			inMesh.createEdgeList();
		int nbEdges = inMesh.getNumberOfEdges();

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
	Data<Mesh> input;
	Data< QVector<Point> > output;
	Data< QVector<int> > edges;
};

int GeneratorMesh_ExtractEdgesClass = RegisterObject<GeneratorMesh_ExtractEdges>("Generator/Mesh/Extract edges")
		.setDescription("Extract some edges from a mesh");

} // namespace Panda
