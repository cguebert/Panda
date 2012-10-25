#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class GeneratorRectangles_Compose : public PandaObject
{
public:
	GeneratorRectangles_Compose(PandaDocument *doc)
		: PandaObject(doc)
		, left(initData(&left, "left", "Left boundary of the rectangle"))
		, top(initData(&top, "top", "Top boundary of the rectangle"))
		, right(initData(&right, "right", "Right boundary of the rectangle"))
		, bottom(initData(&bottom, "bottom", "Bottom boundary of the rectangle"))
		, rectangle(initData(&rectangle, "rectangle", "Rectangle created from the 4 reals"))
	{
		addInput(&left);
		addInput(&top);
		addInput(&right);
		addInput(&bottom);

		addOutput(&rectangle);
	}

	void update()
	{
		double l, t, w, h;
		l = left.getValue();
		t = top.getValue();
		w = right.getValue() - l;
		h = bottom.getValue() - t;
		rectangle.setValue(QRectF(l, t, w, h));

		this->cleanDirty();
	}

protected:
	Data<double> left, top, right, bottom;
	Data<QRectF> rectangle;
};

int GeneratorRectangles_ComposeClass = RegisterObject("Generator/Rectangle/Rectangle from 4 reals").setName("Reals to rect").setClass<GeneratorRectangles_Compose>().setDescription("Create a rectangle from 4 reals");

class GeneratorRectangles_ComposeCenter : public PandaObject
{
public:
	GeneratorRectangles_ComposeCenter(PandaDocument *doc)
		: PandaObject(doc)
		, center(initData(&center, "center", "Center of the rectangle"))
		, size(initData(&size, "size", "Size of the rectangle"))
		, rectangle(initData(&rectangle, "rectangle", "Rectangle created from the 2 points"))
	{
		addInput(&center);
		addInput(&size);

		addOutput(&rectangle);
	}

	void update()
	{
		QPointF c = center.getValue();
		QPointF s = size.getValue();
		rectangle.setValue(QRectF(c.x()-s.x()/2.0, c.y()-s.y()/2.0, s.x(), s.y()));

		this->cleanDirty();
	}

protected:
	Data<QPointF> center, size;
	Data<QRectF> rectangle;
};

int GeneratorRectangles_ComposeCenterClass = RegisterObject("Generator/Rectangle/Rectangle from center and size").setName("Points to rect").setClass<GeneratorRectangles_ComposeCenter>().setDescription("Create a rectangle from center and size");

class GeneratorRectangles_Decompose : public PandaObject
{
public:
	GeneratorRectangles_Decompose(PandaDocument *doc)
		: PandaObject(doc)
		, left(initData(&left, "left", "Left boundary of the rectangle"))
		, top(initData(&top, "top", "Top boundary of the rectangle"))
		, right(initData(&right, "right", "Right boundary of the rectangle"))
		, bottom(initData(&bottom, "bottom", "Bottom boundary of the rectangle"))
		, rectangle(initData(&rectangle, "rectangle", "Rectangle from whith to extract the 4 reals"))
	{
		addInput(&rectangle);

		addOutput(&left);
		addOutput(&top);
		addOutput(&right);
		addOutput(&bottom);
	}

	void update()
	{
		QRectF rect = rectangle.getValue();
		left.setValue(rect.left());
		top.setValue(rect.top());
		right.setValue(rect.right());
		bottom.setValue(rect.bottom());

		this->cleanDirty();
	}

protected:
	Data<double> left, top, right, bottom;
	Data<QRectF> rectangle;
};

int GeneratorRectangles_DecomposeClass = RegisterObject("Generator/Rectangle/Rectangle to 4 reals").setName("Rect to reals").setClass<GeneratorRectangles_Decompose>().setDescription("Extract the boundary of a rectangles");

class GeneratorRectangles_DecomposeCenter : public PandaObject
{
public:
	GeneratorRectangles_DecomposeCenter(PandaDocument *doc)
		: PandaObject(doc)
		, center(initData(&center, "center", "Center of the rectangle"))
		, size(initData(&size, "size", "Size of the rectangle"))
		, rectangle(initData(&rectangle, "rectangle", "Rectangle created from the 2 points"))
	{
		addInput(&rectangle);

		addOutput(&center);
		addOutput(&size);
	}

	void update()
	{
		QRectF rect = rectangle.getValue();
		center.setValue(rect.center());
		size.setValue(QPointF(rect.width(), rect.height()));

		this->cleanDirty();
	}

protected:
	Data<QPointF> center, size;
	Data<QRectF> rectangle;
};

int GeneratorRectangles_DecomposeCenterClass = RegisterObject("Generator/Rectangle/Rectangle to center and size").setName("Rect to points").setClass<GeneratorRectangles_DecomposeCenter>().setDescription("Extract the center and size of a rectangle");

} // namespace Panda
