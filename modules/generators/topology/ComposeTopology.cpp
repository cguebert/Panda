#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Topology.h>

namespace panda {

using types::Topology;

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
		auto topo = topology.getAccessor();

		topo->clear();
		topo->addPoints(pts);

		Topology::Polygon poly;
		for(int i=0; i<pts.size(); ++i)
			poly.push_back(i);
		topo->addPolygon(poly);

		this->cleanDirty();
	}

protected:
	Data< QVector<QPointF> > points;
	Data<Topology> topology;
};

int GeneratorTopology_OnePolygonClass = RegisterObject<GeneratorTopology_OnePolygon>("Generator/Topology/Create one polygon").setName("Pts to polygon").setDescription("Create a topology (of one polygon) using a list of points");

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
		auto pts = points.getAccessor();

		pts.clear();

		int nbEdges = topo.getNumberOfEdges();
		for(int i=0; i<nbEdges; ++i)
		{
			const Topology::Edge& e = topo.getEdge(i);
			pts.push_back(topoPts[e.first]);
			pts.push_back(topoPts[e.second]);
		}

		this->cleanDirty();
	}

protected:
	Data<Topology> topology;
	Data< QVector<QPointF> > points;
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

		this->cleanDirty();
	}

protected:
	Data<Topology> topology;
	Data< QVector<QPointF> > points;
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

		for(int i : polyId)
			outTopo->addPolygon(inTopo.getPolygon(i));

//		outTopo->createPolygonsAroundPointList();
//		const QVector<Topology::IndicesList>& papl = outTopo->getPolygonsAroundPointList();

		this->cleanDirty();
	}

protected:
	Data<Topology> input, output;
	Data< QVector<int> > polygons;
};

int GeneratorTopology_ExtractPolygonsClass = RegisterObject<GeneratorTopology_ExtractPolygons>("Generator/Topology/Extract polygons")
		.setName("Extract poly").setDescription("Extract some polygons from a topology");

//*************************************************************************//

} // namespace Panda
