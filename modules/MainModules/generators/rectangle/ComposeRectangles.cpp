#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Rect.h>

namespace panda {

using types::Point;
using types::Rect;

class GeneratorRectangles_Compose : public PandaObject
{
public:
	PANDA_CLASS(GeneratorRectangles_Compose, PandaObject)

	GeneratorRectangles_Compose(PandaDocument *doc)
		: PandaObject(doc)
		, left(initData("left", "Left boundary of the rectangle"))
		, top(initData("top", "Top boundary of the rectangle"))
		, right(initData("right", "Right boundary of the rectangle"))
		, bottom(initData("bottom", "Bottom boundary of the rectangle"))
		, rectangle(initData("rectangle", "Rectangle created from the 4 reals"))
	{
		addInput(left);
		addInput(top);
		addInput(right);
		addInput(bottom);

		addOutput(rectangle);
	}

	void update()
	{
		const std::vector<PReal> &l = left.getValue();
		const std::vector<PReal> &t = top.getValue();
		const std::vector<PReal> &r = right.getValue();
		const std::vector<PReal> &b = bottom.getValue();

		int nb = std::min(l.size(), std::min(t.size(), std::min(r.size(), b.size())));

		auto rect = rectangle.getAccessor();
		rect.resize(nb);

		for(int i=0; i<nb; ++i)
			rect[i] = Rect(l[i], t[i], r[i], b[i]);

		cleanDirty();
	}

protected:
	Data< std::vector<PReal> > left, top, right, bottom;
	Data< std::vector<Rect> > rectangle;
};

int GeneratorRectangles_ComposeClass = RegisterObject<GeneratorRectangles_Compose>("Generator/Rectangle/Rectangle from 4 reals").setName("Reals to rectangle").setDescription("Create a rectangle from 4 reals");

//****************************************************************************//

class GeneratorRectangles_ComposeCenter : public PandaObject
{
public:
	PANDA_CLASS(GeneratorRectangles_ComposeCenter, PandaObject)

	GeneratorRectangles_ComposeCenter(PandaDocument *doc)
		: PandaObject(doc)
		, center(initData("center", "Center of the rectangle"))
		, size(initData("size", "Size of the rectangle"))
		, rectangle(initData("rectangle", "Rectangle created from the 2 points"))
	{
		addInput(center);
		addInput(size);

		addOutput(rectangle);
	}

