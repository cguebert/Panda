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
		, hsvMode(initData(&hsvMode, 0, "HSV", "Use HSV instead of RGB"))
		, colorMin(initData(&colorMin, QColor(0, 0, 0), "min color", "Color defining the minimum possible value of each component"))
		, colorMax(initData(&colorMax, QColor(255, 255, 255), "max color", "Color defining the maximum possible value of each component"))
		, colors(initData(&colors, "colors", "Randomly generated colors"))
	{
		addInput(&nbColors);
		addInput(&seed);
		addInput(&hsvMode);
		addInput(&colorMin);
		addInput(&colorMax);

		hsvMode.setWidget("checkbox");

		addOutput(&colors);

		seed.setWidget("seed");
		seed.setValue(rnd.getRandomSeed(10000));
	}

	void update()
	{
		rnd.seed(seed.getValue());
		int nb = nbColors.getValue();
		auto list = colors.getAccessor();
		list.resize(nb);

		const auto& cMin = colorMin.getValue();
		const auto& cMax = colorMax.getValue();

		bool useHSV = hsvMode.getValue();

		int minA, minR, minG, minB, maxA, maxR, maxG, maxB;
		if(useHSV)
		{
			cMin.getHsv(&minR, &minG, &minB, &minA);
			cMax.getHsv(&maxR, &maxG, &maxB, &maxA);
		}
		else
		{
			cMin.getRgb(&minR, &minG, &minB, &minA);
			cMax.getRgb(&maxR, &maxG, &maxB, &maxA);
		}
		if(minR > maxR) std::swap(minR, maxR);
		if(minG > maxG) std::swap(minG, maxG);
		if(minB > maxB) std::swap(minB, maxB);
		if(minA > maxA) std::swap(minA, maxA);

		if(useHSV)
		{
			for(int i=0; i<nb; ++i)
			{
				list[i] = QColor::fromHsv(rnd.randomInt(minR, maxR),
										  rnd.randomInt(minG, maxG),
										  rnd.randomInt(minB, maxB),
										  rnd.randomInt(minA, maxA));
			}
		}
		else
		{
			for(int i=0; i<nb; ++i)
			{
				list[i] = QColor(rnd.randomInt(minR, maxR),
								 rnd.randomInt(minG, maxG),
								 rnd.randomInt(minB, maxB),
								 rnd.randomInt(minA, maxA));
			}
		}

		cleanDirty();
	}

protected:
	helper::RandomGenerator rnd;
	Data< int > nbColors, seed, hsvMode;
	Data< QColor > colorMin, colorMax;
	Data< QVector<QColor> > colors;
};

int GeneratorColors_RandomClass = RegisterObject<GeneratorColors_Random>("Generator/Color/Random").setName("Random colors").setDescription("Generate colors by choosing randomly each component");

} // namespace Panda
