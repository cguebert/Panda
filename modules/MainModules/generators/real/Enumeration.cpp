#include <panda/object/ObjectFactory.h>

#include <algorithm>

namespace panda {

class GeneratorReals_Enumeration : public PandaObject
{
public:
	PANDA_CLASS(GeneratorReals_Enumeration, PandaObject)

	GeneratorReals_Enumeration(PandaDocument *doc)
		: PandaObject(doc)
		, nbNumbers(initData(10, "# numbers", "How many numbers to generate"))
		, start(initData((float)0.0, "start", "Start value of the enumeration"))
		, step(initData((float)1.0, "step", "Interval between 2 numbers"))
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

		float valStart = start.getValue(), valStep = step.getValue();
		for(int i=0; i<valNbNumbers; ++i)
			valNumbers[i] = valStart + valStep * i;
	}

protected:
	Data<int> nbNumbers;
	Data<float> start, step;
	Data< std::vector<float> > numbers;
};

int GeneratorReals_EnumerationClass = RegisterObject<GeneratorReals_Enumeration>("Generator/Real/Enumeration").setDescription("Create a list of numbers based on an enumeration");

} // namespace Panda