	void update()
	{
		const std::vector<Point> &c = center.getValue();
		const std::vector<Point> &s = size.getValue();

		auto rect = rectangle.getAccessor();
		rect.clear();

		int nbC = c.size();
		int nbS = s.size();

		if(nbC && nbS)
		{
			if(nbS < nbC) nbS = 1;
			if(nbC < nbS) nbC = 1;

			int nb = std::max(nbC, nbS);
			rect.resize(nb);

			for(int i=0; i<nb; ++i)
				rect[i] = Rect(c[i%nbC]-s[i%nbS]/2, s[i%nbS].x, s[i%nbS].y);
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > center, size;
	Data< std::vector<Rect> > rectangle;
};

int GeneratorRectangles_ComposeCenterClass = RegisterObject<GeneratorRectangles_ComposeCenter>("Generator/Rectangle/Rectangle from center and size").setName("Center to rectangle").setDescription("Create a rectangle from center and size");

//****************************************************************************//

class GeneratorRectangles_ComposeCorners : public PandaObject
{
public:
	PANDA_CLASS(GeneratorRectangles_ComposeCorners, PandaObject)

	GeneratorRectangles_ComposeCorners(PandaDocument *doc)
		: PandaObject(doc)
		, topleft(initData("top-left", "Top left corner of the rectangle"))
		, bottomright(initData("bottom-right", "Bottom right corner of the rectangle"))
		, rectangle(initData("rectangle", "Rectangle created from the 2 points"))
	{
		addInput(topleft);
		addInput(bottomright);

		addOutput(rectangle);
	}

	void update()
	{
		const std::vector<Point> &tl = topleft.getValue();
		const std::vector<Point> &br = bottomright.getValue();

		auto rect = rectangle.getAccessor();
		rect.clear();

		int nbTL = tl.size();
		int nbBR = br.size();

		if(nbTL && nbBR)
		{
			if(nbTL < nbBR) nbTL = 1;
			if(nbBR < nbTL) nbBR = 1;

			int nb = std::max(nbTL, nbBR);
			rect.resize(nb);

			for(int i=0; i<nb; ++i)
				rect[i] = Rect(tl[i%nbTL], br[i%nbBR]);
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > topleft, bottomright;
	Data< std::vector<Rect> > rectangle;
};

int GeneratorRectangles_ComposeCornersClass = RegisterObject<GeneratorRectangles_ComposeCorners>("Generator/Rectangle/Rectangle from corners").setName("Corners to rectangle").setDescription("Create a rectangle from 2 corners");

//****************************************************************************//

class GeneratorRectangles_Decompose : public PandaObject
{
public:
	PANDA_CLASS(GeneratorRectangles_Decompose, PandaObject)

	GeneratorRectangles_Decompose(PandaDocument *doc)
		: PandaObject(doc)
		, left(initData("left", "Left boundary of the rectangle"))
		, top(initData("top", "Top boundary of the rectangle"))
		, right(initData("right", "Right boundary of the rectangle"))
		, bottom(initData("bottom", "Bottom boundary of the rectangle"))
		, rectangle(initData("rectangle", "Rectangle from whith to extract the 4 reals"))
	{
		addInput(rectangle);

		addOutput(left);
		addOutput(top);
		addOutput(right);
		addOutput(bottom);
	}

	void update()
	{
		const std::vector<Rect> &rect = rectangle.getValue();
		int nb = rect.size();

		auto l = left.getAccessor();
		auto t = top.getAccessor();
		auto r = right.getAccessor();
		auto b = bottom.getAccessor();

		l.resize(nb);
		t.resize(nb);
		r.resize(nb);
		b.resize(nb);

		for(int i=0; i<nb; ++i)
		{
			const Rect& tr = rect[i];
			l[i] = tr.left();
			t[i] = tr.top();
			r[i] = tr.right();
			b[i] = tr.bottom();
		}

		cleanDirty();
	}

protected:
	Data< std::vector<PReal> > left, top, right, bottom;
	Data< std::vector<Rect> > rectangle;
};

int GeneratorRectangles_DecomposeClass = RegisterObject<GeneratorRectangles_Decompose>("Generator/Rectangle/Rectangle to 4 reals").setName("Rectangle to reals").setDescription("Extract the boundary of a rectangles");

//****************************************************************************//

class GeneratorRectangles_DecomposeCenter : public PandaObject
{
public:
	PANDA_CLASS(GeneratorRectangles_DecomposeCenter, PandaObject)

	GeneratorRectangles_DecomposeCenter(PandaDocument *doc)
		: PandaObject(doc)
		, center(initData("center", "Center of the rectangle"))
		, size(initData("size", "Size of the rectangle"))
		, rectangle(initData("rectangle", "Rectangle created from the 2 points"))
	{
		addInput(rectangle);

		addOutput(center);
		addOutput(size);
	}

	void update()
	{
		const std::vector<Rect> &rect = rectangle.getValue();
		int nb = rect.size();

		auto c = center.getAccessor();
		auto s = size.getAccessor();

		c.resize(nb);
		s.resize(nb);

		for(int i=0; i<nb; ++i)
		{
			const Rect& tr = rect[i];
			c[i] = tr.center();
			s[i] = Point(tr.width(), tr.height());
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > center, size;
	Data< std::vector<Rect> > rectangle;
};

int GeneratorRectangles_DecomposeCenterClass = RegisterObject<GeneratorRectangles_DecomposeCenter>("Generator/Rectangle/Rectangle to center and size").setName("Rectangle to center").setDescription("Extract the center and size of a rectangle");

//****************************************************************************//

class GeneratorRectangles_DecomposeCorners : public PandaObject
{
public:
	PANDA_CLASS(GeneratorRectangles_DecomposeCorners, PandaObject)

	GeneratorRectangles_DecomposeCorners(PandaDocument *doc)
		: PandaObject(doc)
		, topleft(initData("top-left", "Top left corner of the rectangle"))
		, bottomright(initData("bottom-right", "Bottom right of the rectangle"))
		, rectangle(initData("rectangle", "Rectangle created from the 2 points"))
	{
		addInput(rectangle);

		addOutput(topleft);
		addOutput(bottomright);
	}

	void update()
	{
		const std::vector<Rect> &rect = rectangle.getValue();
		int nb = rect.size();

		auto tl = topleft.getAccessor();
		auto br = bottomright.getAccessor();

		tl.resize(nb);
		br.resize(nb);

		for(int i=0; i<nb; ++i)
		{
			const Rect& tr = rect[i];
			tl[i] = tr.topLeft();
			br[i] = tr.bottomRight();
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > topleft, bottomright;
	Data< std::vector<Rect> > rectangle;
};

int GeneratorRectangles_DecomposeCornersClass = RegisterObject<GeneratorRectangles_DecomposeCorners>("Generator/Rectangle/Rectangle to corners").setName("Rectangle to corners").setDescription("Extract two corners of a rectangle");

} // namespace Panda
