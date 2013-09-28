#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Animation.h>

#include <QPointF>
#include <QEasingCurve>

namespace panda {

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
	}

	void update()
	{
		QEasingCurve curve((QEasingCurve::Type)mode.getValue());

		const QVector<T>& listFrom = inputA.getValue();
		const QVector<T>& listTo = inputB.getValue();
		const QVector<double>& listProg = progress.getValue();
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
				double prog = qBound(0.0, listProg[i%nbV], 1.0);
				double amt = curve.valueForProgress(prog);

				listResult[i] = lerp(listFrom[i%nbP], listTo[i%nbP], amt);
			}
		}

		this->cleanDirty();
	}

protected:
	Data< QVector<T> > inputA, inputB;
	Data< QVector<double> > progress;
	Data< QVector<T> > result;
	Data<int> mode;
};

int AnimationInterpolation_DoubleClass = RegisterObject("Animation/Interpolation/Reals").setClass< AnimationInterpolation<double> >().setName("Inter reals").setDescription("Interpolate between 2 reals");
int AnimationInterpolation_PointsClass = RegisterObject("Animation/Interpolation/Points").setClass< AnimationInterpolation<QPointF> >().setName("Inter points").setDescription("Interpolate between 2 points");
int AnimationInterpolation_ColorClass = RegisterObject("Animation/Interpolation/Colors").setClass< AnimationInterpolation<QColor> >().setName("Inter colors").setDescription("Interpolate between 2 colors");


} // namespace Panda
