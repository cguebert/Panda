#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QVector>
#include <qmath.h>

namespace panda {

class DoubleMath_1Value : public PandaObject
{
public:
	PANDA_ABSTRACT_CLASS(DoubleMath_1Value, PandaObject)

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
		auto valOut = result.getAccessor();

		int size = valIn.size();
		valOut.resize(size);

		for(int i=0; i<size; ++i)
			valOut[i] = compute(valIn[i]);

		cleanDirty();
	}

	virtual double compute(const double& value) = 0;

protected:
	Data< QVector<double> > input, result;
};

//*************************************************************************//

class DoubleMath_2Values : public PandaObject
{
public:
	PANDA_ABSTRACT_CLASS(DoubleMath_1Value, PandaObject)

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
		auto valOut = result.getAccessor();
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
		}

		cleanDirty();
	}

	virtual double compute(const double& valA, const double& valB) = 0;

protected:
	Data< QVector<double> > inputA, inputB, result;
};

//*************************************************************************//

class DoubleMath_Add : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Add, DoubleMath_2Values)

	DoubleMath_Add(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return valA + valB; }
};

int DoubleMath_AddClass = RegisterObject<DoubleMath_Add>("Math/Real/Addition").setDescription("Addition of 2 reals");

//*************************************************************************//

class DoubleMath_Sub : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Sub, DoubleMath_2Values)

	DoubleMath_Sub(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return valA - valB; }
};

int DoubleMath_SubClass = RegisterObject<DoubleMath_Sub>("Math/Real/Substraction").setDescription("Substraction of 2 reals");

//*************************************************************************//

class DoubleMath_Mult : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Mult, DoubleMath_2Values)

	DoubleMath_Mult(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return valA * valB; }
};

int DoubleMath_MultClass = RegisterObject<DoubleMath_Mult>("Math/Real/Multiplication").setDescription("Multiplication of 2 reals");

//*************************************************************************//

class DoubleMath_Div : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Div, DoubleMath_2Values)

	DoubleMath_Div(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return valA / valB; }
};

int DoubleMath_DivClass = RegisterObject<DoubleMath_Div>("Math/Real/Division").setDescription("Division of 2 reals");

//*************************************************************************//

class DoubleMath_Mod : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Mod, DoubleMath_2Values)

	DoubleMath_Mod(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return fmod(valA, valB); }
};

int DoubleMath_ModClass = RegisterObject<DoubleMath_Mod>("Math/Real/Modulo").setDescription("Remainder of the division");

//*************************************************************************//

class DoubleMath_Abs : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Abs, DoubleMath_1Value)

	DoubleMath_Abs(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return qAbs(val); }
};

int DoubleMath_AbsClass = RegisterObject<DoubleMath_Abs>("Math/Real/Absolute").setDescription("Absolute value of the input");

//*************************************************************************//

class DoubleMath_Floor : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Floor, DoubleMath_1Value)

	DoubleMath_Floor(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return qFloor(val); }
};

int DoubleMath_FloorClass = RegisterObject<DoubleMath_Floor>("Math/Real/Floor").setDescription("Floor value of the input");

//*************************************************************************//

class DoubleMath_Ceil : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Ceil, DoubleMath_1Value)

	DoubleMath_Ceil(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return qCeil(val); }
};

int DoubleMath_CeilClass = RegisterObject<DoubleMath_Ceil>("Math/Real/Ceiling").setDescription("Ceiling value of the input");

//*************************************************************************//

class DoubleMath_Round : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Round, DoubleMath_1Value)

	DoubleMath_Round(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return qRound(val); }
};

int DoubleMath_RoundClass = RegisterObject<DoubleMath_Round>("Math/Real/Rounding").setDescription("Rounded value of the input");

//*************************************************************************//

class DoubleMath_Power : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Power, DoubleMath_2Values)

	DoubleMath_Power(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return pow(valA, valB); }
};

int DoubleMath_PowerClass = RegisterObject<DoubleMath_Power>("Math/Real/Power").setDescription("First value raised by the exponent of the second");

//*************************************************************************//

class DoubleMath_Sqrt : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Sqrt, DoubleMath_1Value)

	DoubleMath_Sqrt(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return sqrt(val); }
};

int DoubleMath_SqrtClass = RegisterObject<DoubleMath_Sqrt>("Math/Real/Square root").setDescription("Square root of the value");

//*************************************************************************//

class DoubleMath_Log : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Log, DoubleMath_1Value)

	DoubleMath_Log(PandaDocument *doc)
		: DoubleMath_1Value(doc)
	{}

	virtual double compute(const double& val)
	{ return qLn(val); }

};

