#include <panda/document/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

#include <algorithm>
#include <cmath>

namespace panda {

class DoubleMath_1Value : public PandaObject
{
public:
	PANDA_ABSTRACT_CLASS(DoubleMath_1Value, PandaObject)

	DoubleMath_1Value(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("input", "Operand of the operation"))
		, result(initData("result", "Result of the operation"))
	{
		addInput(input);
		addOutput(result);
	}

	void update()
	{
		const std::vector<float> &valIn = input.getValue();
		auto valOut = result.getAccessor();

		int size = valIn.size();
		valOut.resize(size);

		for(int i=0; i<size; ++i)
			valOut[i] = compute(valIn[i]);
	}

	virtual float compute(float value) = 0;

protected:
	Data< std::vector<float> > input, result;
};

//****************************************************************************//

class DoubleMath_2Values : public PandaObject
{
public:
	PANDA_ABSTRACT_CLASS(DoubleMath_2Values, PandaObject)

	DoubleMath_2Values(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData("input 1", "First operand of the operation"))
		, inputB(initData("input 2", "Second operand of the operation"))
		, result(initData("result", "Result of the operation"))
	{
		addInput(inputA);
		addInput(inputB);

		addOutput(result);
	}

	void update()
	{
		const std::vector<float>	&valInA = inputA.getValue(),
								&valInB = inputB.getValue();
		auto valOut = result.getAccessor();
		valOut.clear();

		int nbA = valInA.size(), nbB = valInB.size();
		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = std::max(nbA, nbB);
			valOut.resize(nb);

			for(int i=0; i<nb; ++i)
				valOut[i] = compute(valInA[i%nbA], valInB[i%nbB]);
		}
	}

	virtual float compute(float valA, float valB) = 0;

protected:
	Data< std::vector<float> > inputA, inputB, result;
};

//****************************************************************************//

class DoubleMath_Add : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Add, DoubleMath_2Values)

	DoubleMath_Add(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual float compute(float valA, float valB)
	{ return valA + valB; }
};

int DoubleMath_AddClass = RegisterObject<DoubleMath_Add>("Math/Real/Addition").setDescription("Addition of 2 reals");

//****************************************************************************//

class DoubleMath_Sub : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Sub, DoubleMath_2Values)

	DoubleMath_Sub(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual float compute(float valA, float valB)
	{ return valA - valB; }
};

int DoubleMath_SubClass = RegisterObject<DoubleMath_Sub>("Math/Real/Substraction").setDescription("Substraction of 2 reals");

//****************************************************************************//

class DoubleMath_Mult : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Mult, DoubleMath_2Values)

	DoubleMath_Mult(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual float compute(float valA, float valB)
	{ return valA * valB; }
};

int DoubleMath_MultClass = RegisterObject<DoubleMath_Mult>("Math/Real/Multiplication").setDescription("Multiplication of 2 reals");

//****************************************************************************//

class DoubleMath_Div : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Div, DoubleMath_2Values)

	DoubleMath_Div(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual float compute(float valA, float valB)
	{ return valA / valB; }
};

int DoubleMath_DivClass = RegisterObject<DoubleMath_Div>("Math/Real/Division").setDescription("Division of 2 reals");

//****************************************************************************//

class DoubleMath_Mod : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Mod, DoubleMath_2Values)

	DoubleMath_Mod(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual float compute(float valA, float valB)
	{ return std::fmod(valA, valB); }
};

int DoubleMath_ModClass = RegisterObject<DoubleMath_Mod>("Math/Real/Modulo").setDescription("Remainder of the division");

//****************************************************************************//

class DoubleMath_Abs : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Abs, DoubleMath_1Value)

	DoubleMath_Abs(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual float compute(float val)
	{ return std::abs(val); }
};

int DoubleMath_AbsClass = RegisterObject<DoubleMath_Abs>("Math/Real/Absolute").setDescription("Absolute value of the input");

//****************************************************************************//

class DoubleMath_Floor : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Floor, DoubleMath_1Value)

	DoubleMath_Floor(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual float compute(float val)
	{ return std::floor(val); }
};

int DoubleMath_FloorClass = RegisterObject<DoubleMath_Floor>("Math/Real/Floor").setDescription("Floor value of the input");

//****************************************************************************//

class DoubleMath_Ceil : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Ceil, DoubleMath_1Value)

	DoubleMath_Ceil(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual float compute(float val)
	{ return std::ceil(val); }
};

int DoubleMath_CeilClass = RegisterObject<DoubleMath_Ceil>("Math/Real/Ceiling").setDescription("Ceiling value of the input");

//****************************************************************************//

class DoubleMath_Round : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Round, DoubleMath_1Value)

	DoubleMath_Round(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual float compute(float val)
	{ return std::round(val); }
};

int DoubleMath_RoundClass = RegisterObject<DoubleMath_Round>("Math/Real/Rounding").setDescription("Rounded value of the input");

//****************************************************************************//

class DoubleMath_Power : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Power, DoubleMath_2Values)

	DoubleMath_Power(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual float compute(float valA, float valB)
	{ return std::pow(valA, valB); }
};

int DoubleMath_PowerClass = RegisterObject<DoubleMath_Power>("Math/Real/Power").setDescription("First value raised by the exponent of the second");

//****************************************************************************//

