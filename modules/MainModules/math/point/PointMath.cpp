#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Point.h>

#include <math.h>

namespace panda {

using types::Point;

class PointMath_Addition : public PandaObject
{
public:
	PANDA_CLASS(PointMath_Addition, PandaObject)

	PointMath_Addition(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData("point 1", "First point"))
		, inputB(initData("point 2", "Second point"))
		, result(initData("result", "Result of the addition of the 2 points"))
	{
		addInput(inputA);
		addInput(inputB);

		addOutput(result);
	}

	void update()
	{
		auto res = result.getAccessor();
		res.clear();

		const std::vector<Point>& valA = inputA.getValue();
		const std::vector<Point>& valB = inputB.getValue();
		int nbA = valA.size(), nbB = valB.size();

		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = std::max(nbA, nbB);
			res.resize(nb);

			for(int i=0; i<nb; ++i)
				res[i] = valA[i%nbA] + valB[i%nbB];
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > inputA, inputB, result;
};

int PointMath_AdditionClass = RegisterObject<PointMath_Addition>("Math/Point/Addition").setName("Add points").setDescription("Compute the addition of 2 points");

//****************************************************************************//

class PointMath_Substraction : public PandaObject
{
public:
	PANDA_CLASS(PointMath_Substraction, PandaObject)

	PointMath_Substraction(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData("point 1", "First point"))
		, inputB(initData("point 2", "Second point"))
		, result(initData("result", "Result of the substraction"))
	{
		addInput(inputA);
		addInput(inputB);

		addOutput(result);
	}

	void update()
	{
		auto res = result.getAccessor();
		res.clear();

		const std::vector<Point>& valA = inputA.getValue();
		const std::vector<Point>& valB = inputB.getValue();
		int nbA = valA.size(), nbB = valB.size();

		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = std::max(nbA, nbB);
			res.resize(nb);

			for(int i=0; i<nb; ++i)
				res[i] = valA[i%nbA] - valB[i%nbB];
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > inputA, inputB, result;
};

int PointMath_SubstractionClass = RegisterObject<PointMath_Substraction>("Math/Point/Substraction").setName("Substract points").setDescription("Compute the substraction of 2 points");

//****************************************************************************//

class PointMath_Scale : public PandaObject
{
public:
	PANDA_CLASS(PointMath_Scale, PandaObject)

	PointMath_Scale(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("point", "Point value to multiply"))
		, result(initData("result", "Result of the multiplication of the point by the real"))
		, factor(initData("factor", "Real by which to multiply the point"))
	{
		addInput(input);
		addInput(factor);

		factor.getAccessor().push_back(1.0);

		addOutput(result);
	}

