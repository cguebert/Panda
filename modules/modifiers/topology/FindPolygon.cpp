#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Topology.h>

namespace panda {

using types::Topology;
using types::Point;

class ModifierTopology_FindPolygon : public PandaObject
{
public:
	PANDA_CLASS(ModifierTopology_FindPolygon, PandaObject)

	ModifierTopology_FindPolygon(PandaDocument *doc)
		: PandaObject(doc)
		, topology(initData(&topology, "topology", "Topology in which to search"))
		, points(initData(&points, "points", "List of points to test"))
		, indices(initData(&indices, "indices", "Indices of the polygons are the corresponding points"))
	{
		addInput(&topology);
		addInput(&points);

		addOutput(&indices);
	}

	void update()
	{
		const Topology& topo = topology.getValue();

		const QVector<Point>& pts = points.getValue();
		auto output = indices.getAccessor();
		int nbPts = pts.size();
		output.wref().fill(Topology::InvalidID, nbPts);

		int nbPoly = topo.getNumberOfPolygons();

		for(int i=0; i<nbPts; ++i)
		{
			Point pt = pts[i];
			for(int j=0; j<nbPoly; ++j)
			{
				const Topology::Polygon& poly = topo.getPolygon(j);
				if(topo.polygonContainsPoint(poly, pt))
				{
					output[i] = j;
					break;
				}
			}
		}

		cleanDirty();
	}

protected:
	Data< Topology > topology;
	Data< QVector<Point> > points;
	Data< QVector<int> > indices;
};

int ModifierTopology_FindPolygonClass = RegisterObject<ModifierTopology_FindPolygon>("Modifier/Topology/Find polygon").setDescription("Find polygons are specific positions");

} // namespace Panda
