#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Path.h>
#include <QVector>

#include <cmath>
#include <algorithm>

namespace panda {

using types::Point;
using types::Path;

class PathMath_Translation : public PandaObject
{
public:
	PANDA_CLASS(PathMath_Translation, PandaObject)

	PathMath_Translation(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData(&m_input, "input", "Path to translate"))
		, m_delta(initData(&m_delta, "translation", "Value of the translation"))
		, m_output(initData(&m_output, "output", "Translated path"))
	{
		addInput(m_input);
		addInput(m_delta);

		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		const auto& delta = m_delta.getValue();
		auto output = m_output.getAccessor();

		int nbA = input.size(), nbB = delta.size();
		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = qMax(nbA, nbB);
			output.resize(nb);

			for(int i=0; i<nb; ++i)
				output[i] = input[i%nbA] + delta[i%nbB];
		}

		cleanDirty();
	}

protected:
	Data< QVector<Path> > m_input, m_output;
	Data< QVector<Point> > m_delta;
};

int PathMath_TranslationClass = RegisterObject<PathMath_Translation>("Math/Path/Translation").setName("Translate path").setDescription("Translate a path");

//****************************************************************************//

class PathMath_Scale : public PandaObject
{
public:
	PANDA_CLASS(PathMath_Scale, PandaObject)

	PathMath_Scale(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData(&m_input, "input", "Path to scale"))
		, m_scale(initData(&m_scale, "scale", "Value of the scale"))
		, m_output(initData(&m_output, "output", "Scaled path"))
	{
		addInput(m_input);
		addInput(m_scale);

		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		const auto& scale = m_scale.getValue();
		auto output = m_output.getAccessor();

		int nbA = input.size(), nbB = scale.size();
		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = qMax(nbA, nbB);
			output.resize(nb);

			for(int i=0; i<nb; ++i)
				output[i] = input[i%nbA] * scale[i%nbB];
		}

		cleanDirty();
	}

protected:
	Data< QVector<Path> > m_input, m_output;
	Data< QVector<PReal> > m_scale;
};

int PathMath_ScaleClass = RegisterObject<PathMath_Scale>("Math/Path/Scale").setName("Scale path").setDescription("Scale a path");

//****************************************************************************//

class PathMath_Rotate : public PandaObject
{
public:
	PANDA_CLASS(PathMath_Rotate, PandaObject)

	PathMath_Rotate(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData(&m_input, "input", "Path to rotate"))
		, m_center(initData(&m_center, "center", "Center of the rotation"))
		, m_angle(initData(&m_angle, "angle", "Angle of the rotation"))
		, m_output(initData(&m_output, "output", "Rotated path"))
	{
		addInput(m_input);
		addInput(m_center);
		addInput(m_angle);

		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		const auto& center = m_center.getValue();
		const auto& angle = m_angle.getValue();
		auto output = m_output.getAccessor();

		int nbP = input.size(), nbC = center.size(), nbA = angle.size();
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

			output.resize(nb);

			PReal PI180 = static_cast<PReal>(M_PI) / static_cast<PReal>(180.0);
			for(int i=0; i<nb; ++i)
				output[i] = types::rotated(input[i%nbP], center[i%nbC], angle[i%nbA] * PI180);
		}

		cleanDirty();
	}

protected:
	Data< QVector<Path> > m_input, m_output;
	Data< QVector<Point> > m_center;
	Data< QVector<PReal> > m_angle;
};

int PathMath_RotateClass = RegisterObject<PathMath_Rotate>("Math/Path/Rotate").setName("Rotate path").setDescription("Rotate a path");

//****************************************************************************//

class PathMath_Length : public PandaObject
{
public:
	PANDA_CLASS(PathMath_Length, PandaObject)

