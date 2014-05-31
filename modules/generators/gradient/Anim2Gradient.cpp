#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Gradient.h>
#include <panda/types/Animation.h>

namespace panda {

using types::Color;
using types::Gradient;
using types::Animation;

class Anim2Gradient : public PandaObject
{
public:
	PANDA_CLASS(Anim2Gradient, PandaObject)

	Anim2Gradient(PandaDocument *doc)
		: PandaObject(doc)
		, anim(initData(&anim, "animation", "The animation from which to create a gradient"))
		, gradient(initData(&gradient, "gradient", "The gradient converted from the animation"))
	{
		addInput(&anim);

		addOutput(&gradient);
	}

	void update()
	{
		const Animation<Color>& colors = anim.getValue();
		auto grad = gradient.getAccessor();
		grad->clear();
		for(auto stop : colors.getStops())
		{
			if(stop.first >= 0 && stop.first <= 1)
				grad->add(stop.first, stop.second);
		}

		cleanDirty();
	}

protected:
	Data< Animation<Color> > anim;
	Data< Gradient > gradient;
};

int Anim2GradientClass = RegisterObject<Anim2Gradient>("Generator/Gradient/Animation to gradient").setDescription("Convert an animation of colors to a gradient");

//****************************************************************************//

class Gradient2Anim : public PandaObject
{
public:
	PANDA_CLASS(Gradient2Anim, PandaObject)

	Gradient2Anim(PandaDocument *doc)
		: PandaObject(doc)
		, gradient(initData(&gradient, "gradient", "The gradient from which to create an animation"))
		, anim(initData(&anim, "animation", "The animation converted from the gradient"))
	{
		addInput(&gradient);

		addOutput(&anim);
	}

	void update()
	{
		const Gradient& grad = gradient.getValue();
		auto acc = anim.getAccessor();
		acc->setStops(grad.getStops());

		cleanDirty();
	}

protected:
	Data< Gradient > gradient;
	Data< Animation<Color> > anim;
};

int Gradient2AnimClass = RegisterObject<Gradient2Anim>("Generator/Gradient/Gradient to animation").setDescription("Convert gradient to an animation of colors");


} // namespace Panda
