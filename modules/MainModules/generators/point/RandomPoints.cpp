#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Point.h>
#include <panda/types/Rect.h>
#include <panda/helper/Random.h>

namespace panda {

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

		Rect area = m_area.getValue();
		if(area.empty())
		{
			auto size = m_parentDocument->getRenderSize();
			area = Rect(0, 0, static_cast<float>(size.width()-1), static_cast<float>(size.height()-1));
		}
		float w = area.width(), h = area.height();
		Point origin = area.topLeft();

		for(int i=0; i<valNbPoints; ++i)
			valPoints.push_back(origin + Point(m_rnd.random() * w, m_rnd.random() * h));

		cleanDirty();
	}

protected:
	helper::RandomGenerator m_rnd;
	Data<int> m_nbPoints, m_seed;
	Data< std::vector<Point> > m_points;
	Data<Rect> m_area;
};

int GeneratorPoints_RandomClass = RegisterObject<GeneratorPoints_Random>("Generator/Point/Random").setName("Random points").setDescription("Generate a list of random points");

} // namespace Panda
