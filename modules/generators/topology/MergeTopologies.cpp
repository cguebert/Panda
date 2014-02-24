#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Topology.h>
#include <panda/helper/PointsGrid.h>

#include <map>

template<>
bool qMapLessThanKey<QPointF>(const QPointF& p1, const QPointF& p2)
{
	return p1.x() < p2.x() || (p1.x() == p2.x() && p1.y() < p2.y());
}

namespace panda {

using types::Topology;

class GeneratorTopology_MergeTopologies : public PandaObject
{
public:
	PANDA_CLASS(GeneratorTopology_MergeTopologies, PandaObject)

	GeneratorTopology_MergeTopologies(PandaDocument *doc)
		: PandaObject(doc)
		, inputs(initData(&inputs, "inputs", "Topologies to merge"))
		, threshold(initData(&threshold, 0.1, "threshold", "Distance under which 2 points are considered the same"))
		, output(initData(&output, "output", "Merged topology"))
	{
		addInput(&inputs);
		addInput(&threshold);

		addOutput(&output);
	}

	void update()
	{
		helper::PointsGrid grid;
		QSize size = parentDocument->getRenderSize();
		QRectF area(0, 0, size.width(), size.height());
		grid.initGrid(area, 10);

		double thres = threshold.getValue();

		Topology::SeqPoints newPoints;
		Topology::SeqPolygons newPolygons;

		QMap<QPointF, Topology::PointID> pointsMap;

		const QVector<Topology>& topoList = inputs.getValue();
		for(const Topology& topo : topoList)
		{
			if(newPoints.empty())
			{
				newPoints = topo.getPoints();
				newPolygons = topo.getPolygons();
				grid.addPoints(newPoints);
				for(int i=0, nb=newPoints.size(); i<nb; ++i)
					pointsMap[newPoints[i]] = i;
			}
			else
			{
				QMap<Topology::PointID, Topology::PointID> tmpIDMap;
				for(int i=0, nb=topo.getNumberOfPoints(); i<nb; ++i)
				{
					const QPointF& pt = topo.getPoint(i);
					QPointF res;
					if(grid.getNearest(pt, thres, res))
					{
						tmpIDMap[i] = pointsMap[res];
					}
					else
					{
						int id = newPoints.size();
						pointsMap[pt] = id;
						tmpIDMap[i] = id;
						newPoints.push_back(pt);
						grid.addPoint(pt);
					}
				}

				for(auto poly : topo.getPolygons())
				{
					for(auto& id : poly)
						id = tmpIDMap[id];

					newPolygons.push_back(poly);
				}
			}
		}

		auto outTopo = output.getAccessor();
		outTopo->clear();
		outTopo->addPoints(newPoints);
		outTopo->addPolygons(newPolygons);

		cleanDirty();
	}

protected:
	Data< QVector<Topology> > inputs;
	Data< double > threshold;
	Data< Topology > output;
};

int GeneratorTopology_MergeTopologiesClass = RegisterObject<GeneratorTopology_MergeTopologies>("Generator/Topology/Merge topologies").setDescription("Merge multiple topologies into one");

} // namespace Panda
