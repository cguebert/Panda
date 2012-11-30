#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <helper/Random.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace panda {

class ModifierPoints_RandomDisplacement : public PandaObject
{
public:
	PANDA_CLASS(ModifierPoints_RandomDisplacement, PandaObject)

	ModifierPoints_RandomDisplacement(PandaDocument *doc)
		: PandaObject(doc)
		, inputPoints(initData(&inputPoints, "input", "List of points to move"))
		, outputPoints(initData(&outputPoints, "output", "List of moved points"))
		, minAngle(initData(&minAngle, 0.0, "minAngle", "Minimum angle of the direction in which to move the points"))
		, maxAngle(initData(&maxAngle, 360.0, "maxAngle", "Maximum angle of the direction in which to move the points"))
		, minDist(initData(&minDist, 0.0, "minDist", "Minimum distance over which to move the points"))
		, maxDist(initData(&maxDist, 100.0, "maxDist", "Maximum distance over which to move the points"))
		, seed(initData(&seed, 0, "seed", "Seed for the random number generator"))
	{
		addInput(&inputPoints);
		addInput(&minAngle);
		addInput(&maxAngle);
		addInput(&minDist);
		addInput(&maxDist);
		addInput(&seed);

		addOutput(&outputPoints);

		seed.setValue(rnd.seedRandom(10000));
	}

	void update()
	{
		const QVector<QPointF>& inPts = inputPoints.getValue();
		QVector<QPointF>& outPts = *outputPoints.beginEdit();
		int nb = inPts.size();
		outPts.resize(nb);

		rnd.seed(seed.getValue());
		double minA = minAngle.getValue(), maxA = maxAngle.getValue();
		double minD = minDist.getValue(), maxD = maxDist.getValue();

		for(int i=0; i<nb; ++i)
		{
			double a = rnd.random(minA, maxA) * M_PI / 180.0;
			double d = rnd.random(minD, maxD);
			QPointF disp(cos(a)*d, sin(a)*d);
			outPts[i] = inPts[i] + disp;
		}


		outputPoints.endEdit();
		this->cleanDirty();
	}

protected:
	helper::RandomGenerator rnd;
	Data< QVector<QPointF> > inputPoints, outputPoints;
	Data<double> minAngle, maxAngle, minDist, maxDist;
	Data<int> seed;
};

int ModifierPoints_RandomDisplacementClass = RegisterObject("Modifier/Point/Random displacement")
		.setClass<ModifierPoints_RandomDisplacement>()
		.setName("Random disp")
		.setDescription("Move points with a random displacement");

} // namespace Panda
