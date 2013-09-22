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
	PANDA_CLASS(PointMath_Addition, PandaObject)

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
		auto res = result.getAccessor();
		res.clear();

        const QVector<QPointF>& valA = inputA.getValue();
        const QVector<QPointF>& valB = inputB.getValue();
		int nbA = valA.size(), nbB = valB.size();

		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = qMax(nbA, nbB);
			res.resize(nb);

			for(int i=0; i<nb; ++i)
				res[i] = valA[i%nbA] + valB[i%nbB];
		}

        this->cleanDirty();
    }

protected:
    Data< QVector<QPointF> > inputA, inputB, result;
};

int PointMath_AdditionClass = RegisterObject("Math/Point/Addition").setClass<PointMath_Addition>().setName("Add points").setDescription("Compute the addition of 2 points");

//*************************************************************************//

class PointMath_Substraction : public PandaObject
{
public:
	PANDA_CLASS(PointMath_Substraction, PandaObject)

	PointMath_Substraction(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData(&inputA, "point 1", "First point"))
		, inputB(initData(&inputB, "point 2", "Second point"))
		, result(initData(&result, "result", "Result of the substraction"))
	{
		addInput(&inputA);
		addInput(&inputB);

		addOutput(&result);
	}

	void update()
	{
		auto res = result.getAccessor();
		res.clear();

		const QVector<QPointF>& valA = inputA.getValue();
		const QVector<QPointF>& valB = inputB.getValue();
		int nbA = valA.size(), nbB = valB.size();

		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = qMax(nbA, nbB);
			res.resize(nb);

			for(int i=0; i<nb; ++i)
				res[i] = valA[i%nbA] - valB[i%nbB];
		}

		this->cleanDirty();
	}

protected:
	Data< QVector<QPointF> > inputA, inputB, result;
};

int PointMath_SubstractionClass = RegisterObject("Math/Point/Substraction").setClass<PointMath_Substraction>().setName("Sub points").setDescription("Compute the substraction of 2 points");

//*************************************************************************//

class PointMath_Scale : public PandaObject
{
public:
	PANDA_CLASS(PointMath_Scale, PandaObject)

    PointMath_Scale(PandaDocument *doc)
        : PandaObject(doc)
        , input(initData(&input, "point", "Point value to multiply"))
        , result(initData(&result, "result", "Result of the multiplication of the point by the real"))
		, factor(initData(&factor, "factor", "Real by which to multiply the point"))
    {
        addInput(&input);
        addInput(&factor);

		factor.getAccessor().push_back(1.0);

        addOutput(&result);
    }

    void update()
    {
		auto res = result.getAccessor();
		res.clear();

        const QVector<QPointF>& points = input.getValue();
		const QVector<double>& reals = factor.getValue();
		int nbP = points.size(), nbR = reals.size();

		if(nbP && nbR)
		{
			if(nbR < nbP && nbR > 1)		nbP = nbR;	// Either 1 real, or equal nb of reals & points
			else if(nbP < nbR && nbP > 1)	nbR = nbP;
			int nb = qMax(nbP, nbR);
			res.resize(nb);

			for(int i=0; i<nb; ++i)
				res[i] = points[i%nbP] * reals[i%nbR];
		}

        this->cleanDirty();
    }

protected:
    Data< QVector<QPointF> > input, result;
	Data< QVector<double> > factor;
};

int PointMath_ScaleClass = RegisterObject("Math/Point/Multiply").setClass<PointMath_Scale>().setName("Scale point").setDescription("Multiply a point by a real");

//*************************************************************************//

class PointMath_Division : public PandaObject
{
public:
	PANDA_CLASS(PointMath_Division, PandaObject)

	PointMath_Division(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "point", "Point value to multiply"))
		, result(initData(&result, "result", "Result of the multiplication of the point by the real"))
		, factor(initData(&factor, "factor", "Real by which to divide the point"))
	{
		addInput(&input);
		addInput(&factor);

		factor.getAccessor().push_back(1.0);

		addOutput(&result);
	}

	void update()
	{
		auto res = result.getAccessor();
		res.clear();

		const QVector<QPointF>& points = input.getValue();
		const QVector<double>& reals = factor.getValue();
		int nbP = points.size(), nbR = reals.size();

		if(nbP && nbR)
		{
			if(nbR < nbP && nbR > 1)		nbP = nbR;	// Either 1 real, or equal nb of reals & points
			else if(nbP < nbR && nbP > 1)	nbR = nbP;
			int nb = qMax(nbP, nbR);
			res.resize(nb);

			for(int i=0; i<nb; ++i)
				res[i] = points[i%nbP] / reals[i%nbR];
		}

		this->cleanDirty();
	}

protected:
	Data< QVector<QPointF> > input, result;
	Data< QVector<double> > factor;
};

int PointMath_DivisionClass = RegisterObject("Math/Point/Division").setClass<PointMath_Division>().setName("Divide point").setDescription("Divide a point by a real");

//*************************************************************************//

class PointMath_Distance : public PandaObject
{
public:
	PANDA_CLASS(PointMath_Distance, PandaObject)

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
		auto res = result.getAccessor();
		res.clear();

        const QVector<QPointF>& valA = inputA.getValue();
        const QVector<QPointF>& valB = inputB.getValue();
		int nbA = valA.size(), nbB = valB.size();

		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = qMax(nbA, nbB);
			res.resize(nb);

			for(int i=0; i<nb; ++i)
			{
				const QPointF& ptA = valA[i%nbA], ptB = valB[i%nbB];
				double dx = ptA.x()-ptB.x(), dy = ptA.y()-ptB.y();
				res[i] = sqrt(dx*dx+dy*dy);
			}
		}

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
	PANDA_CLASS(PointMath_Rotation, PandaObject)

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
		auto res = result.getAccessor();
		res.clear();

        const QVector<QPointF>& points = input.getValue();
		const QVector<QPointF>& centers = center.getValue();
		const QVector<double>& angles = angle.getValue();
		int nbP = points.size(), nbC = centers.size(), nbA = angles.size();

		if(nbP && nbC && nbA)
		{
			int nb = nbP;
			if(nbP > 1)
			{
				if(nbC != nbP) nbC = 1;
				if(nbA != nbA) nbA = 1;
			}
			else
			{
				if(nbC > nbA && nbA > 1)		nbC = nbA;
				else if(nbA > nbC && nbC > 1)	nbA = nbC;
				nb = qMax(nbA, nbC);
			}

			res.resize(nb);

			for(int i=0; i<nb; ++i)
			{
				const QPointF& cen = centers[i%nbC];
				const double& ang = angles[i%nbA] * M_PI / 180.0;
				double ca = cos(ang), sa = sin(ang);
				QPointF pt = points[i%nbP] - cen;
				res[i] = QPointF(pt.x()*ca-pt.y()*sa, pt.x()*sa+pt.y()*ca) + cen;
			}
		}

        this->cleanDirty();
    }

protected:
    Data< QVector<QPointF> > input, result;
	Data< QVector<QPointF> > center;
	Data< QVector<double> > angle;
};

int PointMath_RotationClass = RegisterObject("Math/Point/Rotation").setClass<PointMath_Rotation>().setDescription("Rotation of a point around a center");


} // namespace Panda


