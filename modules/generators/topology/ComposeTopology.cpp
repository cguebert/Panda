#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Topology.h>

namespace panda {

using types::Topology;
using types::Point;

class GeneratorTopology_OnePolygon : public PandaObject
{
public:
	PANDA_CLASS(GeneratorTopology_OnePolygon, PandaObject)

	GeneratorTopology_OnePolygon(PandaDocument *doc)
		: PandaObject(doc)
		, points(initData(&points, "points", "List of points forming the polygon"))
		, topology(initData(&topology, "topology", "Topology created from the list of points"))
	{
		addInput(&points);

		addOutput(&topology);
	}

	void update()
	{
		const QVector<Point>& pts = points.getValue();
		auto topo = topology.getAccessor();

		topo->clear();
		topo->addPoints(pts);

		Topology::Polygon poly;
		for(int i=0; i<pts.size(); ++i)
			poly.push_back(i);
		topo->addPolygon(poly);

		cleanDirty();
	}

protected:
	Data< QVector<Point> > points;
	Data<Topology> topology;
};

int GeneratorTopology_OnePolygonClass = RegisterObject<GeneratorTopology_OnePolygon>("Generator/Topology/Create one polygon").setName("Points to polygon").setDescription("Create a topology (of one polygon) using a list of points");

//*************************************************************************//

class GeneratorTopology_Wireframe : public PandaObject
{
public:
	PANDA_CLASS(GeneratorTopology_Wireframe, PandaObject)

	GeneratorTopology_Wireframe(PandaDocument *doc)
		: PandaObject(doc)
		, topology(initData(&topology, "topology", "Topology from which to extract the edges"))
		, points(initData(&points, "points", "Pairs of points forming the edges"))
	{
		addInput(&topology);

		addOutput(&points);
	}

	void update()
	{
		Topology topo = topology.getValue();
		const QVector<Point>& topoPts = topo.getPoints();
		auto pts = points.getAccessor();

		pts.clear();

		int nbEdges = topo.getNumberOfEdges();
		for(int i=0; i<nbEdges; ++i)
		{
			const Topology::Edge& e = topo.getEdge(i);
			pts.push_back(topoPts[e.first]);
			pts.push_back(topoPts[e.second]);
		}

		cleanDirty();
	}

protected:
	Data<Topology> topology;
	Data< QVector<Point> > points;
};

int GeneratorTopology_WireframeClass = RegisterObject<GeneratorTopology_Wireframe>("Generator/Topology/Wireframe").setDescription("Extract the edges from a topology");

//*************************************************************************//

class GeneratorTopology_Vertices : public PandaObject
{
public:
	PANDA_CLASS(GeneratorTopology_Vertices, PandaObject)

	GeneratorTopology_Vertices(PandaDocument *doc)
		: PandaObject(doc)
		, topology(initData(&topology, "topology", "Topology from which to extract the vertices"))
		, points(initData(&points, "points", "Points used by the topology"))
	{
		addInput(&topology);

		addOutput(&points);
	}

	void update()
	{
		const Topology& topo = topology.getValue();

		auto pts = points.getAccessor();
		pts = topo.getPoints();

		cleanDirty();
	}

protected:
	Data<Topology> topology;
	Data< QVector<Point> > points;
};

int GeneratorTopology_VerticesClass = RegisterObject<GeneratorTopology_Vertices>("Generator/Topology/Vertices").setDescription("Extract the vertices of a topology");

//*************************************************************************//

class GeneratorTopology_ExtractPolygons : public PandaObject
{
public:
	PANDA_CLASS(GeneratorTopology_ExtractPolygons, PandaObject)

	GeneratorTopology_ExtractPolygons(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "Input topology"))
		, output(initData(&output, "output", "Output topology"))
		, polygons(initData(&polygons, "polygons", "Indices of the polygons to extract"))
	{
		addInput(&input);
		addInput(&polygons);

		addOutput(&output);
	}

	void update()
	{
		const Topology& inTopo = input.getValue();
		const QVector<int>& polyId = polygons.getValue();

		auto outTopo = output.getAccessor();

		outTopo->clear();
		outTopo->addPoints(inTopo.getPoints());

		int nbPoly = inTopo.getNumberOfPolygons();

		for(int i : polyId)
		{
			if(i != Topology::InvalidID && i < nbPoly)
				outTopo->addPolygon(inTopo.getPolygon(i));
		}

		outTopo->removeUnusedPoints();

		cleanDirty();
	}

protected:
	Data<Topology> input, output;
	Data< QVector<int> > polygons;
};

int GeneratorTopology_ExtractPolygonsClass = RegisterObject<GeneratorTopology_ExtractPolygons>("Generator/Topology/Extract polygons")
		.setDescription("Extract some polygons from a topology");

//*************************************************************************//

class GeneratorTopology_BorderElements : public PandaObject
{
public:
	PANDA_CLASS(GeneratorTopology_BorderElements, PandaObject)

	GeneratorTopology_BorderElements(PandaDocument *doc)
		: PandaObject(doc)
		, topology(initData(&topology, "topology", "Input topology"))
		, points(initData(&points, "points", "Indices of the points on the border"))
		, edges(initData(&edges, "edges", "Indices of the edges on the border"))
		, polygons(initData(&polygons, "polygons", "Indices of the polygons on the border"))
	{
		addInput(&topology);

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
		Topology inTopo = topology.getValue();

		auto outPoints = points.getAccessor();
		auto outEdges = edges.getAccessor();
		auto outPolygons = polygons.getAccessor();

		outPoints.wref() = toIntVector(inTopo.getPointsOnBorder());
		outEdges.wref() = toIntVector(inTopo.getEdgesOnBorder());
		outPolygons.wref() = toIntVector(inTopo.getPolygonsOnBorder());

		cleanDirty();
	}

protected:
	Data<Topology> topology;
	Data< QVector<int> > points, edges, polygons;
};

int GeneratorTopology_BorderElementsClass = RegisterObject<GeneratorTopology_BorderElements>("Generator/Topology/Border elements")
		.setDescription("Get the indices of the elements on the border");

//*************************************************************************//

class GeneratorTopology_ExtractEdges : public PandaObject
{
public:
	PANDA_CLASS(GeneratorTopology_ExtractEdges, PandaObject)

	GeneratorTopology_ExtractEdges(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "Input topology"))
		, output(initData(&output, "output", "Points pairs forming the extracted edges"))
		, edges(initData(&edges, "edges", "Indices of the edges to extract"))
	{
		addInput(&input);
		addInput(&edges);

		addOutput(&output);
	}

	void update()
	{
		const Topology& inTopo = input.getValue();
		const QVector<int>& edgesId = edges.getValue();

		auto outPts = output.getAccessor();

		outPts.clear();

		int nbEdges = inTopo.getNumberOfEdges();

		for(int i : edgesId)
		{
			if(i != Topology::InvalidID && i < nbEdges)
			{
				Topology::Edge e = inTopo.getEdge(i);
				outPts.push_back(inTopo.getPoint(e.first));
				outPts.push_back(inTopo.getPoint(e.second));
			}
		}

		cleanDirty();
	}

protected:
	Data<Topology> input;
	Data< QVector<Point> > output;
	Data< QVector<int> > edges;
};

int GeneratorTopology_ExtractEdgesClass = RegisterObject<GeneratorTopology_ExtractEdges>("Generator/Topology/Extract edges")
		.setDescription("Extract some edges from a topology");

} // namespace Panda
