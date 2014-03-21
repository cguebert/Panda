#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Path.h>
#include <panda/helper/Point.h>
#include <QVector>

#include <math.h>
#include <algorithm>

namespace panda {

using types::Path;

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
		addInput(&input);

		addOutput(&nbPoints);
		addOutput(&length);
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
				QPointF pt1 = path[0];
				double l = 0.0;
				for(int j=1; j<nbPts; ++j)
				{
					const QPointF& pt2 = path[j];
					l += helper::norm(pt2-pt1);
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

//*************************************************************************//

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
		addInput(&input);
		addInput(&abscissa);

		addOutput(&position);
		addOutput(&rotation);
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
			double totalLength = 0.0;
			QVector<PReal> lengths, starts, ends;
			lengths.resize(nbPts - 1);
			starts.resize(nbPts - 1);
			ends.resize(nbPts - 1);
			QPointF pt1 = curve[0];
			for(unsigned int i=0; i<nbPts-1; ++i)
			{
				starts[i] = totalLength;
				const QPointF& pt2 = curve[i+1];
				double dx = pt2.x()-pt1.x(), dy = pt2.y()-pt1.y();
				double d2 = dx*dx+dy*dy;
				double l = sqrt(d2);
				lengths[i] = l;
				pt1 = pt2;
				totalLength += l;
				ends[i] = totalLength;
			}

			for(unsigned int i=0; i<nbAbscissa; ++i)
			{
				double a = qBound<PReal>(0.0, listAbscissa[i], totalLength - 1e-5);
				QVector<PReal>::iterator iter = std::upper_bound(ends.begin(), ends.end(), a);

				unsigned int index = iter - ends.begin();
				double p = 0.0;
				if(lengths[index] > 0.1)
					p = (a - starts[index]) / lengths[index];
				const QPointF& pt1 = curve[index];
				const QPointF& pt2 = curve[index+1];
				listPos.wref()[i] = pt1 * (1.0 - p) + pt2 * p;
				listRot[i] = atan2(pt2.y()-pt1.y(), pt2.x()-pt1.x()) * 180.0 / M_PI;
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


