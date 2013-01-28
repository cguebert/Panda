#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/Topology.h>

namespace panda {

class TopologyMath_NumberOfPrimitives : public PandaObject
{
public:
	PANDA_CLASS(TopologyMath_NumberOfPrimitives, PandaObject)

	TopologyMath_NumberOfPrimitives(PandaDocument *doc)
		: PandaObject(doc)
		, topology(initData(&topology, "topology", "Topology to analyse"))
		, nbPoints(initData(&nbPoints, "nb points", "Number of points in the topology"))
		, nbEdges(initData(&nbEdges, "nb edges", "Number of edges in the topology"))
		, nbPolygons(initData(&nbPolygons, "nb polygons", "Number of polygons in the topology"))
	{
		addInput(&topology);

		addOutput(&nbPoints);
		addOutput(&nbEdges);
		addOutput(&nbPolygons);
	}

	void update()
	{
		const Topology& topo = topology.getValue();

		nbPoints.setValue(topo.getNumberOfPoints());
		nbEdges.setValue(topo.getNumberOfEdges());
		nbPolygons.setValue(topo.getNumberOfPolygons());

		this->cleanDirty();
	}

protected:
	Data<Topology> topology;
	Data<int> nbPoints, nbEdges, nbPolygons;
};

int TopologyMath_NumberOfPrimitivesClass = RegisterObject("Math/Topology/Number of primitives")
		.setName("# of primitives").setClass<TopologyMath_NumberOfPrimitives>()
		.setDescription("Compute the number of points, edges and polygons in the topology");

//*************************************************************************//

} // namespace Panda
