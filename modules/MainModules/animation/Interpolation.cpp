#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Animation.h>

#include <QEasingCurve>

namespace panda {

using types::Color;
using types::Point;

template<class T>
class AnimationInterpolation : public PandaObject
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(AnimationInterpolation, T), PandaObject)

	AnimationInterpolation(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData(&inputA, "from", "Start value of the interpolation"))
		, inputB(initData(&inputB, "to", "End value of the interpolation"))
		, progress(initData(&progress, "progress", "Position inside the interpolation"))
		, result(initData(&result, "result", "Result of the interpolation"))
		, mode(initData(&mode, 0, "mode", "Interpolation mode"))
	{
		addInput(&inputA);
		addInput(&inputB);
		addInput(&progress);
		addInput(&mode);

		addOutput(&result);

		mode.setWidget("enum");
		mode.setWidgetData("Linear"
							";InQuad;OutQuad;InOutQuad;OutInQuad"
							";InCubic;OutCubic;InOutCubic;OutInCubic"
							";InQuart;OutQuart;InOutQuart;OutInQuart"
							";InQuint;OutQuint;InOutQuint;OutInQuint"
							";InSine;OutSine;InOutSine;OutInSine"
							";InExpo;OutExpo;InOutExpo;OutInExpo"
							";InCirc;OutCirc;InOutCirc;OutInCirc"
							";InElastic;OutElastic;InOutElastic;OutInElastic"
							";InBack;OutBack;InOutBack;OutInBack"
							";InBounce;OutBounce;InOutBounce;OutInBounce"
							";InCurve;OutCurve;SineCurve;CosineCurve");
	}

	void update()
	{
		QEasingCurve curve(static_cast<QEasingCurve::Type>(mode.getValue()));

		const QVector<T>& listFrom = inputA.getValue();
		const QVector<T>& listTo = inputB.getValue();
		const QVector<PReal>& listProg = progress.getValue();
		auto listResult = result.getAccessor();
		listResult.clear();

		int nbV = listProg.size();
		int nbP = qMin(listFrom.size(), listTo.size());

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
				PReal prog = qBound<PReal>(0.0, listProg[i%nbV], 1.0);
				PReal amt = curve.valueForProgress(prog);

				listResult[i] = types::interpolate(listFrom[i%nbP], listTo[i%nbP], amt);
			}
		}

		cleanDirty();
	}

protected:
	Data< QVector<T> > inputA, inputB;
	Data< QVector<PReal> > progress;
	Data< QVector<T> > result;
	Data<int> mode;
};

int AnimationInterpolation_DoubleClass = RegisterObject< AnimationInterpolation<PReal> >("Animation/Interpolation/Reals").setName("Interpolation reals").setDescription("Interpolate between 2 reals");
int AnimationInterpolation_PointsClass = RegisterObject< AnimationInterpolation<Point> >("Animation/Interpolation/Points").setName("Interpolation points").setDescription("Interpolate between 2 points");
int AnimationInterpolation_ColorClass = RegisterObject< AnimationInterpolation<Color> >("Animation/Interpolation/Colors").setName("Interpolation colors").setDescription("Interpolate between 2 colors");


} // namespace Panda