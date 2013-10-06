#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QPointF>
#include <QList>
#include <panda/helper/Random.h>
#include <panda/helper/PointsGrid.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace panda {

class GeneratorPoints_Poisson : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPoints_Poisson, PandaObject)

	GeneratorPoints_Poisson(PandaDocument *doc)
		: PandaObject(doc)
		, nbPoints(initData(&nbPoints, 10, "# points", "Number of points generated"))
		, seed(initData(&seed, 0, "seed", "Seed for the random points generator"))
		, samples(initData(&samples, 20, "samples", "Number of samples to test (higher means tighter, but longer generation)"))
		, minimumDistance(initData(&minimumDistance, 50.0, "min distance", "Minimum distance between 2 points"))
		, points(initData(&points, "points", "The list of points" ))
	{
		addInput(&seed);
		addInput(&minimumDistance);
		addInput(&samples);

		addOutput(&nbPoints);
		addOutput(&points);

		seed.setValue(rnd.seedRandom(10000));
	}

	QPointF randomPointAround(const QPointF& point, double minDist, double maxDist)
	{
		double a = rnd.random() * 2 * M_PI;
		double r = rnd.random(minDist, maxDist);
		return QPointF(point.x() + r*cos(a), point.y() + r*sin(a));
	}

	void update()
	{
		rnd.seed(seed.getValue());
		auto valPoints = points.getAccessor();
		valPoints.clear();
		QSize size = parentDocument->getRenderSize();
		QRectF area = QRectF(0, 0, size.width()-1, size.height()-1);
		grid.initGrid(area, minimumDistance.getValue() / sqrt(2.0));

		QList<QPointF> processList;

		QPointF firstPoint(rnd.random(area.left(), area.right())
						   , rnd.random(area.top(), area.bottom()));
		processList.push_back(firstPoint);
		valPoints.push_back(firstPoint);
		grid.addPoint(firstPoint);

		int rejectionLimit = samples.getValue();
		double minDist = minimumDistance.getValue();
		double maxDist = minDist * 2.0;

		while(!processList.empty())
		{
			int i = floor(rnd.random() * processList.size());
			QPointF pt = processList.at(i);
			processList.removeAt(i);
			for(i=0; i<rejectionLimit; ++i)
			{
				QPointF nPt = randomPointAround(pt, minDist, maxDist);
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
		this->cleanDirty();
	}

protected:
	helper::RandomGenerator rnd;
	helper::PointsGrid grid;

	Data<int> nbPoints, seed, samples;
	Data<double> minimumDistance;
	Data< QVector<QPointF> > points;
};

int GeneratorPoints_PoissonClass = RegisterObject<GeneratorPoints_Poisson>("Generator/Point/Poisson distribution").setName("Poisson dist.").setDescription("Generate a list of random points, but with a minimal distance between neighbors");

} // namespace Panda
