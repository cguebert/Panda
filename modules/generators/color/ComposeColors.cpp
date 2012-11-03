#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class GeneratorColors_ComposeRGB : public PandaObject
{
public:
	GeneratorColors_ComposeRGB(PandaDocument *doc)
		: PandaObject(doc)
		, R(initData(&R, "r", "Red component of the color"))
		, G(initData(&G, "g", "Green component of the color"))
		, B(initData(&B, "b", "Blue component of the color"))
		, A(initData(&A, "a", "Alpha component of the color"))
		, color(initData(&color, "color", "Color created from the 4 components"))
	{
		addInput(&R);
		addInput(&G);
		addInput(&B);
		addInput(&A);

		A.beginEdit()->append(1.0);
		A.endEdit();

		addOutput(&color);
	}

	void update()
	{
		const QVector<double> &r = R.getValue();
		const QVector<double> &g = G.getValue();
		const QVector<double> &b = B.getValue();
		const QVector<double> &a = A.getValue();

		int nb = qMin(r.size(), qMin(g.size(), qMin(b.size(), a.size())));
		QVector<QColor>& c = *color.beginEdit();
		c.resize(nb);
		for(int i=0; i<nb; ++i)
		{
			c[i].setRgbF(qBound(0.0, r[i], 1.0),
						 qBound(0.0, g[i], 1.0),
						 qBound(0.0, b[i], 1.0),
						 qBound(0.0, a[i], 1.0));
		}

		color.endEdit();
		this->cleanDirty();
	}

protected:
	Data< QVector<double> > R, G, B, A;
	Data< QVector<QColor> > color;
};

int GeneratorColors_ComposeRGBClass = RegisterObject("Generator/Color/From RGB").setClass<GeneratorColors_ComposeRGB>().setDescription("Create a color from red, green and blue components");

//*************************************************************************//

class GeneratorColors_DecomposeRGB : public PandaObject
{
public:
	GeneratorColors_DecomposeRGB(PandaDocument *doc)
		: PandaObject(doc)
		, R(initData(&R, "r", "Red component of the color"))
		, G(initData(&G, "g", "Green component of the color"))
		, B(initData(&B, "b", "Blue component of the color"))
		, A(initData(&A, "a", "Alpha component of the color"))
		, color(initData(&color, "color", "Color created from the 4 components"))
	{
		addInput(&color);

		addOutput(&R);
		addOutput(&G);
		addOutput(&B);
		addOutput(&A);
	}

	void update()
	{
		const QVector<QColor>& c = color.getValue();
		QVector<double> &r = *R.beginEdit();
		QVector<double> &g = *G.beginEdit();
		QVector<double> &b = *B.beginEdit();
		QVector<double> &a = *A.beginEdit();

		int nb = c.size();
		r.resize(nb);
		g.resize(nb);
		b.resize(nb);
		a.resize(nb);

		for(int i=0; i<nb; ++i)
		{
			r[i] = c[i].redF();
			g[i] = c[i].greenF();
			b[i] = c[i].blueF();
			a[i] = c[i].alphaF();
		}

		R.endEdit();
		G.endEdit();
		B.endEdit();
		A.endEdit();
		this->cleanDirty();
	}

protected:
	Data< QVector<double> > R, G, B, A;
	Data< QVector<QColor> > color;
};

int GeneratorColors_DecomposeRGBClass = RegisterObject("Generator/Color/To RGB").setClass<GeneratorColors_DecomposeRGB>().setDescription("Extract red, green and blue components from a color");


} // namespace Panda