class DoubleMath_Sqrt : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Sqrt, DoubleMath_1Value)

	DoubleMath_Sqrt(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual float compute(float val)
	{ return std::sqrt(val); }
};

int DoubleMath_SqrtClass = RegisterObject<DoubleMath_Sqrt>("Math/Real/Square root").setDescription("Square root of the value");

//****************************************************************************//

class DoubleMath_Log : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Log, DoubleMath_1Value)

	DoubleMath_Log(PandaDocument *doc)
		: DoubleMath_1Value(doc)
	{}

	virtual float compute(float val)
	{ return std::log(val); }

};

int DoubleMath_LogClass = RegisterObject<DoubleMath_Log>("Math/Real/Natural logarithm").setDescription("Natural logarithm of the value");

//****************************************************************************//

class DoubleMath_LogBase : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_LogBase, DoubleMath_1Value)

	DoubleMath_LogBase(PandaDocument *doc)
		: DoubleMath_1Value(doc)
		, base(initData((float)10.0, "base", "Base of the logarithm"))
	{
		addInput(base);
	}

	virtual float compute(float val)
	{ return std::log(val) / std::log(base.getValue()); }

protected:
	Data<float> base;
};

int DoubleMath_LogBaseClass = RegisterObject<DoubleMath_LogBase>("Math/Real/Logarithm").setDescription("Logarithm to a chosen base of the value");

//****************************************************************************//

class DoubleMath_Exp : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Exp, DoubleMath_1Value)

	DoubleMath_Exp(PandaDocument *doc)
		: DoubleMath_1Value(doc)
	{}

	virtual float compute(float val)
	{ return std::exp(val); }

};

int DoubleMath_ExpClass = RegisterObject<DoubleMath_Exp>("Math/Real/Exponential").setDescription("Returns the exponential function of e to the power the value");

//****************************************************************************//

class DoubleMath_Min : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Min, DoubleMath_2Values)

	DoubleMath_Min(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual float compute(float valA, float valB)
	{ return std::min(valA, valB); }
};

int DoubleMath_MinClass = RegisterObject<DoubleMath_Min>("Math/Real/Functions/Minimum").setDescription("Returns the lowest value of the 2 inputs");

//****************************************************************************//

class DoubleMath_Max : public DoubleMath_2Values
{
public:
	PANDA_CLASS(DoubleMath_Max, DoubleMath_2Values)

	DoubleMath_Max(PandaDocument *doc)
		: DoubleMath_2Values(doc) { }

	virtual float compute(float valA, float valB)
	{ return std::max(valA, valB); }
};

int DoubleMath_MaxClass = RegisterObject<DoubleMath_Max>("Math/Real/Functions/Maximum").setDescription("Returns the highest value of the 2 inputs");

//****************************************************************************//

class DoubleMath_Cos : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Cos, DoubleMath_1Value)

	DoubleMath_Cos(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual float compute(float val)
	{ return std::cos(val * static_cast<float>(M_PI) / 180); }
};

int DoubleMath_CosClass = RegisterObject<DoubleMath_Cos>("Math/Real/Trigonometry/Cosine").setDescription("Cosine of the value (in degrees)");

//****************************************************************************//

class DoubleMath_Sin : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Sin, DoubleMath_1Value)

	DoubleMath_Sin(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual float compute(float val)
	{ return std::sin(val * static_cast<float>(M_PI) / 180); }
};

int DoubleMath_SinClass = RegisterObject<DoubleMath_Sin>("Math/Real/Trigonometry/Sine").setDescription("Sine of the value (in degrees)");

//****************************************************************************//

class DoubleMath_Tan : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Tan, DoubleMath_1Value)

	DoubleMath_Tan(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual float compute(float val)
	{ return std::tan(val * static_cast<float>(M_PI) / 180); }
};

int DoubleMath_TanClass = RegisterObject<DoubleMath_Tan>("Math/Real/Trigonometry/Tangent").setDescription("Tangent of the value (in degrees)");

//****************************************************************************//

class DoubleMath_Arccos : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Arccos, DoubleMath_1Value)

	DoubleMath_Arccos(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual float compute(float val)
	{ return std::acos(val) * 180 / static_cast<float>(M_PI); }
};

int DoubleMath_ArccosClass = RegisterObject<DoubleMath_Arccos>("Math/Real/Trigonometry/Arccosine").setDescription("Arc-cosine of the value (result in degrees)");

//****************************************************************************//

class DoubleMath_Arcsin : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Arcsin, DoubleMath_1Value)

	DoubleMath_Arcsin(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual float compute(float val)
	{ return std::asin(val) * 180 / static_cast<float>(M_PI); }
};

int DoubleMath_ArcsinClass = RegisterObject<DoubleMath_Arcsin>("Math/Real/Trigonometry/Arcsine").setDescription("Arc-sine of the value (result in degrees)");

//****************************************************************************//

class DoubleMath_Arctan : public DoubleMath_1Value
{
public:
	PANDA_CLASS(DoubleMath_Arctan, DoubleMath_1Value)

	DoubleMath_Arctan(PandaDocument *doc)
		: DoubleMath_1Value(doc) { }

	virtual float compute(float val)
	{ return std::atan(val) * 180 / static_cast<float>(M_PI); }
};

int DoubleMath_ArctanClass = RegisterObject<DoubleMath_Arctan>("Math/Real/Trigonometry/Arctangent").setDescription("Arc-tangent of the value (result in degrees)");


} // namespace Panda


