#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Gradient.h>

namespace panda {

using types::Color;
using types::Gradient;

class GradientsSimple_2Colors : public PandaObject
{
public:
	PANDA_CLASS(GradientsSimple_2Colors, PandaObject)

	GradientsSimple_2Colors(PandaDocument *doc)
		: PandaObject(doc)
		, colorA(initData(Color::black(), "color 1", "Start color of the gradient"))
		, colorB(initData(Color::white(), "color 2", "End color of the gradient"))
		, extend(initData(0, "extend", "What to do when a position asked is outside the boundaries"))
		, gradient(initData("gradient", "Gradient created from the 2 input colors"))
	{
		addInput(colorA);
		addInput(colorB);
		addInput(extend);

		extend.setWidget("enum");
		extend.setWidgetData("Pad;Repeat;Reflect");

		addOutput(gradient);
	}

	void update()
	{
		auto grad = gradient.getAccessor();
		grad->setExtend(extend.getValue());
		grad->clear();
		grad->add(0.0, colorA.getValue());
		grad->add(1.0, colorB.getValue());
		cleanDirty();
	}

protected:
	Data<Color> colorA, colorB;
	Data<int> extend;
	Data<Gradient> gradient;
};

int GradientsSimple_2ColorsClass = RegisterObject<GradientsSimple_2Colors>("Generator/Gradient/2 colors").setName("2 colors gradient").setDescription("Create a gradient between 2 colors");

} // namespace Panda
