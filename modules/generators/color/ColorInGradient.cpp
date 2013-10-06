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
		, extend(initData(&extend, 0, "extend", "Method to use when a position is outside the gradient's boundary"))
		, color(initData(&color, "color", "Color extracted from the gradient"))
	{
		addInput(&gradient);
		addInput(&position);
		addInput(&extend);

		addOutput(&color);
	}

	void update()
	{
		Gradient grad = gradient.getValue();
		const QVector<double>& pos = position.getValue();
		grad.setExtend(extend.getValue());
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
	Data< int > extend;
	Data< QVector<QColor> > color;
};

int GeneratorColors_InGradientClass = RegisterObject("Generator/Color/Color in gradient").setClass<GeneratorColors_InGradient>().setName("Gradient color").setDescription("Get colors in specific places in a gradient");

} // namespace Panda
