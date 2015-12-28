#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Point.h>

#include <cmath>
#include <algorithm>

namespace panda {

using types::Point;

class PointListMath_Center : public PandaObject
{
public:
	PANDA_CLASS(PointListMath_Center, PandaObject)

	PointListMath_Center(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("points", "List of points to analyse"))
		, nbElements(initData("# of elements", "Size of the list"))
		, center(initData("center", "Center position of the list of points"))
		, mean(initData("mean distance", "Mean distance to the center"))
		, stdDev(initData("std dev", "Standard deviation of the distance to the center"))
	{
		addInput(input);

		addOutput(nbElements);
		addOutput(center);
		addOutput(mean);
		addOutput(stdDev);
	}

	void update()
	{
		const std::vector<Point>& list = input.getValue();
		int nb = list.size();
		nbElements.setValue(nb);

		if(nb)
		{
			Point sum;
			for(const auto& pt : list)
				sum += pt;

			sum /= nb;

			center.setValue(sum);

			PReal E=0, E2=0;
			for(int i=0; i<nb; ++i)
			{
				Point pt = list[i];
				PReal d2 = (pt - sum).norm2();
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
			center.setValue(Point(0,0));
			mean.setValue(0);
			stdDev.setValue(0);
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > input;
	Data<int> nbElements;
	Data<Point> center;
	Data<PReal> mean, stdDev;
};

int PointListMath_CenterClass = RegisterObject<PointListMath_Center>("Math/List of points/Mean position").setDescription("Compute the mean position of a list of points");

} // namespace Panda


