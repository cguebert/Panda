#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QVector>
#include <math.h>

namespace panda {

class DoubleMath_1Value : public PandaObject
{
public:
	DoubleMath_1Value(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "Operand of the operation"))
		, result(initData(&result, "result", "Result of the operation"))
	{
		addInput(&input);
		addOutput(&result);
	}

	void update()
	{
		const QVector<double> &valIn = input.getValue();
		QVector<double> &valOut = *result.beginEdit();

		int size = valIn.size();
		valOut.resize(size);

		for(int i=0; i<size; ++i)
			valOut[i] = compute(valIn[i]);

		result.endEdit();
		this->cleanDirty();
	}

	virtual double compute(const double& value) = 0;

protected:
	Data< QVector<double> > input, result;
};

//*************************************************************************//

class DoubleMath_2Values : public PandaObject
{
public:
	DoubleMath_2Values(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData(&inputA, "input 1", "First operand of the operation"))
		, inputB(initData(&inputB, "input 2", "Second operand of the operation"))
		, result(initData(&result, "result", "Result of the operation"))
	{
		addInput(&inputA);
		addInput(&inputB);

		addOutput(&result);
	}

	void update()
	{
		const QVector<double>	&valInA = inputA.getValue(),
								&valInB = inputB.getValue();
		QVector<double> &valOut = *result.beginEdit();
		valOut.clear();

		int nbA = valInA.size(), nbB = valInB.size();
		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = qMax(nbA, nbB);
			valOut.resize(nb);

			for(int i=0; i<nb; ++i)
				valOut[i] = compute(valInA[i%nbA], valInB[i%nbB]);

			result.endEdit();
		}

		this->cleanDirty();
	}

	virtual double compute(const double& valA, const double& valB) = 0;

protected:
	Data< QVector<double> > inputA, inputB, result;
};

//*************************************************************************//

class DoubleMath_Add : public DoubleMath_2Values
{
public:
	DoubleMath_Add(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return valA + valB; }
};

int DoubleMath_AddClass = RegisterObject("Math/Real/Addition").setClass<DoubleMath_Add>().setDescription("Addition of 2 reals");

//*************************************************************************//

class DoubleMath_Sub : public DoubleMath_2Values
{
public:
	DoubleMath_Sub(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return valA - valB; }
};

int DoubleMath_SubClass = RegisterObject("Math/Real/Substraction").setClass<DoubleMath_Sub>().setDescription("Substraction of 2 reals");

//*************************************************************************//

class DoubleMath_Mult : public DoubleMath_2Values
{
public:
	DoubleMath_Mult(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return valA * valB; }
};

int DoubleMath_MultClass = RegisterObject("Math/Real/Multiplication").setClass<DoubleMath_Mult>().setDescription("Multiplication of 2 reals");

//*************************************************************************//

class DoubleMath_Div : public DoubleMath_2Values
{
public:
	DoubleMath_Div(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return valA / valB; }
};

int DoubleMath_DivClass = RegisterObject("Math/Real/Division").setClass<DoubleMath_Div>().setDescription("Division of 2 reals");

//*************************************************************************//

class DoubleMath_Mod : public DoubleMath_2Values
{
public:
	DoubleMath_Mod(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return fmod(valA, valB); }
};

int DoubleMath_ModClass = RegisterObject("Math/Real/Modulo").setClass<DoubleMath_Mod>().setDescription("Remainder of the division");

//*************************************************************************//

class DoubleMath_Abs : public DoubleMath_1Value
{
public:
	DoubleMath_Abs(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return qAbs(val); }
};

int DoubleMath_AbsClass = RegisterObject("Math/Real/Absolute").setClass<DoubleMath_Abs>().setDescription("Absolute value of the input");

//*************************************************************************//

class DoubleMath_Floor : public DoubleMath_1Value
{
public:
	DoubleMath_Floor(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return floor(val); }
};

int DoubleMath_FloorClass = RegisterObject("Math/Real/Floor").setClass<DoubleMath_Floor>().setDescription("Floor value of the input");

//*************************************************************************//

class DoubleMath_Ceil : public DoubleMath_1Value
{
public:
	DoubleMath_Ceil(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return ceil(val); }
};

int DoubleMath_CeilClass = RegisterObject("Math/Real/Ceiling").setClass<DoubleMath_Ceil>().setDescription("Ceiling value of the input");

//*************************************************************************//

class DoubleMath_Round : public DoubleMath_1Value
{
public:
	DoubleMath_Round(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return floor(val+0.5); }
};

int DoubleMath_RoundClass = RegisterObject("Math/Real/Rounding").setClass<DoubleMath_Round>().setDescription("Rounded value of the input");

//*************************************************************************//

class DoubleMath_Power : public DoubleMath_2Values
{
public:
	DoubleMath_Power(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return pow(valA, valB); }
};

int DoubleMath_PowerClass = RegisterObject("Math/Real/Power").setClass<DoubleMath_Power>().setDescription("First value raised by the exponent of the second");

//*************************************************************************//

class DoubleMath_Sqrt : public DoubleMath_1Value
{
public:
	DoubleMath_Sqrt(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return sqrt(val); }
};

int DoubleMath_SqrtClass = RegisterObject("Math/Real/Square root").setClass<DoubleMath_Sqrt>().setDescription("Square root of the value");

//*************************************************************************//

class DoubleMath_Min : public DoubleMath_2Values
{
public:
	DoubleMath_Min(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return qMin(valA, valB); }
};

int DoubleMath_MinClass = RegisterObject("Math/Real/Minimum").setClass<DoubleMath_Min>().setDescription("Returns the lowest value of the 2 inputs");

//*************************************************************************//

class DoubleMath_Max : public DoubleMath_2Values
{
public:
	DoubleMath_Max(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return qMax(valA, valB); }
};

int DoubleMath_MaxClass = RegisterObject("Math/Real/Maximum").setClass<DoubleMath_Max>().setDescription("Returns the highest value of the 2 inputs");

//*************************************************************************//

class DoubleMath_Step : public DoubleMath_2Values
{
public:
	DoubleMath_Step(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return (valA >= valB); }
};

int DoubleMath_StepClass = RegisterObject("Math/Real/Step").setClass<DoubleMath_Step>().setDescription("Returns 0 if input1 < input2, 1 otherwise");


} // namespace Panda


