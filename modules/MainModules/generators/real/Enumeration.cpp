#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class GeneratorReals_Enumeration : public PandaObject
{
public:
	PANDA_CLASS(GeneratorReals_Enumeration, PandaObject)

	GeneratorReals_Enumeration(PandaDocument *doc)
		: PandaObject(doc)
		, nbNumbers(initData(10, "# numbers", "How many numbers to generate"))
		, start(initData((PReal)0.0, "start", "Start value of the enumeration"))
		, step(initData((PReal)1.0, "step", "Interval between 2 numbers"))
		, numbers(initData("numbers", "The list of numbers" ))
	{
		addInput(nbNumbers);
		addInput(start);
		addInput(step);

		addOutput(numbers);
	}

	void update()
	{
		auto valNumbers = numbers.getAccessor();
		int valNbNumbers = std::max(0, nbNumbers.getValue());
		valNumbers.resize(valNbNumbers);

		PReal valStart = start.getValue(), valStep = step.getValue();
		for(int i=0; i<valNbNumbers; ++i)
			valNumbers[i] = valStart + valStep * i;

		cleanDirty();
	}

protected:
	Data<int> nbNumbers;
	Data<PReal> start, step;
	Data< QVector<PReal> > numbers;
};

int GeneratorReals_EnumerationClass = RegisterObject<GeneratorReals_Enumeration>("Generator/Real/Enumeration").setDescription("Create a list of numbers based on an enumeration");

} // namespace Panda
