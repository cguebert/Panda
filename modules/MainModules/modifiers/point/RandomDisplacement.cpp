#include <panda/object/ObjectFactory.h>
#include <panda/helper/Random.h>
#include <panda/types/Point.h>

#include <cmath>

namespace panda {

using types::Point;

class ModifierPoints_RandomDisplacement : public PandaObject
{
public:
	PANDA_CLASS(ModifierPoints_RandomDisplacement, PandaObject)

	ModifierPoints_RandomDisplacement(PandaDocument *doc)
		: PandaObject(doc)
		, inputPoints(initData("input", "List of points to move"))
		, outputPoints(initData("output", "List of moved points"))
		, minAngle(initData((float)0.0, "minAngle", "Minimum angle of the direction in which to move the points"))
		, maxAngle(initData((float)360.0, "maxAngle", "Maximum angle of the direction in which to move the points"))
		, minDist(initData((float)0.0, "minDist", "Minimum distance over which to move the points"))
		, maxDist(initData((float)100.0, "maxDist", "Maximum distance over which to move the points"))
		, seed(initData(0, "seed", "Seed for the random number generator"))
	{
		addInput(inputPoints);
		addInput(minAngle);
		addInput(maxAngle);
		addInput(minDist);
		addInput(maxDist);
		addInput(seed);

		addOutput(outputPoints);

		seed.setWidget("seed");
		seed.setValue(rnd.getRandomSeed(10000));
	}

	void update()
	{
		const std::vector<Point>& inPts = inputPoints.getValue();
		auto outPts = outputPoints.getAccessor();
		int nb = inPts.size();
		outPts.resize(nb);

		rnd.seed(seed.getValue());
		float minA = minAngle.getValue(), maxA = maxAngle.getValue();
		float minD = minDist.getValue(), maxD = maxDist.getValue();

		const float pi180 = static_cast<float>(M_PI) / 180;
		for(int i=0; i<nb; ++i)
		{
			float a = rnd.random(minA, maxA) * pi180;
			float d = rnd.random(minD, maxD);
			Point disp(cos(a)*d, sin(a)*d);
			outPts[i] = inPts[i] + disp;
		}
	}

protected:
	helper::RandomGenerator rnd;
	Data< std::vector<Point> > inputPoints, outputPoints;
	Data<float> minAngle, maxAngle, minDist, maxDist;
	Data<int> seed;
};

int ModifierPoints_RandomDisplacementClass = RegisterObject<ModifierPoints_RandomDisplacement>("Modifier/Point/Random displacement")
		.setDescription("Move points with a random displacement");

} // namespace Panda
