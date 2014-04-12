#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Mesh.h>

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
		addInput(&points);

		addOutput(&mesh);
	}

	void update()
	{
		const QVector<Point>& pts = points.getValue();
		auto topo = mesh.getAccessor();

		topo->clear();
		topo->addPoints(pts);

		Mesh::Polygon poly;
		for(int i=0; i<pts.size(); ++i)
			poly.push_back(i);
		topo->addPolygon(poly);

		cleanDirty();
	}

protected:
	Data< QVector<Point> > points;
	Data<Mesh> mesh;
};

int GeneratorMesh_OnePolygonClass = RegisterObject<GeneratorMesh_OnePolygon>("Generator/Mesh/Create one polygon").setName("Points to polygon").setDescription("Create a mesh (of one polygon) using a list of points");

//*************************************************************************//

class GeneratorMesh_Wireframe : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_Wireframe, PandaObject)

	GeneratorMesh_Wireframe(PandaDocument *doc)
		: PandaObject(doc)
		, mesh(initData(&mesh, "mesh", "Mesh from which to extract the edges"))
		, points(initData(&points, "points", "Pairs of points forming the edges"))
	{
		addInput(&mesh);

		addOutput(&points);
	}

	void update()
	{
		Mesh topo = mesh.getValue();
		const QVector<Point>& topoPts = topo.getPoints();
		auto pts = points.getAccessor();

		pts.clear();

		int nbEdges = topo.getNumberOfEdges();
		for(int i=0; i<nbEdges; ++i)
		{
			const Mesh::Edge& e = topo.getEdge(i);
			pts.push_back(topoPts[e.first]);
			pts.push_back(topoPts[e.second]);
		}

		cleanDirty();
	}

protected:
	Data<Mesh> mesh;
	Data< QVector<Point> > points;
};

int GeneratorMesh_WireframeClass = RegisterObject<GeneratorMesh_Wireframe>("Generator/Mesh/Wireframe").setDescription("Extract the edges from a mesh");

//*************************************************************************//

class GeneratorMesh_Vertices : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_Vertices, PandaObject)

	GeneratorMesh_Vertices(PandaDocument *doc)
		: PandaObject(doc)
		, mesh(initData(&mesh, "mesh", "Mesh from which to extract the vertices"))
		, points(initData(&points, "points", "Points used by the mesh"))
	{
		addInput(&mesh);

		addOutput(&points);
	}

	void update()
	{
		const Mesh& topo = mesh.getValue();

		auto pts = points.getAccessor();
		pts = topo.getPoints();

		cleanDirty();
	}

protected:
	Data<Mesh> mesh;
	Data< QVector<Point> > points;
};

int GeneratorMesh_VerticesClass = RegisterObject<GeneratorMesh_Vertices>("Generator/Mesh/Vertices").setDescription("Extract the vertices of a mesh");

//*************************************************************************//

class GeneratorMesh_ExtractPolygons : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_ExtractPolygons, PandaObject)

	GeneratorMesh_ExtractPolygons(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "Input mesh"))
		, output(initData(&output, "output", "Output mesh"))
		, polygons(initData(&polygons, "polygons", "Indices of the polygons to extract"))
	{
		addInput(&input);
		addInput(&polygons);

		addOutput(&output);
	}

	void update()
	{
		const Mesh& inTopo = input.getValue();
		const QVector<int>& polyId = polygons.getValue();

		auto outTopo = output.getAccessor();

		outTopo->clear();
		outTopo->addPoints(inTopo.getPoints());

		int nbPoly = inTopo.getNumberOfPolygons();

		for(int i : polyId)
		{
			if(i != Mesh::InvalidID && i < nbPoly)
				outTopo->addPolygon(inTopo.getPolygon(i));
		}

		outTopo->removeUnusedPoints();

		cleanDirty();
	}

protected:
	Data<Mesh> input, output;
	Data< QVector<int> > polygons;
};

int GeneratorMesh_ExtractPolygonsClass = RegisterObject<GeneratorMesh_ExtractPolygons>("Generator/Mesh/Extract polygons")
		.setDescription("Extract some polygons from a mesh");

//*************************************************************************//

class GeneratorMesh_BorderElements : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_BorderElements, PandaObject)

	GeneratorMesh_BorderElements(PandaDocument *doc)
		: PandaObject(doc)
		, mesh(initData(&mesh, "mesh", "Input mesh"))
		, points(initData(&points, "points", "Indices of the points on the border"))
		, edges(initData(&edges, "edges", "Indices of the edges on the border"))
		, polygons(initData(&polygons, "polygons", "Indices of the polygons on the border"))
	{
		addInput(&mesh);

		addOutput(&points);
		addOutput(&edges);
		addOutput(&polygons);
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
		Mesh inTopo = mesh.getValue();

		auto outPoints = points.getAccessor();
		auto outEdges = edges.getAccessor();
		auto outPolygons = polygons.getAccessor();

		outPoints.wref() = toIntVector(inTopo.getPointsOnBorder());
		outEdges.wref() = toIntVector(inTopo.getEdgesOnBorder());
		outPolygons.wref() = toIntVector(inTopo.getPolygonsOnBorder());

		cleanDirty();
	}

protected:
	Data<Mesh> mesh;
	Data< QVector<int> > points, edges, polygons;
};

int GeneratorMesh_BorderElementsClass = RegisterObject<GeneratorMesh_BorderElements>("Generator/Mesh/Border elements")
		.setDescription("Get the indices of the elements on the border");

//*************************************************************************//

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
		addInput(&input);
		addInput(&edges);

		addOutput(&output);
	}

	void update()
	{
		const Mesh& inTopo = input.getValue();
		const QVector<int>& edgesId = edges.getValue();

		auto outPts = output.getAccessor();

		outPts.clear();

		int nbEdges = inTopo.getNumberOfEdges();

		for(int i : edgesId)
		{
			if(i != Mesh::InvalidID && i < nbEdges)
			{
				Mesh::Edge e = inTopo.getEdge(i);
				outPts.push_back(inTopo.getPoint(e.first));
				outPts.push_back(inTopo.getPoint(e.second));
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
