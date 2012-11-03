#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QVector>
#include <math.h>

namespace panda {

class DoubleMath_SmoothStep : public PandaObject
{
public:
	DoubleMath_SmoothStep(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData(&inputA, "min", "Start of the smooth step"))
		, inputB(initData(&inputB, "max", "End of the smooth step"))
		, inputV(initData(&inputV, "input", "Input of the smooth step"))
		, result(initData(&result, "result", "Result of the smooth step"))
	{
		addInput(&inputA);
		addInput(&inputB);
		addInput(&inputV);

		addOutput(&result);
	}

	double smoothStep(double a, double b, double x)
	{
		if (x < a)	return 0;
		if (x >= b)	return 1;
		x = (x - a)/(b - a);
		return (x*x * (3 - 2*x));
	}

	void update()
	{
		const QVector<double>	&valInA = inputA.getValue(),
								&valInB = inputB.getValue(),
								&valInV = inputV.getValue();
		QVector<double> &valOut = *result.beginEdit();
		valOut.clear();

		int nbA = valInA.size(), nbB = valInB.size(), nbV = valInV.size();
		if(nbA && nbB && nbV)
		{
			int nb = nbV;
			if(nbV > 1)
			{
				if(nbA != nbV) nbA = 1;
				if(nbB != nbV) nbB = 1;
			}
			else
			{
				if(nbB > nbA && nbA > 1)		nbB = nbA;
				else if(nbA > nbB && nbB > 1)	nbA = nbB;
				nb = qMax(nbA, nbB);
			}
			valOut.resize(nb);

			for(int i=0; i<nb; ++i)
				valOut[i] = smoothStep(valInA[i%nbA], valInB[i%nbB], valInV[i%nbV]);

			result.endEdit();
		}

		this->cleanDirty();
	}

protected:
	Data< QVector<double> > inputA, inputB, inputV, result;
};

int DoubleMath_SmoothStepClass = RegisterObject("Math/Real/Smooth Step").setClass<DoubleMath_SmoothStep>().setDescription("Compute the smooth step of the input between min and max");

//*************************************************************************//

class DoubleMath_RiseCount : public PandaObject
{
public:
	DoubleMath_RiseCount(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "Value to observe"))
		, count(initData(&count, "count", "Count of rise transitions"))
	{
		addInput(&input);
		addOutput(&count);
	}

	void update()
	{
		const QVector<double> &valIn = input.getValue();
		QVector<int> &valOut = *count.beginEdit();

		int size = valIn.size();
		valOut.resize(size);
		prevValues.resize(size);

		for(int i=0; i<size; ++i)
		{
			if(valIn[i] > prevValues[i])
				++valOut[i];
			prevValues[i] = valIn[i];
		}

		count.endEdit();
		this->cleanDirty();
	}

	void reset()
	{
		PandaObject::reset();

		const QVector<double> &valIn = input.getValue();
		QVector<int> &valOut = *count.beginEdit();
		int size = valIn.size();
		prevValues.resize(size);
		for(int i=0; i<size; ++i)
		{
			prevValues[i] = valIn[i];
			valOut[i] = 0;
		}
	}

protected:
	QVector<double> prevValues;
	Data< QVector<double> > input;
	Data< QVector<int> > count;
};

int DoubleMath_RiseCountClass = RegisterObject("Math/Real/Rise count").setClass<DoubleMath_RiseCount>().setDescription("Count the number of rise transitions of a value");

//*************************************************************************//

class DoubleMath_FallCount : public PandaObject
{
public:
	DoubleMath_FallCount(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "Value to observe"))
		, count(initData(&count, "count", "Count of fall transitions"))
	{
		addInput(&input);
		addOutput(&count);
	}

	void update()
	{
		const QVector<double> &valIn = input.getValue();
		QVector<int> &valOut = *count.beginEdit();

		int size = valIn.size();
		valOut.resize(size);
		prevValues.resize(size);

		for(int i=0; i<size; ++i)
		{
			if(valIn[i] < prevValues[i])
				++valOut[i];
			prevValues[i] = valIn[i];
		}

		count.endEdit();
		this->cleanDirty();
	}

	void reset()
	{
		PandaObject::reset();

		const QVector<double> &valIn = input.getValue();
		QVector<int> &valOut = *count.beginEdit();
		int size = valIn.size();
		prevValues.resize(size);
		for(int i=0; i<size; ++i)
		{
			prevValues[i] = valIn[i];
			valOut[i] = 0;
		}
	}

protected:
	QVector<double> prevValues;
	Data< QVector<double> > input;
	Data< QVector<int> > count;
};

int DoubleMath_FallCountClass = RegisterObject("Math/Real/Fall count").setClass<DoubleMath_FallCount>().setDescription("Count the number of fall transitions of a value");

} // namespace Panda


