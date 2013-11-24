#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Gradient.h>

namespace panda {

using types::Gradient;

class GeneratorColors_InGradient : public PandaObject
{
public:
	PANDA_CLASS(GeneratorColors_InGradient, PandaObject)

	GeneratorColors_InGradient(PandaDocument *doc)
		: PandaObject(doc)
		, gradient(initData(&gradient, "gradient", "The gradient from which to get the color"))
		, position(initData(&position, "position", "Position in the gradient where to get the color"))
		, color(initData(&color, "color", "Color extracted from the gradient"))
	{
		addInput(&gradient);
		addInput(&position);

		addOutput(&color);
	}

	void update()
	{
		Gradient grad = gradient.getValue();
		const QVector<double>& pos = position.getValue();
		auto colorsList = color.getAccessor();

		int nb = pos.size();
		colorsList.resize(nb);
		for(int i=0; i<nb; ++i)
			colorsList[i] = grad.get(pos[i]);

		this->cleanDirty();
	}

protected:
	Data< Gradient > gradient;
	Data< QVector<double> > position;
	Data< QVector<QColor> > color;
};

int GeneratorColors_InGradientClass = RegisterObject<GeneratorColors_InGradient>("Generator/Color/Color in gradient").setDescription("Get colors in specific places in a gradient");

} // namespace Panda
