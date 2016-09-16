#include <panda/document/RenderedDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/helper/Random.h>
#include <panda/helper/PointsGrid.h>

namespace panda {

using types::Point;
using types::Rect;

class GeneratorPoints_Poisson : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPoints_Poisson, PandaObject)

	GeneratorPoints_Poisson(PandaDocument *doc)
		: PandaObject(doc)
		, m_seed(initData(0, "seed", "Seed for the random points generator"))
		, m_samples(initData(20, "samples", "Number of samples to test (higher means tighter, but longer generation)"))
		, minimumDistance(initData((float)50.0, "min distance", "Minimum distance between 2 points"))
		, m_points(initData("points", "The list of points"))
		, m_nbPoints(initData(10, "# points", "Number of points generated"))
		, m_area(initData("area", "Where to create the points. If null, the render area is used instead"))
	{
		addInput(m_seed);
		addInput(minimumDistance);
		addInput(m_samples);
		addInput(m_area);

		addOutput(m_points);
		addOutput(m_nbPoints);

		m_seed.setWidget("seed");
		m_seed.setValue(m_rnd.getRandomSeed(10000));

		// The output is dependent on the document's size
		BaseData* data = doc->getData("render size");
		if(data) addInput(*data);
	}

	Point randomPointAround(const Point& point, float minDist, float maxDist)
	{
		float a = m_rnd.random() * 2 * static_cast<float>(M_PI);
		float r = m_rnd.random(minDist, maxDist);
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
			auto docPtr = dynamic_cast<RenderedDocument*>(parentDocument());
			if (!docPtr)
				return; // Empty area, cannot compute
			auto size = docPtr->getRenderSize();
			area = Rect(0, 0, static_cast<float>(size.width()-1), static_cast<float>(size.height()-1));
		}

		m_grid.initGrid(area, minimumDistance.getValue() / sqrt(2.f));

		std::vector<Point> processList;

		Point firstPoint(m_rnd.random(area.left(), area.right())
						   , m_rnd.random(area.top(), area.bottom()));
		processList.push_back(firstPoint);
		valPoints.push_back(firstPoint);
		m_grid.addPoint(firstPoint);

		int rejectionLimit = m_samples.getValue();
		float minDist = minimumDistance.getValue();
		float maxDist = minDist * 2.f;

		while(!processList.empty())
		{
			int i = static_cast<int>(floor(m_rnd.random() * static_cast<float>(processList.size())));
			Point pt = processList.at(i);
			processList.erase(processList.begin() + i);
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
	}

protected:
	helper::RandomGenerator m_rnd;
	helper::PointsGrid m_grid;

	Data<int> m_seed, m_samples;
	Data<float> minimumDistance;
	Data< std::vector<Point> > m_points;
	Data<int> m_nbPoints;
	Data<Rect> m_area;
};

int GeneratorPoints_PoissonClass = RegisterObject<GeneratorPoints_Poisson>("Generator/Point/Poisson distribution").setDescription("Generate a list of random points, but with a minimal distance between neighbors");

} // namespace Panda
