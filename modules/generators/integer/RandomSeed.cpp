#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include <modules/generators/integer/RandomSeed.h>

#include <random>
#include <panda/helper/Random.h>

namespace panda {

GeneratorInteger_RandomSeed::GeneratorInteger_RandomSeed(PandaDocument *doc)
	: PandaObject(doc)
	, dist(0, 10000)
	, seed(initData(&seed, 0, "seed", "Seed for the random numbers generator"))
	, value(initData(&value, 0, "value", "Value generated this time step"))
{
	addInput(&seed);

	addOutput(&value);

	seed.setWidget("seed");
	seed.setValue(helper::RandomGenerator::getRandomSeed(10000));

	connect(doc, SIGNAL(timeChanged()), this, SLOT(timeChanged()));

	reset();
}

void GeneratorInteger_RandomSeed::reset()
{
	gen.seed(seed.getValue());

	newValue();
}

void GeneratorInteger_RandomSeed::timeChanged()
{
	newValue();
}

void GeneratorInteger_RandomSeed::newValue()
{
	value.setValue(dist(gen));
	emit modified(this);
}

int GeneratorInteger_RandomSeedClass = RegisterObject<GeneratorInteger_RandomSeed>("Generator/Integer/Random seed each time step")
		.setName("Random seed").setDescription("Create a new number at each time step");

} // namespace Panda
