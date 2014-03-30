#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/helper/Perlin.h>
#include <panda/helper/Random.h>

namespace panda {

using types::Point;

class GeneratorPoints_Noise2d : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPoints_Noise2d, PandaObject)

	GeneratorPoints_Noise2d(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "The list of positions at which to compute the noise" ))
		, seed(initData(&seed, 0, "seed", "Seed for the random points generator"))
		, scale(initData(&scale, (PReal)0.001, "scale", "Input points are scaled by this value before computing the noise"))
		, output(initData(&output, "output", "The values of the noise at the given positions" ))
	{
		addInput(&input);
		addInput(&seed);
		addInput(&scale);

		addOutput(&output);

		seed.setWidget("seed");
		seed.setValue(helper::RandomGenerator::getRandomSeed(10000));
	}

	void update()
	{
		perlin.setSeed(seed.getValue());
		const auto& valInput = input.getValue();
		const PReal& valScale = scale.getValue();
		auto valOutput = output.getAccessor();
		int nb = valInput.size();
		valOutput.resize(nb);

		for(int i=0; i<nb; ++i)
			valOutput[i] = perlin.fBm(valInput[i] * valScale);

		cleanDirty();
	}

protected:
	helper::Perlin perlin;
	Data< QVector<Point> > input;
	Data<int> seed;
	Data<PReal> scale;
	Data< QVector<PReal> > output;
};

int GeneratorPoints_Noise2dClass = RegisterObject<GeneratorPoints_Noise2d>("Generator/Point/Noise 2d")
		.setDescription("Compute the fractional brownian motion at the given positions");

} // namespace Panda