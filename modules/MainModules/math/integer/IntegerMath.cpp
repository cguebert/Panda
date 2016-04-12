#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

#include <algorithm>
#include <cmath>

namespace panda {

class IntegerMath_1Value : public PandaObject
{
public:
	PANDA_ABSTRACT_CLASS(IntegerMath_1Value, PandaObject)

	IntegerMath_1Value(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("input", "Operand of the operation"))
		, result(initData("result", "Result of the operation"))
	{
		addInput(input);
		addOutput(result);
	}

	void update()
	{
		const std::vector<int> &valIn = input.getValue();
		auto valOut = result.getAccessor();

		int size = valIn.size();
		valOut.resize(size);

		for(int i=0; i<size; ++i)
			valOut[i] = compute(valIn[i]);
	}

	virtual int compute(int value) = 0;

protected:
	Data< std::vector<int> > input, result;
};

//****************************************************************************//

class IntegerMath_2Values : public PandaObject
{
public:
	PANDA_ABSTRACT_CLASS(IntegerMath_2Values, PandaObject)

	IntegerMath_2Values(PandaDocument *doc)
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
		const std::vector<int>	&valInA = inputA.getValue(),
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

	virtual int compute(int valA, int valB) = 0;

protected:
	Data< std::vector<int> > inputA, inputB, result;
};

//****************************************************************************//

class IntegerMath_Add : public IntegerMath_2Values
{
public:
	PANDA_CLASS(IntegerMath_Add, IntegerMath_2Values)

	IntegerMath_Add(PandaDocument *doc)
		: IntegerMath_2Values(doc) { }

	virtual int compute(int valA, int valB)
	{ return valA + valB; }
};

int IntegerMath_AddClass = RegisterObject<IntegerMath_Add>("Math/Integer/Addition").setDescription("Addition of 2 integers");

//****************************************************************************//

class IntegerMath_Sub : public IntegerMath_2Values
{
public:
	PANDA_CLASS(IntegerMath_Sub, IntegerMath_2Values)

	IntegerMath_Sub(PandaDocument *doc)
		: IntegerMath_2Values(doc) { }

	virtual int compute(int valA, int valB)
	{ return valA - valB; }
};

int IntegerMath_SubClass = RegisterObject<IntegerMath_Sub>("Math/Integer/Substraction").setDescription("Substraction of 2 integers");

//****************************************************************************//

class IntegerMath_Mult : public IntegerMath_2Values
{
public:
	PANDA_CLASS(IntegerMath_Mult, IntegerMath_2Values)

	IntegerMath_Mult(PandaDocument *doc)
		: IntegerMath_2Values(doc) { }

	virtual int compute(int valA, int valB)
	{ return valA * valB; }
};

int IntegerMath_MultClass = RegisterObject<IntegerMath_Mult>("Math/Integer/Multiplication").setDescription("Multiplication of 2 integers");

//****************************************************************************//

class IntegerMath_Div : public IntegerMath_2Values
{
public:
	PANDA_CLASS(IntegerMath_Div, IntegerMath_2Values)

	IntegerMath_Div(PandaDocument *doc)
		: IntegerMath_2Values(doc) { }

	virtual int compute(int valA, int valB)
	{ return valA / valB; }
};

int IntegerMath_DivClass = RegisterObject<IntegerMath_Div>("Math/Integer/Euclidean division").setDescription("Euclidean division of 2 integers");

//****************************************************************************//

class IntegerMath_Mod : public IntegerMath_2Values
{
public:
	PANDA_CLASS(IntegerMath_Mod, IntegerMath_2Values)

	IntegerMath_Mod(PandaDocument *doc)
		: IntegerMath_2Values(doc) { }

	virtual int compute(int valA, int valB)
	{ return valA % valB; }
};

int IntegerMath_ModClass = RegisterObject<IntegerMath_Mod>("Math/Integer/Modulo").setDescription("Remainder of the division");

//****************************************************************************//

class IntegerMath_Abs : public IntegerMath_1Value
{
public:
	PANDA_CLASS(IntegerMath_Abs, IntegerMath_1Value)

	IntegerMath_Abs(PandaDocument *doc)
		: IntegerMath_1Value(doc) { }

	virtual int compute(int val)
	{ return std::abs(val); }
};

int IntegerMath_AbsClass = RegisterObject<IntegerMath_Abs>("Math/Integer/Absolute").setDescription("Absolute value of the input");

//****************************************************************************//

class IntegerMath_Min : public IntegerMath_2Values
{
public:
	PANDA_CLASS(IntegerMath_Min, IntegerMath_2Values)

	IntegerMath_Min(PandaDocument *doc)
		: IntegerMath_2Values(doc) { }

	virtual int compute(int valA, int valB)
	{ return std::min(valA, valB); }
};

int IntegerMath_MinClass = RegisterObject<IntegerMath_Min>("Math/Integer/Functions/Minimum").setDescription("Returns the lowest value of the 2 inputs");

//****************************************************************************//

class IntegerMath_Max : public IntegerMath_2Values
{
public:
	PANDA_CLASS(IntegerMath_Max, IntegerMath_2Values)

	IntegerMath_Max(PandaDocument *doc)
		: IntegerMath_2Values(doc) { }

	virtual int compute(int valA, int valB)
	{ return std::max(valA, valB); }
};

int IntegerMath_MaxClass = RegisterObject<IntegerMath_Max>("Math/Integer/Functions/Maximum").setDescription("Returns the highest value of the 2 inputs");

} // namespace Panda


