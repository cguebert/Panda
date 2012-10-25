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
		, A(initData(&A, 1.0, "a", "Alpha component of the color"))
		, color(initData(&color, "color", "Color created from the 4 components"))
	{
		addInput(&R);
		addInput(&G);
		addInput(&B);
		addInput(&A);

		addOutput(&color);
	}

	void update()
	{
		QColor col;
		double r, g, b, a;
		r = qBound(0.0, R.getValue(), 1.0);
		g = qBound(0.0, G.getValue(), 1.0);
		b = qBound(0.0, B.getValue(), 1.0);
		a = qBound(0.0, A.getValue(), 1.0);
		col.setRgbF(r, g, b, a);
		color.setValue(col);

		this->cleanDirty();
	}

protected:
	Data<double> R, G, B, A;
	Data<QColor> color;
};

int GeneratorColors_ComposeRGBClass = RegisterObject("Generator/Color/From RGB").setClass<GeneratorColors_ComposeRGB>().setDescription("Create a color from red, green and blue components");

class GeneratorColors_DecomposeRGB : public PandaObject
{
public:
	GeneratorColors_DecomposeRGB(PandaDocument *doc)
		: PandaObject(doc)
		, R(initData(&R, "r", "Red component of the color"))
		, G(initData(&G, "g", "Green component of the color"))
		, B(initData(&B, "b", "Blue component of the color"))
		, A(initData(&A, 1.0, "a", "Alpha component of the color"))
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
		QColor col = color.getValue();
		R.setValue(col.redF());
		G.setValue(col.greenF());
		B.setValue(col.blueF());
		A.setValue(col.alphaF());

		this->cleanDirty();
	}

protected:
	Data<double> R, G, B, A;
	Data<QColor> color;
};

int GeneratorColors_DecomposeRGBClass = RegisterObject("Generator/Color/To RGB").setClass<GeneratorColors_DecomposeRGB>().setDescription("Extract red, green and blue components from a color");


} // namespace Panda
