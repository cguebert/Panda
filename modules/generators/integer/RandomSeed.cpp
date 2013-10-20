#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/helper/Random.h>

namespace panda {

class GeneratorReals_Random : public PandaObject
{
public:
	PANDA_CLASS(GeneratorReals_Random, PandaObject)

	GeneratorReals_Random(PandaDocument *doc)
		: PandaObject(doc)
		, nbNumbers(initData(&nbNumbers, 10, "# numbers", "How many numbers to generate"))
		, seed(initData(&seed, 0, "seed", "Seed for the random numbers generator"))
		, numMin(initData(&numMin, 0.0, "minimum", "Minimum limit of the numbers"))
		, numMax(initData(&numMax, 1.0, "maximum", "Maximum limit of the numbers"))
		, numbers(initData(&numbers, "numbers", "The list of numbers" ))
	{
		addInput(&nbNumbers);
		addInput(&seed);
		addInput(&numMin);
		addInput(&numMax);

		addOutput(&numbers);

		seed.setWidget("seed");
		seed.setValue(rnd.getRandomSeed(10000));
	}

	void update()
	{
		rnd.seed(seed.getValue());
		auto valNumbers = numbers.getAccessor();
		int valNbNumbers = nbNumbers.getValue();
		valNumbers.resize(valNbNumbers);

		double min = numMin.getValue(), max = numMax.getValue();
		for(int i=0; i<valNbNumbers; ++i)
			valNumbers[i] = rnd.random(min, max);

		this->cleanDirty();
	}

protected:
	helper::RandomGenerator rnd;
	Data<int> nbNumbers, seed;
	Data<double> numMin, numMax;
	Data< QVector<double> > numbers;
};

int GeneratorReals_RandomClass = RegisterObject<GeneratorReals_Random>("Generator/Real/Random").setName("Random numbers").setDescription("Generate a list of random numbers");

} // namespace Panda