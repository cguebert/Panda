#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/helper/algorithm.h>
#include <panda/helper/EasingFunctions.h>
#include <panda/types/Gradient.h>

namespace panda {

using types::Color;
using types::Gradient;
using types::Point;

template<class T>
class AnimationInterpolation : public PandaObject
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(AnimationInterpolation, T), PandaObject)

	AnimationInterpolation(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData("from", "Start value of the interpolation"))
		, inputB(initData("to", "End value of the interpolation"))
		, progress(initData("progress", "Position inside the interpolation"))
		, result(initData("result", "Result of the interpolation"))
		, mode(initData(0, "mode", "Interpolation mode"))
	{
		addInput(inputA);
		addInput(inputB);
		addInput(progress);
		addInput(mode);

		addOutput(result);

		const auto interpolationNames = helper::EasingFunctions::TypeNames();
		std::string interpolationWidgetData;
		for (const auto& name : interpolationNames)
			interpolationWidgetData += name + ";";
		interpolationWidgetData = interpolationWidgetData.substr(0, interpolationWidgetData.size() - 1);

		mode.setWidget("enum");
		mode.setWidgetData(interpolationWidgetData);
	}

	void update()
	{
		helper::EasingFunctions easingFunc;
		easingFunc.setType(static_cast<helper::EasingFunctions::Type>(mode.getValue()));

		const std::vector<T>& listFrom = inputA.getValue();
		const std::vector<T>& listTo = inputB.getValue();
		const std::vector<float>& listProg = progress.getValue();
		auto listResult = result.getAccessor();
		listResult.clear();

		int nbV = listProg.size();
		int nbP = std::min(listFrom.size(), listTo.size());

		if(nbV && nbP)
		{
			int nb = nbV;
			if(nbV > 1)
			{
				if(nbP != nbV)
					nbP = 1;
			}
			else
				nb = nbP;
			listResult.resize(nb);

			for(int i=0; i<nb; ++i)
			{
				float prog = helper::bound<float>(0.0, listProg[i%nbV], 1.0);
				float amt = easingFunc.valueForProgress(prog);

				listResult[i] = types::interpolate(listFrom[i%nbP], listTo[i%nbP], amt);
			}
		}
	}

protected:
	Data< std::vector<T> > inputA, inputB;
	Data< std::vector<float> > progress;
	Data< std::vector<T> > result;
	Data<int> mode;
};

int AnimationInterpolation_DoubleClass = RegisterObject< AnimationInterpolation<float> >("Animation/Interpolation/Reals").setName("Interpolation reals").setDescription("Interpolate between 2 reals");
int AnimationInterpolation_PointsClass = RegisterObject< AnimationInterpolation<Point> >("Animation/Interpolation/Points").setName("Interpolation points").setDescription("Interpolate between 2 points");
int AnimationInterpolation_ColorClass = RegisterObject< AnimationInterpolation<Color> >("Animation/Interpolation/Colors").setName("Interpolation colors").setDescription("Interpolate between 2 colors");
int AnimationInterpolation_GradientClass = RegisterObject< AnimationInterpolation<Gradient> >("Animation/Interpolation/Gradients").setName("Interpolation gradients").setDescription("Interpolate between 2 gradients");

} // namespace Panda
