#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QVector>
#include <cmath>

/* Rules when there are many input parameters :
  (1 list of input values + X lists of parameters)
  - nb val == nb params
	nb out = nb val
	operation on each value with each param
  - nb params < nb val
	nb out = nb val
	operation on each value using the first param
  - nb val == 1 && nb params > 1
	nb out = min(nb params)
	operation on the value with each param
*/

namespace panda {

class DoubleMath_SmoothStep : public PandaObject
{
public:
	PANDA_CLASS(DoubleMath_SmoothStep, PandaObject)

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

	PReal smoothStep(PReal a, PReal b, PReal x)
	{
		if (x < a)	return 0;
		if (x >= b)	return 1;
		x = (x - a)/(b - a);
		return (x*x * (3 - 2*x));
	}

	void update()
	{
		const QVector<PReal>	&valInA = inputA.getValue(),
								&valInB = inputB.getValue(),
								&valInV = inputV.getValue();
		auto valOut = result.getAccessor();
		valOut.clear();

		int nbA = valInA.size(), nbB = valInB.size(), nbV = valInV.size();
		if(nbA && nbB && nbV)
		{
			int nb = nbV;
			int nbP = qMin(nbA, nbB);
			if(nbV > 1)
			{
				if(nbP != nbV)
					nbP = 1;
			}
			else
				nb = nbP;
			valOut.resize(nb);

			for(int i=0; i<nb; ++i)
				valOut[i] = smoothStep(valInA[i%nbP], valInB[i%nbP], valInV[i%nbV]);
		}

		cleanDirty();
	}

protected:
	Data< QVector<PReal> > inputA, inputB, inputV, result;
};

int DoubleMath_SmoothStepClass = RegisterObject<DoubleMath_SmoothStep>("Math/Real/Functions/Smooth Step").setDescription("Compute the smooth step of the input between min and max");

//****************************************************************************//

class DoubleMath_RiseCount : public PandaObject
{
public:
	PANDA_CLASS(DoubleMath_RiseCount, PandaObject)

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
		const QVector<PReal> &valIn = input.getValue();
		auto valOut = count.getAccessor();

		int size = valIn.size();
		valOut.resize(size);
		prevValues.resize(size);

		for(int i=0; i<size; ++i)
		{
			if(valIn[i] > prevValues[i])
				++valOut[i];
			prevValues[i] = valIn[i];
		}

		cleanDirty();
	}

	void reset()
	{
		PandaObject::reset();

		const QVector<PReal> &valIn = input.getValue();
		auto valOut = count.getAccessor();
		int size = valIn.size();
		prevValues.resize(size);
		for(int i=0; i<size; ++i)
		{
			prevValues[i] = valIn[i];
			valOut[i] = 0;
		}
	}

protected:
	QVector<PReal> prevValues;
	Data< QVector<PReal> > input;
	Data< QVector<int> > count;
};

int DoubleMath_RiseCountClass = RegisterObject<DoubleMath_RiseCount>("Math/Real/Functions/Rise count").setDescription("Count the number of rise transitions of a value");

//****************************************************************************//

class DoubleMath_FallCount : public PandaObject
{
public:
	PANDA_CLASS(DoubleMath_FallCount, PandaObject)

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
		const QVector<PReal> &valIn = input.getValue();
		auto valOut = count.getAccessor();

		int size = valIn.size();
		valOut.resize(size);
		prevValues.resize(size);

		for(int i=0; i<size; ++i)
		{
			if(valIn[i] < prevValues[i])
				++valOut[i];
			prevValues[i] = valIn[i];
		}

		cleanDirty();
	}

	void reset()
	{
		PandaObject::reset();

		const QVector<PReal> &valIn = input.getValue();
		auto valOut = count.getAccessor();
		int size = valIn.size();
		prevValues.resize(size);
		for(int i=0; i<size; ++i)
		{
			prevValues[i] = valIn[i];
			valOut[i] = 0;
		}
	}

protected:
	QVector<PReal> prevValues;
	Data< QVector<PReal> > input;
	Data< QVector<int> > count;
};

int DoubleMath_FallCountClass = RegisterObject<DoubleMath_FallCount>("Math/Real/Functions/Fall count").setDescription("Count the number of fall transitions of a value");

//****************************************************************************//

class DoubleMath_Constrain : public PandaObject
{
public:
	PANDA_CLASS(DoubleMath_Constrain, PandaObject)

	DoubleMath_Constrain(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData(&inputA, "min", "Minimum value"))
		, inputB(initData(&inputB, "max", "Maximum value"))
		, inputV(initData(&inputV, "input", "Value to constrain"))
		, result(initData(&result, "result", "Result of the operation"))
	{
		addInput(&inputA);
		addInput(&inputB);
		addInput(&inputV);

		addOutput(&result);
	}

	PReal constrain(PReal a, PReal b, PReal x)
	{
		if (x < a)	return a;
		if (x > b)	return b;
		return x;
	}

