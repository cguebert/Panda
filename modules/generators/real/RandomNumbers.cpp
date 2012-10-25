#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <helper/Random.h>

namespace panda {

class GeneratorReals_Random : public PandaObject
{
public:
    GeneratorReals_Random(PandaDocument *doc)
        : PandaObject(doc)
        , nbNumbers(initData(&nbNumbers, 10, "# numbers", "How many numbers to generate"))
        , seed(initData(&seed, 0, "seed", "Seed for the random points generator"))
        , numMin(initData(&numMin, 0.0, "minimum", "Minimum limit of the numbers"))
        , numMax(initData(&numMax, 1.0, "maximum", "Maximum limit of the numbers"))
        , numbers(initData(&numbers, "numbers", "The list of numbers" ))
    {
        addInput(&nbNumbers);
        addInput(&seed);
        addInput(&numMin);
        addInput(&numMax);

        addOutput(&numbers);

        seed.setValue(rnd.seedRandom(10000));
    }

    void update()
    {
        rnd.seed(seed.getValue());
        QVector<double>& valNumbers = *numbers.beginEdit();
        int valNbNumbers = nbNumbers.getValue();
        valNumbers.resize(valNbNumbers);

        double min = numMin.getValue(), max = numMax.getValue();
        for(int i=0; i<valNbNumbers; ++i)
            valNumbers[i] = rnd.random(min, max);

        numbers.endEdit();
        this->cleanDirty();
    }

protected:
    helper::RandomGenerator rnd;
    Data<int> nbNumbers, seed;
    Data<double> numMin, numMax;
    Data< QVector<double> > numbers;
};

int GeneratorReals_RandomClass = RegisterObject("Generator/Real/Random").setClass<GeneratorReals_Random>().setName("Rnd numbers").setDescription("Generate a list of random numbers");

} // namespace Panda
