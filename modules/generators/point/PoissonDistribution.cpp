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
		, m_seed(initData(&m_seed, 0, "seed", "Seed for the random points generator"))
		, m_samples(initData(&m_samples, 20, "samples", "Number of samples to test (higher means tighter, but longer generation)"))
		, minimumDistance(initData(&minimumDistance, (PReal)50.0, "min distance", "Minimum distance between 2 points"))
		, m_points(initData(&m_points, "points", "The list of points"))
		, m_nbPoints(initData(&m_nbPoints, 10, "# points", "Number of points generated"))
		, m_area(initData(&m_area, "area", "Where to create the points. If null, the render area is used instead"))
	{
		addInput(&m_seed);
		addInput(&minimumDistance);
		addInput(&m_samples);
		addInput(&m_area);

		addOutput(&m_points);
		addOutput(&m_nbPoints);

		m_seed.setWidget("seed");
		m_seed.setValue(m_rnd.getRandomSeed(10000));
	}

	Point randomPointAround(const Point& point, PReal minDist, PReal maxDist)
	{
		PReal a = m_rnd.random() * 2 * M_PI;
		PReal r = m_rnd.random(minDist, maxDist);
		return point + r * Point(cos(a), sin(a));
	}

	void update()
	{
		m_rnd.seed(m_seed.getValue());
		auto valPoints = m_points.getAccessor();
		valPoints.clear();

		Rect area = m_area.getValue();
		if(area.empty())
		{
			QSize size = m_parentDocument->getRenderSize();
			area = Rect(0, 0, size.width()-1, size.height()-1);
		}

		m_grid.initGrid(area, minimumDistance.getValue() / sqrt(2.0));

		QList<Point> processList;

		Point firstPoint(m_rnd.random(area.left(), area.right())
						   , m_rnd.random(area.top(), area.bottom()));
		processList.push_back(firstPoint);
		valPoints.push_back(firstPoint);
		m_grid.addPoint(firstPoint);

		int rejectionLimit = m_samples.getValue();
		PReal minDist = minimumDistance.getValue();
		PReal maxDist = minDist * 2.0;

		while(!processList.empty())
		{
			int i = floor(m_rnd.random() * processList.size());
			Point pt = processList.at(i);
			processList.removeAt(i);
			for(i=0; i<rejectionLimit; ++i)
			{
				Point nPt = randomPointAround(pt, minDist, maxDist);
				if(area.contains(nPt) && !m_grid.testNeighbor(nPt, minDist))
				{
					processList.push_back(nPt);
					valPoints.push_back(nPt);
					m_grid.addPoint(nPt);
				}
			}
		}

		m_nbPoints.setValue(valPoints.size());
		m_grid.clear();
		cleanDirty();
	}

protected:
	helper::RandomGenerator m_rnd;
	helper::PointsGrid m_grid;

	Data<int> m_seed, m_samples;
	Data<PReal> minimumDistance;
	Data< QVector<Point> > m_points;
	Data<int> m_nbPoints;
	Data<Rect> m_area;
};

int GeneratorPoints_PoissonClass = RegisterObject<GeneratorPoints_Poisson>("Generator/Point/Poisson distribution").setDescription("Generate a list of random points, but with a minimal distance between neighbors");

} // namespace Panda