	void update()
	{
		auto res = result.getAccessor();
		res.clear();

		const std::vector<Point>& points = input.getValue();
		const std::vector<PReal>& reals = factor.getValue();
		int nbP = points.size(), nbR = reals.size();

		if(nbP && nbR)
		{
			if(nbR < nbP && nbR > 1)		nbP = nbR;	// Either 1 real, or equal nb of reals & points
			else if(nbP < nbR && nbP > 1)	nbR = nbP;
			int nb = std::max(nbP, nbR);
			res.resize(nb);

			for(int i=0; i<nb; ++i)
				res[i] = points[i%nbP] * reals[i%nbR];
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > input, result;
	Data< std::vector<PReal> > factor;
};

int PointMath_ScaleClass = RegisterObject<PointMath_Scale>("Math/Point/Multiply").setName("Scale point").setDescription("Multiply a point by a real");

//****************************************************************************//

class PointMath_Division : public PandaObject
{
public:
	PANDA_CLASS(PointMath_Division, PandaObject)

	PointMath_Division(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("point", "Point value to multiply"))
		, result(initData("result", "Result of the multiplication of the point by the real"))
		, factor(initData("factor", "Real by which to divide the point"))
	{
		addInput(input);
		addInput(factor);

		factor.getAccessor().push_back(1.0);

		addOutput(result);
	}

	void update()
	{
		auto res = result.getAccessor();
		res.clear();

		const std::vector<Point>& points = input.getValue();
		const std::vector<PReal>& reals = factor.getValue();
		int nbP = points.size(), nbR = reals.size();

		if(nbP && nbR)
		{
			if(nbR < nbP && nbR > 1)		nbP = nbR;	// Either 1 real, or equal nb of reals & points
			else if(nbP < nbR && nbP > 1)	nbR = nbP;
			int nb = std::max(nbP, nbR);
			res.resize(nb);

			for(int i=0; i<nb; ++i)
				res[i] = points[i%nbP] / reals[i%nbR];
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > input, result;
	Data< std::vector<PReal> > factor;
};

int PointMath_DivisionClass = RegisterObject<PointMath_Division>("Math/Point/Division").setName("Divide point").setDescription("Divide a point by a real");

//****************************************************************************//

class PointMath_Distance : public PandaObject
{
public:
	PANDA_CLASS(PointMath_Distance, PandaObject)

	PointMath_Distance(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData("point 1", "First point"))
		, inputB(initData("point 2", "Second point"))
		, result(initData("distance", "Distance between the 2 points"))
	{
		addInput(inputA);
		addInput(inputB);

		addOutput(result);
	}

	void update()
	{
		auto res = result.getAccessor();
		res.clear();

		const std::vector<Point>& valA = inputA.getValue();
		const std::vector<Point>& valB = inputB.getValue();
		int nbA = valA.size(), nbB = valB.size();

		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = std::max(nbA, nbB);
			res.resize(nb);

			for(int i=0; i<nb; ++i)
				res[i] = (valA[i%nbA] - valB[i%nbB]).norm();
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > inputA, inputB;
	Data< std::vector<PReal> > result;
};

int PointMath_DistanceClass = RegisterObject<PointMath_Distance>("Math/Point/Distance").setDescription("Compute the distance between 2 points");

//****************************************************************************//

class PointMath_Rotation : public PandaObject
{
public:
	PANDA_CLASS(PointMath_Rotation, PandaObject)

	PointMath_Rotation(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("point", "Point to rotate"))
		, result(initData("result", "Result of the rotation"))
		, center(initData("center", "Center of the rotation"))
		, angle(initData("angle", "Angle of the rotation"))
	{
		addInput(input);
		addInput(center);
		addInput(angle);

		addOutput(result);
	}

	void update()
	{
		auto res = result.getAccessor();
		res.clear();

		const std::vector<Point>& points = input.getValue();
		const std::vector<Point>& centers = center.getValue();
		const std::vector<PReal>& angles = angle.getValue();
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
				nb = std::max(nbA, nbC);
			}

			res.resize(nb);

			PReal PI180 = static_cast<PReal>(M_PI) / static_cast<PReal>(180.0);
			for(int i=0; i<nb; ++i)
				res[i] = rotated(points[i%nbP], centers[i%nbC], angles[i%nbA] * PI180);
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > input, result;
	Data< std::vector<Point> > center;
	Data< std::vector<PReal> > angle;
};

int PointMath_RotationClass = RegisterObject<PointMath_Rotation>("Math/Point/Rotation").setDescription("Rotation of a point around a center");

//****************************************************************************//

class PointMath_AngleOfVector : public PandaObject
{
public:
	PANDA_CLASS(PointMath_AngleOfVector, PandaObject)

	PointMath_AngleOfVector(PandaDocument *doc)
		: PandaObject(doc)
		, vector(initData("vector", "Vector to analyse"))
		, angle(initData("angle", "Angle of the vector"))
	{
		addInput(vector);

		addOutput(angle);
	}

	void update()
	{
		auto angleList = angle.getAccessor();
		angleList.clear();

		const std::vector<Point>& vecList = vector.getValue();
		int nb = vecList.size();

		if(nb)
		{
			angleList.resize(nb);

			for(int i=0; i<nb; ++i)
			{
				const Point& pt = vecList[i];
				if(pt.norm2() < 1e-10)
					angleList[i] = 0;
				else
					angleList[i] = -atan2(pt.y, pt.x) * 180 / M_PI;
			}
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > vector;
	Data< std::vector<PReal> > angle;
};

int PointMath_AngleOfVectorClass = RegisterObject<PointMath_AngleOfVector>("Math/Point/Angle of vector").setDescription("Computes the angle of a vector");

//****************************************************************************//

class PointMath_AngleToVector : public PandaObject
{
public:
	PANDA_CLASS(PointMath_AngleToVector, PandaObject)

	PointMath_AngleToVector(PandaDocument *doc)
		: PandaObject(doc)
		, angle(initData("angle", "Angle of the vector"))
		, vector(initData("vector", "Vector corresponding to the angle"))
	{
		addInput(angle);

		addOutput(vector);
	}

	void update()
	{
		auto vecList = vector.getAccessor();
		vecList.clear();

		const std::vector<PReal>& angleList = angle.getValue();
		int nb = angleList.size();

		if(nb)
		{
			vecList.resize(nb);

			for(int i=0; i<nb; ++i)
			{
				PReal a = -angleList[i] * M_PI / 180;
				vecList[i] = Point(cos(a), sin(a));
			}
		}

		cleanDirty();
	}

protected:
	Data< std::vector<PReal> > angle;
	Data< std::vector<Point> > vector;
};

int PointMath_AngleToVectorClass = RegisterObject<PointMath_AngleToVector>("Math/Point/Angle to vector").setDescription("Creates a vector corresponding to a rotation");

//****************************************************************************//

class PointMath_Norm : public PandaObject
{
public:
	PANDA_CLASS(PointMath_Norm, PandaObject)

	PointMath_Norm(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("input", "Input point"))
		, output(initData("output", "Norm of the input point"))
	{
		addInput(input);

		addOutput(output);
	}

	void update()
	{
		auto res = output.getAccessor();
		res.clear();

		const std::vector<Point>& val = input.getValue();
		int nb = val.size();

		if(nb)
		{
			res.resize(nb);
			for(int i=0; i<nb; ++i)
				res[i] = val[i].norm();
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > input;
	Data< std::vector<PReal> > output;
};

int PointMath_NormClass = RegisterObject<PointMath_Norm>("Math/Point/Norm").setDescription("Compute the norm of a point");

//****************************************************************************//

class PointMath_Norm2 : public PandaObject
{
public:
	PANDA_CLASS(PointMath_Norm2, PandaObject)

	PointMath_Norm2(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("input", "Input point"))
		, output(initData("output", "Square of the norm of the input point"))
	{
		addInput(input);

		addOutput(output);
	}

	void update()
	{
		auto res = output.getAccessor();
		res.clear();

		const std::vector<Point>& val = input.getValue();
		int nb = val.size();

		if(nb)
		{
			res.resize(nb);
			for(int i=0; i<nb; ++i)
				res[i] = val[i].norm2();
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > input;
	Data< std::vector<PReal> > output;
};

int PointMath_Norm2Class = RegisterObject<PointMath_Norm2>("Math/Point/Norm2").setDescription("Compute the square of the norm of a point");

//****************************************************************************//

class PointMath_Normalize : public PandaObject
{
public:
	PANDA_CLASS(PointMath_Normalize, PandaObject)

	PointMath_Normalize(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("input", "Input point"))
		, output(initData("output", "Square of the norm of the input point"))
	{
		addInput(input);

		addOutput(output);
	}

	void update()
	{
		auto res = output.getAccessor();
		res.clear();

		const std::vector<Point>& val = input.getValue();
		int nb = val.size();

		if(nb)
		{
			res.resize(nb);
			for(int i=0; i<nb; ++i)
				res[i] = val[i].normalized();
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > input, output;
};

int PointMath_NormalizeClass = RegisterObject<PointMath_Normalize>("Math/Point/Normalize").setDescription("Normalize a point");

//****************************************************************************//

class PointMath_LinearProduct : public PandaObject
{
public:
	PANDA_CLASS(PointMath_LinearProduct, PandaObject)

	PointMath_LinearProduct(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData("point 1", "First point"))
		, inputB(initData("point 2", "Second point"))
		, result(initData("result", "Result of the linear product"))
	{
		addInput(inputA);
		addInput(inputB);

		addOutput(result);
	}

	void update()
	{
		auto res = result.getAccessor();
		res.clear();

		const std::vector<Point>& valA = inputA.getValue();
		const std::vector<Point>& valB = inputB.getValue();
		int nbA = valA.size(), nbB = valB.size();

		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = std::max(nbA, nbB);
			res.resize(nb);

			for(int i=0; i<nb; ++i)
				res[i] = valA[i%nbA].linearProduct(valB[i%nbB]);
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > inputA, inputB, result;
};

int PointMath_LinearProductClass = RegisterObject<PointMath_LinearProduct>("Math/Point/Linear product").setDescription("Compute the linear product of 2 points");

//****************************************************************************//

class PointMath_LinearDivision : public PandaObject
{
public:
	PANDA_CLASS(PointMath_LinearDivision, PandaObject)

	PointMath_LinearDivision(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData("point 1", "First point"))
		, inputB(initData("point 2", "Second point"))
		, result(initData("result", "Result of the linear division"))
	{
		addInput(inputA);
		addInput(inputB);

		addOutput(result);
	}

	void update()
	{
		auto res = result.getAccessor();
		res.clear();

		const std::vector<Point>& valA = inputA.getValue();
		const std::vector<Point>& valB = inputB.getValue();
		int nbA = valA.size(), nbB = valB.size();

		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = std::max(nbA, nbB);
			res.resize(nb);

			for(int i=0; i<nb; ++i)
				res[i] = valA[i%nbA].linearDivision(valB[i%nbB]);
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > inputA, inputB, result;
};

int PointMath_LinearDivisionClass = RegisterObject<PointMath_LinearDivision>("Math/Point/Linear division").setDescription("Compute the linear division of 2 points");

} // namespace Panda


