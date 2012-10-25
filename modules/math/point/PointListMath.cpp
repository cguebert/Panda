#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QVector>
#include <math.h>

namespace panda {

class PointListMath_Center : public PandaObject
{
public:
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

		this->cleanDirty();
	}

protected:
	Data< QVector<QPointF> > input;
	Data<int> nbElements;
	Data<QPointF> center;
	Data<double> mean, stdDev;
};

int PointListMath_CenterClass = RegisterObject("Math/List of points/Mean position").setClass<PointListMath_Center>().setDescription("Compute the mean position of a list of points");

} // namespace Panda