	void update()
	{
		const QVector<PReal>	&valInA = inputA.getValue(),
								&valInB = inputB.getValue(),
								&valInV = inputV.getValue();
		auto valOut = result.getAccessor();
		valOut.clear();

		int nbA = valInA.size(), nbB = valInB.size(), nbV = valInV.size();
		if(nbA && nbB && nbV)
		{
			int nb = nbV;
			int nbP = qMin(nbA, nbB);
			if(nbV > 1)
			{
				if(nbP != nbV)
					nbP = 1;
			}
			else
				nb = nbP;
			valOut.resize(nb);

			for(int i=0; i<nb; ++i)
				valOut[i] = constrain(valInA[i%nbP], valInB[i%nbP], valInV[i%nbV]);
		}

		cleanDirty();
	}

protected:
	Data< QVector<PReal> > inputA, inputB, inputV, result;
};

int DoubleMath_ConstrainClass = RegisterObject<DoubleMath_Constrain>("Math/Real/Functions/Constrain").setDescription("Constrain a value between a min and a max");

//****************************************************************************//

class DoubleMath_Pulse : public PandaObject
{
public:
	PANDA_CLASS(DoubleMath_Pulse, PandaObject)

	DoubleMath_Pulse(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData(&inputA, "min", "Minimum value"))
		, inputB(initData(&inputB, "max", "Maximum value"))
		, inputV(initData(&inputV, "input", "Value to constrain"))
		, result(initData(&result, "result", "Result of the operation"))
	{
		addInput(&inputA);
		addInput(&inputB);
		addInput(&inputV);

		addOutput(&result);
	}

	int pulse(PReal a, PReal b, PReal x)
	{
		if (x < a || x > b)	return 0;
		return 1;
	}

	void update()
	{
		const QVector<PReal>	&valInA = inputA.getValue(),
								&valInB = inputB.getValue(),
								&valInV = inputV.getValue();
		auto valOut = result.getAccessor();
		valOut.clear();

		int nbA = valInA.size(), nbB = valInB.size(), nbV = valInV.size();
		if(nbA && nbB && nbV)
		{
			int nb = nbV;
			int nbP = qMin(nbA, nbB);
			if(nbV > 1)
			{
				if(nbP != nbV)
					nbP = 1;
			}
			else
				nb = nbP;
			valOut.resize(nb);

			for(int i=0; i<nb; ++i)
				valOut[i] = pulse(valInA[i%nbP], valInB[i%nbP], valInV[i%nbV]);
		}

		cleanDirty();
	}

protected:
	Data< QVector<PReal> > inputA, inputB, inputV;
	Data< QVector<int> > result;
};

int DoubleMath_PulseClass = RegisterObject<DoubleMath_Pulse>("Math/Real/Functions/Pulse").setDescription("Set the output to 1 if the value is between min and max, 0 otherwise");

//****************************************************************************//

class DoubleMath_Remap : public PandaObject
{
public:
	PANDA_CLASS(DoubleMath_Remap, PandaObject)

	DoubleMath_Remap(PandaDocument *doc)
		: PandaObject(doc)
		, inputV(initData(&inputV, "input", "Values to re-map"))
		, inputMin(initData(&inputMin, "iMin", "Lower bound of the value's current range"))
		, inputMax(initData(&inputMax, "iMax", "Upper bound of the value's current range"))
		, outputMin(initData(&outputMin, "oMin", "Lower bound of the value's target  range"))
		, outputMax(initData(&outputMax, "oMax", "Upper bound of the value's target  range"))
		, result(initData(&result, "result", "Result of the remap"))
	{
		addInput(&inputV);
		addInput(&inputMin);
		addInput(&inputMax);
		addInput(&outputMin);
		addInput(&outputMax);

		addOutput(&result);
	}

	PReal remap(PReal v, PReal iMin, PReal iMax, PReal oMin, PReal oMax)
	{
		PReal p = (v - iMin) / (iMax - iMin);
		return oMin + p * (oMax - oMin);
	}

	void update()
	{
		const QVector<PReal>	&valInV = inputV.getValue(),
								&valInMin = inputMin.getValue(),
								&valInMax = inputMax.getValue(),
								&valOutMin = outputMin.getValue(),
								&valOutMax = outputMax.getValue();
		auto valOut = result.getAccessor();
		valOut.clear();

		int nbV = valInV.size(),
				nbInMin = valInMin.size(), nbInMax = valInMax.size(),
				nbOutMin = valOutMin.size(), nbOutMax = valOutMax.size();
		if(nbV && nbInMin && nbInMax && nbOutMin && nbOutMax)
		{
			int nb = nbV;
			int nbP = qMin(qMin(nbInMin, nbInMax), qMin(nbOutMin, nbOutMax));
			if(nbV > 1)
			{
				if(nbP != nbV)
					nbP = 1;
			}
			else
				nb = nbP;
			valOut.resize(nb);

			for(int i=0; i<nb; ++i)
				valOut[i] = remap(valInV[i%nbV],
								  valInMin[i%nbP], valInMax[i%nbP],
								  valOutMin[i%nbP], valOutMax[i%nbP]);
		}

		cleanDirty();
	}

protected:
	Data< QVector<PReal> > inputV, inputMin, inputMax, outputMin, outputMax, result;
};

int DoubleMath_RemapClass = RegisterObject<DoubleMath_Remap>("Math/Real/Functions/Remap").setDescription("Re-maps a value from one range to another");


} // namespace Panda

