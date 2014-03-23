#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/helper/Random.h>
#include <panda/helper/PointsGrid.h>

#include <cmath>

namespace panda {

using types::Point;
using types::Rect;

class GeneratorPoints_Poisson : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPoints_Poisson, PandaObject)

	GeneratorPoints_Poisson(PandaDocument *doc)
		: PandaObject(doc)
		, seed(initData(&seed, 0, "seed", "Seed for the random points generator"))
		, samples(initData(&samples, 20, "samples", "Number of samples to test (higher means tighter, but longer generation)"))
		, minimumDistance(initData(&minimumDistance, (PReal)50.0, "min distance", "Minimum distance between 2 points"))
		, points(initData(&points, "points", "The list of points"))
		, nbPoints(initData(&nbPoints, 10, "# points", "Number of points generated"))
	{
		addInput(&seed);
		addInput(&minimumDistance);
		addInput(&samples);

		addOutput(&points);
		addOutput(&nbPoints);

		seed.setWidget("seed");
		seed.setValue(rnd.getRandomSeed(10000));
	}

	Point randomPointAround(const Point& point, PReal minDist, PReal maxDist)
	{
		PReal a = rnd.random() * 2 * M_PI;
		PReal r = rnd.random(minDist, maxDist);
		return point + r * Point(cos(a), sin(a));
	}

	void update()
	{
		rnd.seed(seed.getValue());
		auto valPoints = points.getAccessor();
		valPoints.clear();
		QSize size = parentDocument->getRenderSize();
		Rect area = Rect(0, 0, size.width()-1, size.height()-1);
		grid.initGrid(area, minimumDistance.getValue() / sqrt(2.0));

		QList<Point> processList;

		Point firstPoint(rnd.random(area.left(), area.right())
						   , rnd.random(area.top(), area.bottom()));
		processList.push_back(firstPoint);
		valPoints.push_back(firstPoint);
		grid.addPoint(firstPoint);

		int rejectionLimit = samples.getValue();
		PReal minDist = minimumDistance.getValue();
		PReal maxDist = minDist * 2.0;

		while(!processList.empty())
		{
			int i = floor(rnd.random() * processList.size());
			Point pt = processList.at(i);
			processList.removeAt(i);
			for(i=0; i<rejectionLimit; ++i)
			{
				Point nPt = randomPointAround(pt, minDist, maxDist);
				if(area.contains(nPt) && !grid.testNeighbor(nPt, minDist))
				{
					processList.push_back(nPt);
					valPoints.push_back(nPt);
					grid.addPoint(nPt);
				}
			}
		}

		nbPoints.setValue(valPoints.size());
		grid.clear();
		cleanDirty();
	}

protected:
	helper::RandomGenerator rnd;
	helper::PointsGrid grid;

	Data<int> seed, samples;
	Data<PReal> minimumDistance;
	Data< QVector<Point> > points;
	Data<int> nbPoints;
};

int GeneratorPoints_PoissonClass = RegisterObject<GeneratorPoints_Poisson>("Generator/Point/Poisson distribution").setDescription("Generate a list of random points, but with a minimal distance between neighbors");

} // namespace Panda
