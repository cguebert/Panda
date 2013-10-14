#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QPointF>
#include <panda/helper/Random.h>

namespace panda {

class GeneratorPoints_Random : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPoints_Random, PandaObject)

	GeneratorPoints_Random(PandaDocument *doc)
		: PandaObject(doc)
		, document(doc)
		, nbPoints(initData(&nbPoints, 10, "# points", "Number of points to generate"))
		, seed(initData(&seed, 0, "seed", "Seed for the random points generator"))
		, points(initData(&points, "points", "The list of points" ))
	{
		addInput(&nbPoints);
		addInput(&seed);

		addOutput(&points);

		seed.setValue(rnd.seedRandom(10000));

		// The output is dependent on the document's size
		BaseData* data = doc->getData("render size");
		if(data) addInput(data);
	}

	void update()
	{
		rnd.seed(seed.getValue());
		auto valPoints = points.getAccessor();
		int valNbPoints = nbPoints.getValue();
		valPoints.clear();
		QSize size = document->getRenderSize();

		for(int i=0; i<valNbPoints; ++i)
			valPoints.push_back(QPointF(rnd.random()*size.width(), rnd.random()*size.height()));

		this->cleanDirty();
	}

protected:
	helper::RandomGenerator rnd;
	PandaDocument *document;
	Data<int> nbPoints, seed;
	Data< QVector<QPointF> > points;
};

int GeneratorPoints_RandomClass = RegisterObject<GeneratorPoints_Random>("Generator/Point/Random").setName("Random points").setDescription("Generate a list of random points");

} // namespace Panda
