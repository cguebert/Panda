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
		const QVector<double> &l = left.getValue();
		const QVector<double> &t = top.getValue();
		const QVector<double> &r = right.getValue();
		const QVector<double> &b = bottom.getValue();

		int nb = qMin(l.size(), qMin(t.size(), qMin(r.size(), b.size())));

		QVector<QRectF>& rect = *rectangle.beginEdit();
		rect.resize(nb);

		for(int i=0; i<nb; ++i)
		{
			double tl = l[i], tt = t[i];
			rect[i] = QRectF(tl, tt, r[i] - tl, b[i] - tt);
		}

		rectangle.endEdit();
		this->cleanDirty();
	}

protected:
	Data< QVector<double> > left, top, right, bottom;
	Data< QVector<QRectF> > rectangle;
};

int GeneratorRectangles_ComposeClass = RegisterObject("Generator/Rectangle/Rectangle from 4 reals").setName("Reals to rect").setClass<GeneratorRectangles_Compose>().setDescription("Create a rectangle from 4 reals");

//*************************************************************************//

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
		const QVector<QPointF> &c = center.getValue();
		const QVector<QPointF> &s = size.getValue();
		int nb = qMin(c.size(), s.size());

		QVector<QRectF>& rect = *rectangle.beginEdit();
		rect.resize(nb);

		for(int i=0; i<nb; ++i)
			rect[i] = QRectF(c[i].x()-s[i].x()/2.0, c[i].y()-s[i].y()/2.0, s[i].x(), s[i].y());

		rectangle.endEdit();
		this->cleanDirty();
	}

protected:
	Data< QVector<QPointF> > center, size;
	Data< QVector<QRectF> > rectangle;
};

int GeneratorRectangles_ComposeCenterClass = RegisterObject("Generator/Rectangle/Rectangle from center and size").setName("Points to rect").setClass<GeneratorRectangles_ComposeCenter>().setDescription("Create a rectangle from center and size");

//*************************************************************************//

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
		const QVector<QRectF> &rect = rectangle.getValue();
		int nb = rect.size();

		QVector<double> &l = *left.beginEdit();
		QVector<double> &t = *top.beginEdit();
		QVector<double> &r = *right.beginEdit();
		QVector<double> &b = *bottom.beginEdit();

		l.resize(nb);
		t.resize(nb);
		r.resize(nb);
		b.resize(nb);

		for(int i=0; i<nb; ++i)
		{
			const QRectF& tr = rect[i];
			l[i] = tr.left();
			t[i] = tr.top();
			r[i] = tr.right();
			b[i] = tr.bottom();
		}

		left.endEdit();
		top.endEdit();
		right.endEdit();
		bottom.endEdit();
		this->cleanDirty();
	}

protected:
	Data< QVector<double> > left, top, right, bottom;
	Data< QVector<QRectF> > rectangle;
};

int GeneratorRectangles_DecomposeClass = RegisterObject("Generator/Rectangle/Rectangle to 4 reals").setName("Rect to reals").setClass<GeneratorRectangles_Decompose>().setDescription("Extract the boundary of a rectangles");

//*************************************************************************//

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
		const QVector<QRectF> &rect = rectangle.getValue();
		int nb = rect.size();

		QVector<QPointF> &c = *center.beginEdit();
		QVector<QPointF> &s = *size.beginEdit();

		c.resize(nb);
		s.resize(nb);

		for(int i=0; i<nb; ++i)
		{
			const QRectF& tr = rect[i];
			c[i] = tr.center();
			s[i] = QPointF(tr.width(), tr.height());
		}

		center.endEdit();
		size.endEdit();
		this->cleanDirty();
	}

protected:
	Data< QVector<QPointF> > center, size;
	Data< QVector<QRectF> > rectangle;
};

int GeneratorRectangles_DecomposeCenterClass = RegisterObject("Generator/Rectangle/Rectangle to center and size").setName("Rect to points").setClass<GeneratorRectangles_DecomposeCenter>().setDescription("Extract the center and size of a rectangle");

} // namespace Panda