int DoubleMath_LogClass = RegisterObject<DoubleMath_Log>("Math/Real/Natural logarithm").setDescription("Natural logarithm of the value");

//*************************************************************************//

class DoubleMath_LogBase : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_LogBase, DoubleMath_1Value)

	DoubleMath_LogBase(PandaDocument *doc)
		: DoubleMath_1Value(doc)
		, base(initData(&base, 10.0, "base", "Base of the logarithm"))
	{
		addInput(&base);
	}

	virtual double compute(const double& val)
	{ return qLn(val) / qLn(base.getValue()); }

protected:
	Data<double> base;
};

int DoubleMath_LogBaseClass = RegisterObject<DoubleMath_LogBase>("Math/Real/Logarithm").setDescription("Logarithm to a chosen base of the value");

//*************************************************************************//

class DoubleMath_Exp : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Exp, DoubleMath_1Value)

	DoubleMath_Exp(PandaDocument *doc)
		: DoubleMath_1Value(doc)
	{}

	virtual double compute(const double& val)
	{ return qExp(val); }

};

int DoubleMath_ExpClass = RegisterObject<DoubleMath_Exp>("Math/Real/Exponential").setDescription("Returns the exponential function of e to the power the value");

//*************************************************************************//

class DoubleMath_Min : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Min, DoubleMath_2Values)

	DoubleMath_Min(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return qMin(valA, valB); }
};

int DoubleMath_MinClass = RegisterObject<DoubleMath_Min>("Math/Real/Functions/Minimum").setDescription("Returns the lowest value of the 2 inputs");

//*************************************************************************//

class DoubleMath_Max : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Max, DoubleMath_2Values)

	DoubleMath_Max(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return qMax(valA, valB); }
};

int DoubleMath_MaxClass = RegisterObject<DoubleMath_Max>("Math/Real/Functions/Maximum").setDescription("Returns the highest value of the 2 inputs");

//*************************************************************************//

class DoubleMath_Step : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Step, DoubleMath_2Values)

	DoubleMath_Step(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual double compute(const double& valA, const double& valB)
	{ return (valA >= valB); }
};

int DoubleMath_StepClass = RegisterObject<DoubleMath_Step>("Math/Real/Functions/Step").setDescription("Returns 0 if input1 < input2, 1 otherwise");

//*************************************************************************//

class DoubleMath_Cos : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Cos, DoubleMath_1Value)

	DoubleMath_Cos(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return qCos(val * M_PI / 180); }
};

int DoubleMath_CosClass = RegisterObject<DoubleMath_Cos>("Math/Real/Trigonometry/Cosine").setDescription("Cosine of the value (in degrees)");

//*************************************************************************//

class DoubleMath_Sin : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Sin, DoubleMath_1Value)

	DoubleMath_Sin(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return qSin(val * M_PI / 180); }
};

int DoubleMath_SinClass = RegisterObject<DoubleMath_Sin>("Math/Real/Trigonometry/Sine").setDescription("Sine of the value (in degrees)");

//*************************************************************************//

class DoubleMath_Tan : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Tan, DoubleMath_1Value)

	DoubleMath_Tan(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return qCos(val * M_PI / 180); }
};

int DoubleMath_TanClass = RegisterObject<DoubleMath_Tan>("Math/Real/Trigonometry/Tangent").setDescription("Tangent of the value (in degrees)");

//*************************************************************************//

class DoubleMath_Arccos : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Arccos, DoubleMath_1Value)

	DoubleMath_Arccos(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return qAcos(val) * 180 / M_PI; }
};

int DoubleMath_ArccosClass = RegisterObject<DoubleMath_Arccos>("Math/Real/Trigonometry/Arccosine").setDescription("Arc-cosine of the value (result in degrees)");

//*************************************************************************//

class DoubleMath_Arcsin : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Arcsin, DoubleMath_1Value)

	DoubleMath_Arcsin(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return qAsin(val) * 180 / M_PI; }
};

int DoubleMath_ArcsinClass = RegisterObject<DoubleMath_Arcsin>("Math/Real/Trigonometry/Arcsine").setDescription("Arc-sine of the value (result in degrees)");

//*************************************************************************//

class DoubleMath_Arctan : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Arctan, DoubleMath_1Value)

	DoubleMath_Arctan(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual double compute(const double& val)
	{ return qAtan(val) * 180 / M_PI; }
};

int DoubleMath_ArctanClass = RegisterObject<DoubleMath_Arctan>("Math/Real/Trigonometry/Arctangent").setDescription("Arc-tangent of the value (result in degrees)");


} // namespace Panda


