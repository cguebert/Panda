#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Color.h>

namespace panda {

using types::Color;

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
		addInput(R);
		addInput(G);
		addInput(B);
		addInput(A);

		A.getAccessor().push_back(1.0);

		addOutput(color);
	}

	void update()
	{
		const QVector<PReal> &r = R.getValue();
		const QVector<PReal> &g = G.getValue();
		const QVector<PReal> &b = B.getValue();
		const QVector<PReal> &a = A.getValue();
		auto c = color.getAccessor();

		int nbR = r.size(), nbG = g.size(), nbB = b.size(), nbA = a.size();
		if(!nbR || !nbG || !nbB || !nbA)
		{
			c.clear();
			cleanDirty();
			return;
		}

		int nb = qMax(nbR, qMax(nbG, qMax(nbB, nbA)));
		if(nbR < nb) nbR = 1;
		if(nbG < nb) nbG = 1;
		if(nbB < nb) nbB = 1;
		if(nbA < nb) nbA = 1;

		c.resize(nb);
		for(int i=0; i<nb; ++i)
		{
			c[i].set(qBound<float>(0.0f, r[i%nbR], 1.0f),
					 qBound<float>(0.0f, g[i%nbG], 1.0f),
					 qBound<float>(0.0f, b[i%nbB], 1.0f),
					 qBound<float>(0.0f, a[i%nbA], 1.0f));
		}

		cleanDirty();
	}

protected:
	Data< QVector<PReal> > R, G, B, A;
	Data< QVector<Color> > color;
};

int GeneratorColors_ComposeRGBClass = RegisterObject<GeneratorColors_ComposeRGB>("Generator/Color/From RGB").setDescription("Create a color from red, green and blue components");

//****************************************************************************//

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
		addInput(color);

		addOutput(R);
		addOutput(G);
		addOutput(B);
		addOutput(A);
	}

	void update()
	{
		const QVector<Color>& c = color.getValue();
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
			c[i].get(r[i], g[i], b[i], a[i]);

		cleanDirty();
	}

protected:
	Data< QVector<PReal> > R, G, B, A;
	Data< QVector<Color> > color;
};

int GeneratorColors_DecomposeRGBClass = RegisterObject<GeneratorColors_DecomposeRGB>("Generator/Color/To RGB").setDescription("Extract red, green and blue components from a color");

//****************************************************************************//

class GeneratorColors_ComposeHSV : public PandaObject
{
public:
	PANDA_CLASS(GeneratorColors_ComposeHSV, PandaObject)

	GeneratorColors_ComposeHSV(PandaDocument *doc)
		: PandaObject(doc)
		, H(initData(&H, "h", "Hue component of the color"))
		, S(initData(&S, "s", "Saturation component of the color"))
		, V(initData(&V, "v", "Value component of the color"))
		, A(initData(&A, "a", "Alpha component of the color"))
		, color(initData(&color, "color", "Color created from the 4 components"))
	{
		addInput(H);
		addInput(S);
		addInput(V);
		addInput(A);

		A.getAccessor().push_back(1.0);

		addOutput(color);
	}

	void update()
	{
		const QVector<PReal> &h = H.getValue();
		const QVector<PReal> &s = S.getValue();
		const QVector<PReal> &v = V.getValue();
		const QVector<PReal> &a = A.getValue();
		auto c = color.getAccessor();

		int nbH = h.size(), nbS = s.size(), nbV = v.size(), nbA = a.size();
		if(!nbH || !nbS || !nbV || !nbA)
		{
			c.clear();
			cleanDirty();
			return;
		}

		int nb = qMax(nbH, qMax(nbS, qMax(nbV, nbA)));
		if(nbH < nb) nbH = 1;
		if(nbS < nb) nbS = 1;
		if(nbV < nb) nbV = 1;
		if(nbA < nb) nbA = 1;

		c.resize(nb);
		for(int i=0; i<nb; ++i)
		{
			c[i] = Color::fromHsv(
					qBound<float>(0.0f, h[i%nbH], 1.0f),
					qBound<float>(0.0f, s[i%nbS], 1.0f),
					qBound<float>(0.0f, v[i%nbV], 1.0f),
					qBound<float>(0.0f, a[i%nbA], 1.0f));
		}

		cleanDirty();
	}

protected:
	Data< QVector<PReal> > H, S, V, A;
	Data< QVector<Color> > color;
};

int GeneratorColors_ComposeHSVClass = RegisterObject<GeneratorColors_ComposeHSV>("Generator/Color/From HSV").setDescription("Create a color from hue, saturation and value components");

//****************************************************************************//

class GeneratorColors_DecomposeHSV : public PandaObject
{
public:
	PANDA_CLASS(GeneratorColors_DecomposeHSV, PandaObject)

	GeneratorColors_DecomposeHSV(PandaDocument *doc)
		: PandaObject(doc)
		, H(initData(&H, "h", "Hue component of the color"))
		, S(initData(&S, "s", "Saturation component of the color"))
		, V(initData(&V, "v", "Value component of the color"))
		, A(initData(&A, "a", "Alpha component of the color"))
		, color(initData(&color, "color", "Color created from the 4 components"))
	{
		addInput(color);

		addOutput(H);
		addOutput(S);
		addOutput(V);
		addOutput(A);
	}

	void update()
	{
		const QVector<Color>& c = color.getValue();
		auto h = H.getAccessor();
		auto s = S.getAccessor();
		auto v = V.getAccessor();
		auto a = A.getAccessor();

		int nb = c.size();
		h.resize(nb);
		s.resize(nb);
		v.resize(nb);
		a.resize(nb);

		for(int i=0; i<nb; ++i)
			c[i].getHsv(h[i], s[i], v[i], a[i]);

		cleanDirty();
	}

protected:
	Data< QVector<PReal> > H, S, V, A;
	Data< QVector<Color> > color;
};

int GeneratorColors_DecomposeHSVClass = RegisterObject<GeneratorColors_DecomposeHSV>("Generator/Color/To HSV").setDescription("Extract hue, saturation and value components from a color");


} // namespace Panda
