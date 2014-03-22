#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Topology.h>

namespace panda {

using types::Topology;
using types::Point;

class TopologyMath_Centroids : public PandaObject
{
public:
	PANDA_CLASS(TopologyMath_Centroids, PandaObject)

	TopologyMath_Centroids(PandaDocument *doc)
		: PandaObject(doc)
		, topology(initData(&topology, "topology", "Topology to analyse"))
		, centroids(initData(&centroids, "centroids", "Centroids of the polygons"))
	{
		addInput(&topology);

		addOutput(&centroids);
	}

	void update()
	{
		const Topology& topo = topology.getValue();
		auto pts = centroids.getAccessor();
		pts.clear();
		pts.resize(topo.getNumberOfPolygons());

		int i=0;
		for(auto poly : topo.getPolygons())
			pts[i++] = topo.centroidOfPolygon(poly);

		cleanDirty();
	}

protected:
	Data<Topology> topology;
	Data< QVector<Point> > centroids;
};

int TopologyMath_CentroidsClass = RegisterObject<TopologyMath_Centroids>("Math/Topology/Centroids")
		.setDescription("Compute the centroid of each polygon");

} // namespace Panda
