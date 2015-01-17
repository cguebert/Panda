#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Mesh.h>

#include <set>

namespace panda {

using types::Point;
using types::Mesh;

class ModifierMesh_EdgeRelaxation : public PandaObject
{
public:
	PANDA_CLASS(ModifierMesh_EdgeRelaxation, PandaObject)

	ModifierMesh_EdgeRelaxation(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "Input mesh"))
		, output(initData(&output, "output", "Output mesh"))
		, factor(initData(&factor, (PReal)0.1, "factor", "Portion of each edge added to a point"))
		, iterations(initData(&iterations, 1, "iterations", "Number of times to do the relaxation"))
		, fixBorder(initData(&fixBorder, 0, "fix border", "If true, the points on the border will not move"))
	{
		addInput(input);
		addInput(factor);
		addInput(iterations);
		addInput(fixBorder);
		fixBorder.setWidget("checkbox");

		addOutput(output);
	}

	void update()
	{
		auto outMesh = output.getAccessor();
		outMesh.wref() = input.getValue();
		if(!outMesh->hasEdgesAroundPoint())
			outMesh->createEdgesAroundPointList();

		std::set<int> borderSet;
		if(fixBorder.getValue())
		{
			auto borderPts = outMesh->getPointsOnBorder();
			borderSet.insert(borderPts.begin(), borderPts.end());
		}

		int nbIter = iterations.getValue();
		int nbPts = outMesh->getNumberOfPoints();
		PReal fact = factor.getValue();
		for(int i=0; i<nbIter; ++i)
		{
			auto ptsCopy = outMesh->getPoints();
			for(int j=0; j<nbPts; ++j)
			{
				if(borderSet.find(j) != borderSet.end())
					continue;

				Point& pt = outMesh->getPoint(j);
				Point pt1 = ptsCopy[j];
				for(auto eid : outMesh->getEdgesAroundPoint(j))
				{
					Mesh::Edge e = outMesh->getEdge(eid);
					Mesh::PointID p2id = outMesh->getOtherPointInEdge(e, j);
					Point pt2 = ptsCopy[p2id];

					Point dir = pt2 - pt1;
					pt += dir * fact;
				}
			}
		}

		cleanDirty();
	}

protected:
	Data< Mesh > input, output;
	Data< PReal > factor;
	Data< int > iterations, fixBorder;
};

int ModifierMesh_EdgeRelaxationClass = RegisterObject<ModifierMesh_EdgeRelaxation>("Modifier/Mesh/Edge relaxation")
		.setDescription("Pull each point a little bit closer to its neighbors");

} // namespace Panda
