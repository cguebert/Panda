#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Topology.h>

#include <set>

namespace panda {

using types::Topology;

class ModifierTopology_EdgeRelaxation : public PandaObject
{
public:
	PANDA_CLASS(ModifierTopology_EdgeRelaxation, PandaObject)

	ModifierTopology_EdgeRelaxation(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "Input topology"))
		, output(initData(&output, "output", "Output topology"))
		, factor(initData(&factor, (PReal)0.1, "factor", "Portion of each edge added to a point"))
		, iterations(initData(&iterations, 1, "iterations", "Number of times to do the relaxation"))
		, fixBorder(initData(&fixBorder, 0, "fix border", "If true, the points on the border will not move"))
	{
		addInput(&input);
		addInput(&factor);
		addInput(&iterations);
		addInput(&fixBorder);
		fixBorder.setWidget("checkbox");

		addOutput(&output);
	}

	void update()
	{
		auto outTopo = output.getAccessor();
		outTopo.wref() = input.getValue();
		if(!outTopo->hasEdgesAroundPoint())
			outTopo->createEdgesAroundPointList();

		std::set<int> borderSet;
		if(fixBorder.getValue())
		{
			auto borderPts = outTopo->getPointsOnBorder();
			borderSet.insert(borderPts.begin(), borderPts.end());
		}

		int nbIter = iterations.getValue();
		int nbPts = outTopo->getNumberOfPoints();
		double fact = factor.getValue();
		for(int i=0; i<nbIter; ++i)
		{
			auto ptsCopy = outTopo->getPoints();
			for(int j=0; j<nbPts; ++j)
			{
				if(borderSet.find(j) != borderSet.end())
					continue;

				QPointF& pt = outTopo->getPoint(j);
				QPointF pt1 = ptsCopy[j];
				for(auto eid : outTopo->getEdgesAroundPoint(j))
				{
					Topology::Edge e = outTopo->getEdge(eid);
					Topology::PointID p2id = outTopo->getOtherPointInEdge(e, j);
					QPointF pt2 = ptsCopy[p2id];

					QPointF dir = pt2 - pt1;
					pt += dir * fact;
				}
			}
		}

		cleanDirty();
	}

protected:
	Data< Topology > input, output;
	Data< PReal > factor;
	Data< int > iterations, fixBorder;
};

int ModifierTopology_EdgeRelaxationClass = RegisterObject<ModifierTopology_EdgeRelaxation>("Modifier/Topology/Edge relaxation")
		.setDescription("Pull each point a little bit closer to its neighbors");

} // namespace Panda
