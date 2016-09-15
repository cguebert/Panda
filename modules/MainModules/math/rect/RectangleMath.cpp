#include <panda/document/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Rect.h>

namespace panda {

using types::Point;
using types::Rect;

class RectangleMath_Translation : public PandaObject
{
public:
	PANDA_CLASS(RectangleMath_Translation, PandaObject)

	RectangleMath_Translation(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("rectangle", "Rectangle to be translated"))
		, translation(initData("translation", "Translation to apply to the rectangle"))
		, result(initData("result", "Result of the translation"))
	{
		addInput(input);
		addInput(translation);

		addOutput(result);
	}

	void update()
	{
		auto res = result.getAccessor();
		res.clear();

		const std::vector<Rect>& rects = input.getValue();
		const std::vector<Point>& trans = translation.getValue();
		int nbR = rects.size(), nbT = trans.size();

		if(nbR && nbT)
		{
			if(nbT < nbR) nbT = 1;
			res.resize(nbR);

			for(int i=0; i<nbR; ++i)
				res[i] = rects[i].translated(trans[i%nbT]);
		}
	}

protected:
	Data< std::vector<Rect> > input, result;
	Data< std::vector<Point> > translation;
};

int RectangleMath_TranslationClass = RegisterObject<RectangleMath_Translation>("Math/Rectangle/Translation")
		.setName("Translate rectangle").setDescription("Translate a rectangle");

//****************************************************************************//

class RectangleMath_Adjust : public PandaObject
{
public:
	PANDA_CLASS(RectangleMath_Adjust, PandaObject)

	RectangleMath_Adjust(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("rectangle", "Rectangle to be translated"))
		, adjust(initData("adjust", "Translation to apply to the sides of the rectangle"))
		, result(initData("result", "Result of the adjustment"))
	{
		addInput(input);
		addInput(adjust);

		addOutput(result);
	}

	void update()
	{
		auto res = result.getAccessor();
		res.clear();

		const std::vector<Rect>& rects = input.getValue();
		const std::vector<Rect>& adj = adjust.getValue();
		int nbR = rects.size(), nbA = adj.size();

		if(nbR && nbA)
		{
			if(nbA < nbR) nbA = 1;
			res.resize(nbR);

			for(int i=0; i<nbR; ++i)
				res[i] = rects[i].adjusted(adj[i%nbA]);
		}
	}

protected:
	Data< std::vector<Rect> > input, adjust, result;
};

int RectangleMath_AdjustClass = RegisterObject<RectangleMath_Adjust>("Math/Rectangle/Adjust")
		.setName("Adjust rectangle").setDescription("Move each side of a rectangle");

//****************************************************************************//

class RectangleMath_Intersection : public PandaObject
{
public:
	PANDA_CLASS(RectangleMath_Intersection, PandaObject)

	RectangleMath_Intersection(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData("input 1", "First rectangle"))
		, inputB(initData("input 2", "Second rectangle"))
		, result(initData("result", "Result of the intersection"))
	{
		addInput(inputA);
		addInput(inputB);

		addOutput(result);
	}

	void update()
	{
		auto res = result.getAccessor();
		res.clear();

		const std::vector<Rect>& rectsA = inputA.getValue();
		const std::vector<Rect>& rectsB = inputB.getValue();
		int nbA = rectsA.size(), nbB = rectsB.size();

		if(nbA)
		{
			if(nbB)
			{	// Intersections of A & B
				res.resize(nbA);
				if(nbB < nbA) nbB = 1;
				for(int i=0; i<nbA; ++i)
					res[i] = rectsA[i] & rectsB[i%nbB];
			}
			else
			{	// Intersections of all rectangles A
				Rect tmp;
				res.resize(1);
				tmp = rectsA[0];
				for(int i=1; i<nbA; ++i)
					tmp &= rectsA[i];
				res[0] = tmp;
			}
		}
	}

protected:
	Data< std::vector<Rect> > inputA, inputB, result;
};

int RectangleMath_IntersectionClass = RegisterObject<RectangleMath_Intersection>("Math/Rectangle/Intersection")
		.setName("Intersection of rectangles").setDescription("Compute the intersection of 2 rectangles");

//****************************************************************************//

class RectangleMath_Union : public PandaObject
{
public:
	PANDA_CLASS(RectangleMath_Union, PandaObject)

	RectangleMath_Union(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData("input 1", "First rectangle"))
		, inputB(initData("input 2", "Second rectangle"))
		, result(initData("result", "Result of the union"))
	{
		addInput(inputA);
		addInput(inputB);

		addOutput(result);
	}

	void update()
	{
		auto res = result.getAccessor();
		res.clear();

		const std::vector<Rect>& rectsA = inputA.getValue();
		const std::vector<Rect>& rectsB = inputB.getValue();
		int nbA = rectsA.size(), nbB = rectsB.size();

		if(nbA)
		{
			if(nbB)
			{	// Unions of A & B
				res.resize(nbA);
				if(nbB < nbA) nbB = 1;
				for(int i=0; i<nbA; ++i)
					res[i] = rectsA[i] | rectsB[i%nbB];
			}
			else
			{	// Unions of all rectangles A
				Rect tmp;
				res.resize(1);
				tmp = rectsA[0];
				for(int i=1; i<nbA; ++i)
					tmp |= rectsA[i];
				res[0] = tmp;
			}
		}
	}

protected:
	Data< std::vector<Rect> > inputA, inputB, result;
};

int RectangleMath_UnionClass = RegisterObject<RectangleMath_Union>("Math/Rectangle/Union")
		.setName("Union of rectangles").setDescription("Compute the union of 2 rectangles");


} // namespace Panda


