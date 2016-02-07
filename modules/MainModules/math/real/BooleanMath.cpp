#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

#include <algorithm>

namespace panda {

class BooleanMath_Greater : public PandaObject
{
public:
	PANDA_ABSTRACT_CLASS(BooleanMath_Greater, PandaObject)

	BooleanMath_Greater(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData("inputA", "First input"))
		, inputB(initData("inputB", "Second input"))
		, result(initData("result", "Result of the comparison"))
	{
		addInput(inputA);
		addInput(inputB);
		addOutput(result);
	}

	void update()
	{
		const std::vector<float> &valInA = inputA.getValue();
		const std::vector<float> &valInB = inputB.getValue();
		auto valOut = result.getAccessor();
		valOut.clear();

		int nbA = valInA.size(), nbB = valInB.size();
		if(!nbA || !nbB)
		{
			cleanDirty();
			return;
		}

		int size = std::max(nbA, nbB);
		valOut.resize(size);

		if(nbA < size) nbA = 1;
		if(nbB < size) nbB = 1;

		for(int i=0; i<size; ++i)
			valOut[i] = (valInA[i%nbA] > valInB[i%nbB]) ? 1 : 0;

		cleanDirty();
	}

protected:
	Data< std::vector<float> > inputA, inputB;
	Data< std::vector<int> > result;
};

int BooleanMath_GreaterClass = RegisterObject<BooleanMath_Greater>("Math/Boolean/Greater").setDescription("Compare 2 reals; output is 1 if A > B, 0 otherwise");

//****************************************************************************//

class BooleanMath_Equal : public PandaObject
{
public:
	PANDA_ABSTRACT_CLASS(BooleanMath_Equal, PandaObject)

	BooleanMath_Equal(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData("inputA", "First input"))
		, inputB(initData("inputB", "Second input"))
		, result(initData("result", "Result of the comparison"))
	{
		addInput(inputA);
		addInput(inputB);
		addOutput(result);
	}

	void update()
	{
		const std::vector<float> &valInA = inputA.getValue();
		const std::vector<float> &valInB = inputB.getValue();
		auto valOut = result.getAccessor();
		valOut.clear();

		int nbA = valInA.size(), nbB = valInB.size();
		if(!nbA || !nbB)
		{
			cleanDirty();
			return;
		}

		int size = std::max(nbA, nbB);
		valOut.resize(size);

		if(nbA < size) nbA = 1;
		if(nbB < size) nbB = 1;

		for(int i=0; i<size; ++i)
			valOut[i] = (valInA[i%nbA] == valInB[i%nbB]) ? 1 : 0;

		cleanDirty();
	}

protected:
	Data< std::vector<float> > inputA, inputB;
	Data< std::vector<int> > result;
};

int BooleanMath_EqualClass = RegisterObject<BooleanMath_Equal>("Math/Boolean/Equal").setDescription("Compare 2 reals; output is 1 if A is equal to B, 0 otherwise");

//****************************************************************************//

class BooleanMath_Or : public PandaObject
{
public:
	PANDA_ABSTRACT_CLASS(BooleanMath_Or, PandaObject)

	BooleanMath_Or(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData("inputA", "First input"))
		, inputB(initData("inputB", "Second input"))
		, result(initData("result", "Result of the OR operation"))
	{
		addInput(inputA);
		addInput(inputB);
		addOutput(result);
	}

	void update()
	{
		const std::vector<int> &valInA = inputA.getValue();
		const std::vector<int> &valInB = inputB.getValue();
		auto valOut = result.getAccessor();
		valOut.clear();

		int nbA = valInA.size(), nbB = valInB.size();
		if(!nbA || !nbB)
		{
			cleanDirty();
			return;
		}

		int size = std::max(nbA, nbB);
		valOut.resize(size);

		if(nbA < size) nbA = 1;
		if(nbB < size) nbB = 1;

		for(int i=0; i<size; ++i)
			valOut[i] = (valInA[i%nbA] || valInB[i%nbB]) ? 1 : 0;

		cleanDirty();
	}

protected:
	Data< std::vector<int> > inputA, inputB, result;
};

int BooleanMath_OrClass = RegisterObject<BooleanMath_Or>("Math/Boolean/Or").setDescription("Compute the boolean operation OR on the 2 inputs");

//****************************************************************************//

class BooleanMath_And : public PandaObject
{
public:
	PANDA_ABSTRACT_CLASS(BooleanMath_And, PandaObject)

