#include <panda/document/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Point.h>
#include <panda/types/Rect.h>
#include <panda/graphics/Image.h>
#include <panda/helper/Random.h>

#include <algorithm>

namespace panda {

using helper::RandomGenerator;
using types::ImageWrapper;
using types::Point;
using types::Rect;

class GeneratorPoints_Random : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPoints_Random, PandaObject)

	GeneratorPoints_Random(PandaDocument *doc)
		: PandaObject(doc)
		, m_nbPoints(initData(10, "# points", "Number of points to generate"))
		, m_seed(initData(0, "seed", "Seed for the random points generator"))
		, m_points(initData("points", "The list of points" ))
		, m_area(initData("area", "Where to create the points. If null, the render area is used instead"))
	{
		addInput(m_nbPoints);
		addInput(m_seed);
		addInput(m_area);

		addOutput(m_points);

		m_seed.setWidget("seed");
		m_seed.setValue(m_rnd.getRandomSeed(10000));

		// The output is dependent on the document's size
		BaseData* data = doc->getData("render size");
		if(data) addInput(*data);
	}

	void update()
	{
		m_rnd.seed(m_seed.getValue());
		auto valPoints = m_points.getAccessor();
		int valNbPoints = m_nbPoints.getValue();
		valPoints.clear();
		valPoints.reserve(valNbPoints);

		Rect area = m_area.getValue();
		if(area.empty())
		{
			auto size = parentDocument()->getRenderSize();
			area = Rect(0, 0, static_cast<float>(size.width()-1), static_cast<float>(size.height()-1));
		}
		float w = area.width(), h = area.height();
		Point origin = area.topLeft();

		for(int i=0; i<valNbPoints; ++i)
			valPoints.push_back(origin + Point(m_rnd.random() * w, m_rnd.random() * h));
	}

protected:
	RandomGenerator m_rnd;
	Data<int> m_nbPoints, m_seed;
	Data< std::vector<Point> > m_points;
	Data<Rect> m_area;
};

int GeneratorPoints_RandomClass = RegisterObject<GeneratorPoints_Random>("Generator/Point/Random").setName("Random points").setDescription("Generate a list of random points");

//****************************************************************************//

class GeneratorPoints_RandomWithDensity : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPoints_RandomWithDensity, PandaObject)

	GeneratorPoints_RandomWithDensity(PandaDocument *doc)
		: PandaObject(doc)
		, m_nbPoints(initData(10, "# points", "Number of points to generate"))
		, m_seed(initData(0, "seed", "Seed for the random points generator"))
		, m_points(initData("points", "The list of points" ))
		, m_densityMap(initData("density", "Density map"))
	{
		addInput(m_nbPoints);
		addInput(m_seed);
		addInput(m_densityMap);

		addOutput(m_points);

		m_seed.setWidget("seed");
		m_seed.setValue(RandomGenerator::getRandomSeed(10000));

		// The output is dependent on the document's size
		BaseData* data = doc->getData("render size");
		if(data) addInput(*data);
	}

	void prepareDensityMap()
	{
		m_accumulation.clear();
		m_densityCounter = m_densityMap.getCounter();
		const auto& densityMap = m_densityMap.getValue();
		if (densityMap.isNull())
			return;

		const auto& img = densityMap.getImage();
		int w = img.width(), h = img.height();
		int nb = w * h;
		if (!nb)
			return;
		m_width = w;

		m_accumulation.resize(nb);

		const auto* data = img.data();
		m_accumulation[0] = graphics::gray(data);
		data += 4;

		for (int i = 1; i < nb; ++i)
		{
			m_accumulation[i] = m_accumulation[i - 1] + graphics::gray(data);
			data += 4;
		}
	}

	void update()
	{
		auto points = m_points.getAccessor();
		int nbPoints = m_nbPoints.getValue();
		points.clear();

		if (m_densityCounter != m_densityMap.getCounter())
			prepareDensityMap();

		if (!m_accumulation.empty())
		{
			auto maxBound = m_accumulation.back();
			std::mt19937 gen(m_seed.getValue());
			std::uniform_int_distribution<std::uint64_t> dist(0, maxBound);

			auto itBeg = m_accumulation.begin(), itEnd = m_accumulation.end();
			auto& refPts = points.wref();
			refPts.reserve(nbPoints);
			for (int i = 0; i < nbPoints; ++i)
			{
				auto rnd = dist(gen);
				auto index = std::lower_bound(itBeg, itEnd, rnd) - itBeg;
				refPts.emplace_back(static_cast<float>(index % m_width), static_cast<float>(index / m_width));
			}
		}
	}

protected:
	Data<int> m_nbPoints, m_seed;
	Data< std::vector<Point> > m_points;
	Data<ImageWrapper> m_densityMap;

	int m_densityCounter = -1, m_width = -1;
	std::vector<std::uint64_t> m_accumulation;
};

int GeneratorPoints_RandomWithDensityClass = RegisterObject<GeneratorPoints_RandomWithDensity>("Generator/Point/Random with density map").setName("Points from density").setDescription("Generate a list of random points");

} // namespace Panda
