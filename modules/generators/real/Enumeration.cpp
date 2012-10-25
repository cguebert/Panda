#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
namespace panda {

class GeneratorReals_Enumeration : public PandaObject
{
public:
	GeneratorReals_Enumeration(PandaDocument *doc)
		: PandaObject(doc)
		, nbNumbers(initData(&nbNumbers, 10, "# numbers", "How many numbers to generate"))
		, start(initData(&start, 0.0, "start", "Start value of the enumeration"))
		, step(initData(&step, 1.0, "step", "Interval between 2 numbers"))
		, numbers(initData(&numbers, "numbers", "The list of numbers" ))
	{
		addInput(&nbNumbers);
		addInput(&start);
		addInput(&step);

		addOutput(&numbers);
	}

	void update()
	{
		QVector<double>& valNumbers = *numbers.beginEdit();
		int valNbNumbers = nbNumbers.getValue();
		valNumbers.resize(valNbNumbers);

		double valStart = start.getValue(), valStep = step.getValue();
		for(int i=0; i<valNbNumbers; ++i)
			valNumbers[i] = valStart + valStep * i;

		numbers.endEdit();
		this->cleanDirty();
	}

protected:
	Data<int> nbNumbers;
	Data<double> start, step;
	Data< QVector<double> > numbers;
};

int GeneratorReals_EnumerationClass = RegisterObject("Generator/Real/Enumeration").setClass<GeneratorReals_Enumeration>().setDescription("Create a list of numbers based on an enumeration");

} // namespace Panda