	BooleanMath_And(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData("inputA", "First input"))
		, inputB(initData("inputB", "Second input"))
		, result(initData("result", "Result of the AND operation"))
	{
		addInput(inputA);
		addInput(inputB);
		addOutput(result);
	}

	void update()
	{
		const std::vector<int> &valInA = inputA.getValue();
		const std::vector<int> &valInB = inputB.getValue();
		auto valOut = result.getAccessor();
		valOut.clear();

		int nbA = valInA.size(), nbB = valInB.size();
		if(!nbA || !nbB)
		{
			cleanDirty();
			return;
		}

		int size = std::max(nbA, nbB);
		valOut.resize(size);

		if(nbA < size) nbA = 1;
		if(nbB < size) nbB = 1;

		for(int i=0; i<size; ++i)
			valOut[i] = (valInA[i%nbA] && valInB[i%nbB]) ? 1 : 0;

		cleanDirty();
	}

protected:
	Data< std::vector<int> > inputA, inputB, result;
};

int BooleanMath_AndClass = RegisterObject<BooleanMath_And>("Math/Boolean/And").setDescription("Compute the boolean operation AND on the 2 inputs");

//****************************************************************************//

class BooleanMath_Xor : public PandaObject
{
public:
	PANDA_ABSTRACT_CLASS(BooleanMath_Xor, PandaObject)

	BooleanMath_Xor(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData("inputA", "First input"))
		, inputB(initData("inputB", "Second input"))
		, result(initData("result", "Result of the XOR operation"))
	{
		addInput(inputA);
		addInput(inputB);
		addOutput(result);
	}

	void update()
	{
		const std::vector<int> &valInA = inputA.getValue();
		const std::vector<int> &valInB = inputB.getValue();
		auto valOut = result.getAccessor();
		valOut.clear();

		int nbA = valInA.size(), nbB = valInB.size();
		if(!nbA || !nbB)
		{
			cleanDirty();
			return;
		}

		int size = std::max(nbA, nbB);
		valOut.resize(size);

		if(nbA < size) nbA = 1;
		if(nbB < size) nbB = 1;

		for(int i=0; i<size; ++i)
			valOut[i] = ((valInA[i%nbA] != 0) ^ (valInB[i%nbB] != 0));

		cleanDirty();
	}

protected:
	Data< std::vector<int> > inputA, inputB, result;
};

int BooleanMath_XorClass = RegisterObject<BooleanMath_Xor>("Math/Boolean/Xor").setDescription("Compute the boolean operation XOR on the 2 inputs");

//****************************************************************************//


class BooleanMath_Not : public PandaObject
{
public:
	PANDA_ABSTRACT_CLASS(BooleanMath_Not, PandaObject)

	BooleanMath_Not(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("input", "Input"))
		, result(initData("result", "Negation of the input"))
	{
		addInput(input);
		addOutput(result);
	}

	void update()
	{
		const std::vector<int> &valIn = input.getValue();
		auto valOut = result.getAccessor();

		int nb = valIn.size();
		valOut.resize(nb);

		for(int i=0; i<nb; ++i)
			valOut[i] = (valIn[i] == 0 ? 1 : 0);

		cleanDirty();
	}

protected:
	Data< std::vector<int> > input, result;
};

int BooleanMath_NotClass = RegisterObject<BooleanMath_Not>("Math/Boolean/Not").setDescription("Compute the negation of the input");

//****************************************************************************//


} // namespace Panda


