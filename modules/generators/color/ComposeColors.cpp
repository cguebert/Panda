#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class GeneratorColors_ComposeRGB : public PandaObject
{
public:
	PANDA_CLASS(GeneratorColors_ComposeRGB, PandaObject)

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

		A.getAccessor().push_back(0.0);

		addOutput(&color);
	}

	void update()
	{
		const QVector<PReal> &r = R.getValue();
		const QVector<PReal> &g = G.getValue();
		const QVector<PReal> &b = B.getValue();
		const QVector<PReal> &a = A.getValue();

		int nbR = r.size(), nbG = g.size(), nbB = b.size(), nbA = a.size();
		int nb = qMax(nbR, qMax(nbG, qMax(nbB, nbA)));
		if(nbR < nb) nbR = 1;
		if(nbG < nb) nbG = 1;
		if(nbB < nb) nbB = 1;
		if(nbA < nb) nbA = 1;
		auto c = color.getAccessor();
		c.resize(nb);
		for(int i=0; i<nb; ++i)
		{
			c[i].setRgbF(qBound<PReal>(0.0, r[i%nbR], 1.0),
						 qBound<PReal>(0.0, g[i%nbG], 1.0),
						 qBound<PReal>(0.0, b[i%nbB], 1.0),
						 qBound<PReal>(0.0, a[i%nbA], 1.0));
		}

		cleanDirty();
	}

protected:
	Data< QVector<PReal> > R, G, B, A;
	Data< QVector<QColor> > color;
};

int GeneratorColors_ComposeRGBClass = RegisterObject<GeneratorColors_ComposeRGB>("Generator/Color/From RGB").setDescription("Create a color from red, green and blue components");

//*************************************************************************//

class GeneratorColors_DecomposeRGB : public PandaObject
{
public:
	PANDA_CLASS(GeneratorColors_DecomposeRGB, PandaObject)

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
		auto r = R.getAccessor();
		auto g = G.getAccessor();
		auto b = B.getAccessor();
		auto a = A.getAccessor();

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

		cleanDirty();
	}

protected:
	Data< QVector<PReal> > R, G, B, A;
	Data< QVector<QColor> > color;
};

int GeneratorColors_DecomposeRGBClass = RegisterObject<GeneratorColors_DecomposeRGB>("Generator/Color/To RGB").setDescription("Extract red, green and blue components from a color");


} // namespace Panda
