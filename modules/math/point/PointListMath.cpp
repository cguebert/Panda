#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QVector>

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

namespace panda {

class PointListMath_Center : public PandaObject
{
public:
	PANDA_CLASS(PointListMath_Center, PandaObject)

	PointListMath_Center(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "points", "List of points to analyse"))
		, nbElements(initData(&nbElements, "# of elements", "Size of the list"))
		, center(initData(&center, "center", "Center position of the list of points"))
		, mean(initData(&mean, "mean distance", "Mean distance to the center"))
		, stdDev(initData(&stdDev, "std dev", "Standard deviation of the distance to the center"))
	{
		addInput(&input);

		addOutput(&nbElements);
		addOutput(&center);
		addOutput(&mean);
		addOutput(&stdDev);
	}

	void update()
	{
		const QVector<QPointF>& list = input.getValue();
		int nb = list.size();
		nbElements.setValue(nb);

		if(nb)
		{
			double sumX=0, sumY=0;
			for(int i=0; i<nb; ++i)
			{
				QPointF pt = list[i];
				sumX += pt.x();
				sumY += pt.y();
			}

			sumX /= nb;
			sumY /= nb;

			center.setValue(QPointF(sumX, sumY));

			double E=0, E2=0;
			for(int i=0; i<nb; ++i)
			{
				QPointF pt = list[i];
				double dx = pt.x()-sumX, dy = pt.y()-sumY;
				double d2 = dx*dx+dy*dy;
				E += sqrt(d2);
				E2 += d2;
			}

			E /= nb;
			E2 /= nb;
			mean.setValue(E);
			stdDev.setValue(sqrt(E2 - E*E));
		}
		else
		{
			center.setValue(QPointF(0,0));
			mean.setValue(0);
			stdDev.setValue(0);
		}

		cleanDirty();
	}

protected:
	Data< QVector<QPointF> > input;
	Data<int> nbElements;
	Data<QPointF> center;
	Data<double> mean, stdDev;
};

int PointListMath_CenterClass = RegisterObject<PointListMath_Center>("Math/List of points/Mean position").setDescription("Compute the mean position of a list of points");

//*************************************************************************//

class PointListMath_Length : public PandaObject
{
public:
	PANDA_CLASS(PointListMath_Length, PandaObject)

	PointListMath_Length(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "points", "List of points to analyse"))
		, nbElements(initData(&nbElements, "# of elements", "Size of the list"))
		, length(initData(&length, "length", "Length of the series of segments"))
	{
		addInput(&input);

		addOutput(&nbElements);
		addOutput(&length);
	}

	void update()
	{
		const QVector<QPointF>& list = input.getValue();
		int nb = list.size();
		nbElements.setValue(nb);

		if(nb > 1)
		{
			double l = 0.0;
			QPointF pt1 = list[0];
			for(int i=1; i<nb; ++i)
			{
				const QPointF& pt2 = list[i];
				double dx = pt2.x()-pt1.x(), dy = pt2.y()-pt1.y();
				double d2 = dx*dx+dy*dy;
				l += sqrt(d2);
				pt1 = pt2;
			}
			length.setValue(l);
		}
		else
			length.setValue(0.0);

		cleanDirty();
	}

protected:
	Data< QVector<QPointF> > input;
	Data<int> nbElements;
	Data<double> length;
};

int PointListMath_LengthClass = RegisterObject<PointListMath_Length>("Math/List of points/Curve length").setDescription("Compute the length of a series of segments");

//*************************************************************************//

class PointListMath_GetPoint : public PandaObject
{
public:
	PANDA_CLASS(PointListMath_GetPoint, PandaObject)

	PointListMath_GetPoint(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "points", "List of points forming the curve"))
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
		const QVector<QPointF>& curve = input.getValue();
		const QVector<double>& listAbscissa = abscissa.getValue();
		unsigned int nbPts = curve.size();
		unsigned int nbAbscissa = listAbscissa.size();

		auto listPos = position.getAccessor();
		auto listRot = rotation.getAccessor();

		if(nbPts > 1 && nbAbscissa)
		{
			listPos.resize(nbAbscissa);
			listRot.resize(nbAbscissa);

			// Some precomputation
			double totalLength = 0.0;
			QVector<double> lengths, starts, ends;
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
				double a = qBound(0.0, listAbscissa[i], totalLength);
				QVector<double>::iterator iter = std::upper_bound(ends.begin(), ends.end(), a);

				unsigned int index = iter - ends.begin();
				double p = 0.0;
				if(lengths[index] > 0.1)
					p = (a - starts[index]) / lengths[index];
				const QPointF& pt1 = curve[index];
				const QPointF& pt2 = curve[index+1];
				listPos[i] = pt1 * (1.0 - p) + pt2 * p;
				listRot[i] = atan2(pt2.y()-pt1.y(), pt2.x()-pt1.x()) * 180.0 / M_PI;
			}
		}
		else
		{
			listPos.clear();
			listRot.clear();
		}

		cleanDirty();
	}

protected:
	Data< QVector<QPointF> > input, position;
	Data< QVector<double> > abscissa, rotation;
};

int PointListMath_GetPointClass = RegisterObject<PointListMath_GetPoint>("Math/List of points/Point on curve").setDescription("Get the position and the rotation of a point on a curve based on his abscissa");


} // namespace Panda


