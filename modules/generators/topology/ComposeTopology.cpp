#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/Topology.h>

namespace panda {

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
		const QVector<QPointF>& pts = points.getValue();
		Topology& topo = *topology.beginEdit();

		topo.clear();
		topo.addPoints(pts);

		Topology::Polygon poly;
		for(int i=0; i<pts.size(); ++i)
			poly.push_back(i);
		topo.addPolygon(poly);

		topology.endEdit();
		this->cleanDirty();
	}

protected:
	Data< QVector<QPointF> > points;
	Data<Topology> topology;
};

int GeneratorTopology_OnePolygonClass = RegisterObject("Generator/Topology/Create one polygon").setName("Pts to polygon").setClass<GeneratorTopology_OnePolygon>().setDescription("Create a topology (of one polygon) using a list of points");

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
		const QVector<QPointF>& topoPts = topo.getPoints();
		QVector<QPointF>& pts = *points.beginEdit();

		pts.clear();

		int nbEdges = topo.getNumberOfEdges();
		for(int i=0; i<nbEdges; ++i)
		{
			const Topology::Edge& e = topo.getEdge(i);
			pts << topoPts[e.first] << topoPts[e.second];
		}

		points.endEdit();
		this->cleanDirty();
	}

protected:
	Data<Topology> topology;
	Data< QVector<QPointF> > points;
};

int GeneratorTopology_WireframeClass = RegisterObject("Generator/Topology/Wireframe").setClass<GeneratorTopology_Wireframe>().setDescription("Extract the edges from a topology");

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

		QVector<QPointF>& pts = *points.beginEdit();
		pts = topo.getPoints();
		points.endEdit();

		this->cleanDirty();
	}

protected:
	Data<Topology> topology;
	Data< QVector<QPointF> > points;
};

int GeneratorTopology_VerticesClass = RegisterObject("Generator/Topology/Vertices").setClass<GeneratorTopology_Vertices>().setDescription("Extract the vertices of a topology");

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

		Topology& outTopo = *output.beginEdit();

		outTopo.clear();
		outTopo.addPoints(inTopo.getPoints());

		foreach(int i, polyId)
			outTopo.addPolygon(inTopo.getPolygon(i));

//		outTopo.createPolygonsAroundPointList();
//		const QVector<Topology::IndicesList>& papl = outTopo.getPolygonsAroundPointList();

		output.endEdit();

		this->cleanDirty();
	}

protected:
	Data<Topology> input, output;
	Data< QVector<int> > polygons;
};

int GeneratorTopology_ExtractPolygonsClass = RegisterObject("Generator/Topology/Extract polygons")
		.setName("Extract poly").setClass<GeneratorTopology_ExtractPolygons>()
		.setDescription("Extract some polygons from a topology");

//*************************************************************************//

} // namespace Panda
