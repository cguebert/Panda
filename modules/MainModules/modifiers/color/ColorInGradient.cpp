#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Gradient.h>

namespace panda {

using types::Color;
using types::Gradient;

class ModifierColor_InGradient : public PandaObject
{
public:
	PANDA_CLASS(ModifierColor_InGradient, PandaObject)

	ModifierColor_InGradient(PandaDocument *doc)
		: PandaObject(doc)
		, gradient(initData("gradient", "The gradient from which to get the color"))
		, position(initData("position", "Position in the gradient where to get the color"))
		, color(initData("color", "Color extracted from the gradient"))
	{
		addInput(gradient);
		addInput(position);

		addOutput(color);
	}

	void update()
	{
		Gradient grad = gradient.getValue();
		const std::vector<float>& pos = position.getValue();
		auto colorsList = color.getAccessor();

		int nb = pos.size();
		colorsList.resize(nb);
		for(int i=0; i<nb; ++i)
			colorsList[i] = grad.get(pos[i]);

		cleanDirty();
	}

protected:
	Data< Gradient > gradient;
	Data< std::vector<float> > position;
	Data< std::vector<Color> > color;
};

int ModifierColor_InGradientClass = RegisterObject<ModifierColor_InGradient>("Modifier/Color/Color in gradient").setDescription("Get colors in specific places in a gradient");

} // namespace Panda
