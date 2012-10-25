#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QVector>

#define _USE_MATH_DEFINES
#include <math.h>

namespace panda {

class PointMath_Addition : public PandaObject
{
public:
    PointMath_Addition(PandaDocument *doc)
        : PandaObject(doc)
        , inputA(initData(&inputA, "point 1", "First point"))
        , inputB(initData(&inputB, "point 2", "Second point"))
        , result(initData(&result, "result", "Result of the addition of the 2 points"))
    {
        addInput(&inputA);
        addInput(&inputB);

        addOutput(&result);
    }

    void update()
    {
        const QVector<QPointF>& valA = inputA.getValue();
        const QVector<QPointF>& valB = inputB.getValue();
        int nb = qMin(valA.size(), valB.size());

        QVector<QPointF>& res = *result.beginEdit();
        res.clear();
        res.resize(nb);

        for(int i=0; i<nb; ++i)
            res[i] = valA[i] + valB[i];

        result.endEdit();
        this->cleanDirty();
    }

protected:
    Data< QVector<QPointF> > inputA, inputB, result;
};

int PointMath_AdditionClass = RegisterObject("Math/Point/Addition").setClass<PointMath_Addition>().setName("Add points").setDescription("Compute the addition of 2 points");

//*************************************************************************//

class PointMath_Scale : public PandaObject
{
public:
    PointMath_Scale(PandaDocument *doc)
        : PandaObject(doc)
        , input(initData(&input, "point", "Point value to multiply"))
        , result(initData(&result, "result", "Result of the multiplication of the point by the real"))
        , factor(initData(&factor, 1.0, "factor", "Real by which to multiply the point"))
    {
        addInput(&input);
        addInput(&factor);

        addOutput(&result);
    }

    void update()
    {
        const QVector<QPointF>& points = input.getValue();
        int nb = points.size();

        QVector<QPointF>& res = *result.beginEdit();
        res.clear();
        res.resize(nb);

        double f = factor.getValue();
        for(int i=0; i<nb; ++i)
            res[i] = points[i] * f;

        result.endEdit();
        this->cleanDirty();
    }

protected:
    Data< QVector<QPointF> > input, result;
    Data< double > factor;
};

int PointMath_ScaleClass = RegisterObject("Math/Point/Multiply").setClass<PointMath_Scale>().setName("Scale point").setDescription("Multiply a point by a real");

//*************************************************************************//

class PointMath_Distance : public PandaObject
{
public:
    PointMath_Distance(PandaDocument *doc)
        : PandaObject(doc)
        , inputA(initData(&inputA, "point 1", "First point"))
        , inputB(initData(&inputB, "point 2", "Second point"))
        , result(initData(&result, "distance", "Distance between the 2 points"))
    {
        addInput(&inputA);
        addInput(&inputB);

        addOutput(&result);
    }

    void update()
    {
        const QVector<QPointF>& valA = inputA.getValue();
        const QVector<QPointF>& valB = inputB.getValue();
        int nb = qMin(valA.size(), valB.size());

        QVector<double>& res = *result.beginEdit();
        res.clear();
        res.resize(nb);

        for(int i=0; i<nb; ++i)
        {
            const QPointF& ptA = valA[i], ptB = valB[i];
            double dx = ptA.x()-ptB.x(), dy = ptA.y()-ptB.y();
            res[i] = sqrt(dx*dx+dy*dy);
        }

        result.endEdit();
        this->cleanDirty();
    }

protected:
    Data< QVector<QPointF> > inputA, inputB;
    Data< QVector<double> > result;
};

int PointMath_DistanceClass = RegisterObject("Math/Point/Distance").setClass<PointMath_Distance>().setDescription("Compute the distance between 2 points");

//*************************************************************************//

class PointMath_Rotation : public PandaObject
{
public:
    PointMath_Rotation(PandaDocument *doc)
        : PandaObject(doc)
        , input(initData(&input, "point", "Point to rotate"))
        , result(initData(&result, "result", "Result of the rotation"))
        , center(initData(&center, "center", "Center of the rotation"))
        , angle(initData(&angle, "angle", "Angle of the rotation"))
    {
        addInput(&input);
        addInput(&center);
        addInput(&angle);

        addOutput(&result);
    }

    void update()
    {
        const QVector<QPointF>& points = input.getValue();
        int nb = points.size();

        QVector<QPointF>& res = *result.beginEdit();
        res.clear();
        res.resize(nb);

        const QPointF& cen = center.getValue();
        const double& ang = angle.getValue() * M_PI / 180.0;
        double ca = cos(ang), sa = sin(ang);
        for(int i=0; i<nb; ++i)
        {
            QPointF pt = points[i] - cen;
            res[i] = QPointF(pt.x()*ca-pt.y()*sa, pt.x()*sa+pt.y()*ca) + cen;
        }

        result.endEdit();
        this->cleanDirty();
    }

protected:
    Data< QVector<QPointF> > input, result;
    Data< QPointF > center;
    Data< double > angle;
};

int PointMath_RotationClass = RegisterObject("Math/Point/Rotation").setClass<PointMath_Rotation>().setDescription("Rotation of a point around a center");


} // namespace Panda


