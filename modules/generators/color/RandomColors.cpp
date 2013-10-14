#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/helper/Random.h>

namespace panda {

class GeneratorColors_Random : public PandaObject
{
public:
	PANDA_CLASS(GeneratorColors_Random, PandaObject)

	GeneratorColors_Random(PandaDocument *doc)
		: PandaObject(doc)
		, nbColors(initData(&nbColors, 1, "number", "Number of colors to generate"))
		, seed(initData(&seed, "seed", "Seed for the random points generator"))
		, colorMin(initData(&colorMin, QColor(0, 0, 0), "min color", "Color defining the minimum possible value of each component"))
		, colorMax(initData(&colorMax, QColor(255, 255, 255), "max color", "Color defining the maximum possible value of each component"))
		, colors(initData(&colors, "colors", "Randomly generated colors"))
	{
		addInput(&nbColors);
		addInput(&seed);
		addInput(&colorMin);
		addInput(&colorMax);

		addOutput(&colors);

		seed.setValue(rnd.seedRandom(10000));
	}

	void update()
	{
		rnd.seed(seed.getValue());
		int nb = nbColors.getValue();
		auto list = colors.getAccessor();
		list.resize(nb);

		const auto& cMin = colorMin.getValue();
		const auto& cMax = colorMax.getValue();

		int minA, minR, minG, minB, maxA, maxR, maxG, maxB;
		cMin.getRgb(&minR, &minG, &minB, &minA);
		cMax.getRgb(&maxR, &maxG, &maxB, &maxA);

		for(int i=0; i<nb; ++i)
		{
			list[i] = QColor(rnd.randomInt(minR, maxR),
							 rnd.randomInt(minG, maxG),
							 rnd.randomInt(minB, maxB),
							 rnd.randomInt(minA, maxA));
		}

		this->cleanDirty();
	}

protected:
	helper::RandomGenerator rnd;
	Data< int > nbColors, seed;
	Data< QColor > colorMin, colorMax;
	Data< QVector<QColor> > colors;
};

int GeneratorColors_RandomClass = RegisterObject<GeneratorColors_Random>("Generator/Color/Random").setName("Random colors").setDescription("Generate colors by choosing randomly each component");

} // namespace Panda