	PathMath_Length(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "path", "Path to analyse"))
		, nbPoints(initData(&nbPoints, "# of points", "Number of points in the path"))
		, length(initData(&length, "length", "Length of the path"))
	{
		addInput(input);

		addOutput(nbPoints);
		addOutput(length);
	}

	void update()
	{
		const QVector<Path>& paths = input.getValue();
		int nb = paths.size();

		auto nbPtsList = nbPoints.getAccessor();
		auto lengthList = length.getAccessor();
		nbPtsList.resize(nb);
		lengthList.resize(nb);
		for(int i=0; i<nb; ++i)
		{
			const Path& path = paths[i];
			int nbPts = path.size();
			nbPtsList[i] = nbPts;
			if(nbPts > 1)
			{
				Point pt1 = path[0];
				PReal l = 0.0;
				for(int j=1; j<nbPts; ++j)
				{
					const Point& pt2 = path[j];
					l += (pt2-pt1).norm();
					pt1 = pt2;
				}
				lengthList[i] = l;
			}
			else
				lengthList[i] = 0;
		}

		cleanDirty();
	}

protected:
	Data< QVector<Path> > input;
	Data< QVector<int> > nbPoints;
	Data< QVector<PReal> > length;
};

int PathMath_LengthClass = RegisterObject<PathMath_Length>("Math/Path/Curve length").setDescription("Compute the length of a series of segments");

//****************************************************************************//

class PathMath_GetPoint : public PandaObject
{
public:
	PANDA_CLASS(PathMath_GetPoint, PandaObject)

	PathMath_GetPoint(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "path", "Input Path"))
		, position(initData(&position, "position", "Position of the point on the curve"))
		, abscissa(initData(&abscissa, "abscissa", "Abscissa of the point to find"))
		, rotation(initData(&rotation, "rotation", "Rotation of the point on the curve"))
	{
		addInput(input);
		addInput(abscissa);

		addOutput(position);
		addOutput(rotation);
	}

	void update()
	{
		const Path& curve = input.getValue();
		const QVector<PReal>& listAbscissa = abscissa.getValue();
		unsigned int nbPts = curve.size();
		unsigned int nbAbscissa = listAbscissa.size();

		auto listPos = position.getAccessor();
		auto listRot = rotation.getAccessor();

		if(nbPts > 1 && nbAbscissa)
		{
			listPos->resize(nbAbscissa);
			listRot.resize(nbAbscissa);

			// Some precomputation
			PReal totalLength = 0.0;
			QVector<PReal> lengths, starts, ends;
			lengths.resize(nbPts - 1);
			starts.resize(nbPts - 1);
			ends.resize(nbPts - 1);
			Point pt1 = curve[0];
			for(unsigned int i=0; i<nbPts-1; ++i)
			{
				starts[i] = totalLength;
				const Point& pt2 = curve[i+1];
				PReal l = (pt2-pt1).norm();
				lengths[i] = l;
				pt1 = pt2;
				totalLength += l;
				ends[i] = totalLength;
			}

			for(unsigned int i=0; i<nbAbscissa; ++i)
			{
				PReal a = qBound<PReal>(0.0, listAbscissa[i], totalLength - 1e-3);
				QVector<PReal>::iterator iter = std::upper_bound(ends.begin(), ends.end(), a);

				unsigned int index = iter - ends.begin();
				PReal p = 0.0;
				if(lengths[index] > 0.1)
					p = (a - starts[index]) / lengths[index];
				const Point& pt1 = curve[index];
				const Point& pt2 = curve[index+1];
				listPos.wref()[i] = pt1 * (1.0 - p) + pt2 * p;
				listRot[i] = atan2(pt2.y-pt1.y, pt2.x-pt1.x) * 180.0 / M_PI;
			}
		}
		else
		{
			listPos->clear();
			listRot.clear();
		}

		cleanDirty();
	}

protected:
	Data< Path > input, position;
	Data< QVector<PReal> > abscissa, rotation;
};

int PathMath_GetPointClass = RegisterObject<PathMath_GetPoint>("Math/Path/Point on curve").setDescription("Get the position and the rotation of a point on a curve based on his abscissa");


} // namespace Panda


