#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Gradient.h>

namespace panda {

class GradientsSimple_2Colors : public PandaObject
{
public:
	PANDA_CLASS(GradientsSimple_2Colors, PandaObject)

	GradientsSimple_2Colors(PandaDocument *doc)
		: PandaObject(doc)
		, colorA(initData(&colorA, QColor(0,0,0), "color 1", "Start color of the gradient"))
		, colorB(initData(&colorB, QColor(255,255,255), "color 2", "End color of the gradient"))
		, gradient(initData(&gradient, "gradient", "Gradient created from the 2 input colors"))
	{
		addInput(&colorA);
		addInput(&colorB);

		addOutput(&gradient);
	}

	void update()
	{
		auto grad = gradient.getAccessor();
		grad->clear();
		grad->add(0.0, colorA.getValue());
		grad->add(1.0, colorB.getValue());
		this->cleanDirty();
	}

protected:
	Data<QColor> colorA, colorB;
	Data<types::Gradient> gradient;
};

int GradientsSimple_2ColorsClass = RegisterObject("Generator/Gradient/2 colors").setClass<GradientsSimple_2Colors>().setName("2 colors grad").setDescription("Create a gradient between 2 colors");

} // namespace Panda
