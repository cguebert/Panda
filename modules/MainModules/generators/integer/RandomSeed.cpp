#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include <random>
#include <panda/helper/Random.h>

namespace panda {

class GeneratorInteger_RandomSeed : public PandaObject
{
public:
	PANDA_CLASS(GeneratorInteger_RandomSeed, PandaObject)

	GeneratorInteger_RandomSeed(PandaDocument *doc)
		: PandaObject(doc)
		, dist(0, 10000)
		, seed(initData(0, "seed", "Seed for the random numbers generator"))
		, value(initData(0, "value", "Value generated this time step"))
		, prevSeed(-1)
	{
		addInput(seed);

		addOutput(value);

		seed.setWidget("seed");
		seed.setValue(helper::RandomGenerator::getRandomSeed(10000));

		BaseData* docTime = doc->getData("time");
		if(docTime)
			addInput(*docTime);

		reset();
	}

	void reset()
	{
		prevSeed = -1;
		setDirtyValue(this);
	}

	void update()
	{
		int seedV = seed.getValue();
		if(seedV != prevSeed)
		{
			gen.seed(seed.getValue());
			prevSeed = seedV;
		}
		value.setValue(dist(gen));

		cleanDirty();
	}

	void load(QDomElement &elem)
	{
		PandaObject::load(elem);

		reset();
	}

protected:
	std::mt19937 gen;
	std::uniform_int_distribution<int> dist;
	Data<int> seed, value;
	int prevSeed;
};

int GeneratorInteger_RandomSeedClass = RegisterObject<GeneratorInteger_RandomSeed>("Generator/Integer/Random seed each time step")
		.setName("Random seed").setDescription("Create a new number at each time step");

} // namespace Panda
