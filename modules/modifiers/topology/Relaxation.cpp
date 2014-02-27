#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Topology.h>

namespace panda {

using types::Topology;

class ModifierTopology_Relaxation : public PandaObject
{
public:
	PANDA_CLASS(ModifierTopology_Relaxation, PandaObject)

	ModifierTopology_Relaxation(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "Input topology"))
		, output(initData(&output, "output", "Output topology"))
		, factor(initData(&factor, 0.1, "factor", "Portion of each edge added to a point"))
		, iterations(initData(&iterations, 1, "iterations", "Number of times to do the relaxation"))
	{
		addInput(&input);
		addInput(&factor);
		addInput(&iterations);

		addOutput(&output);
	}

	void update()
	{
		auto outTopo = output.getAccessor();
		outTopo.wref() = input.getValue();
		if(!outTopo->hasEdgesAroundPoint())
			outTopo->createEdgesAroundPointList();

		int nbIter = iterations.getValue();
		int nbPts = outTopo->getNumberOfPoints();
		double fact = factor.getValue();
		for(int i=0; i<nbIter; ++i)
		{
			for(int j=0; j<nbPts; ++j)
			{
				QPointF& pt = outTopo->getPoint(j);
				for(auto eid : outTopo->getEdgesAroundPoint(j))
				{
					Topology::Edge e = outTopo->getEdge(eid);
					Topology::PointID p2id = outTopo->getOtherPointInEdge(e, j);
					QPointF pt2 = outTopo->getPoint(p2id);

					QPointF dir = pt2 - pt;
					pt += dir * fact;
				}
			}
		}

		cleanDirty();
	}

protected:
	Data< Topology > input, output;
	Data< double > factor;
	Data< int > iterations;
};

int ModifierTopology_RelaxationClass = RegisterObject<ModifierTopology_Relaxation>("Modifier/Topology/Relaxation").setDescription("Pull each point a little bit closer to its neighbors");

} // namespace Panda
