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

        progress.beginEdit()->append(0.0);
        progress.endEdit();
    }

    void update()
    {
        QEasingCurve curve((QEasingCurve::Type)mode.getValue());

        const QVector<T>& listFrom = inputA.getValue();
        const QVector<T>& listTo = inputB.getValue();
        int inputSize = qMin(listFrom.size(), listTo.size());

        const QVector<double>& listProg = progress.getValue();
        int progSize = listProg.size();

        QVector<T>& listResult = *result.beginEdit();
        listResult.resize(inputSize * progSize);

        for(int i=0; i<progSize; ++i)
        {
            double prog = qBound(0.0, listProg[i], 1.0);
            double amt = curve.valueForProgress(prog);

            for(int j=0; j<inputSize; ++j)
                listResult[i*inputSize+j] = lerp(listFrom[j], listTo[j], amt);
        }

        result.endEdit();
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
