#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QVector>
#include <math.h>
#include <algorithm>

namespace panda {

class DoubleListMath_SumMeanStdDev : public PandaObject
{
public:
	DoubleListMath_SumMeanStdDev(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "List of reals to analyse"))
		, nbElements(initData(&nbElements, "# of elements", "Size of the list"))
		, sum(initData(&sum, "sum", "Sum of the list of reals"))
		, mean(initData(&mean, "mean", "Mean value of the list of reals"))
		, stdDev(initData(&stdDev, "std dev", "Standard deviation of the list of reals"))
	{
		addInput(&input);

		addOutput(&nbElements);
		addOutput(&sum);
		addOutput(&mean);
		addOutput(&stdDev);
	}

	void update()
	{
		const QVector<double>& list = input.getValue();
		int nb = list.size();
		nbElements.setValue(nb);

		if(nb)
		{
			double E=0, E2=0;
			for(int i=0; i<nb; ++i)
			{
				double v = list[i];
				E += v;
				E2 += v*v;
			}

			sum.setValue(E);
			E /= nb;
			E2 /= nb;
			mean.setValue(E);
			stdDev.setValue(sqrt(E2 - E*E));
		}
		else
		{
			sum.setValue(0);
			mean.setValue(0);
			stdDev.setValue(0);
		}

		this->cleanDirty();
	}

protected:
	Data< QVector<double> > input;
	Data<int> nbElements;
	Data<double> sum, mean, stdDev;
};

int DoubleListMath_SumMeanStdDevClass = RegisterObject("Math/List of reals/Sum, mean and deviation").setClass<DoubleListMath_SumMeanStdDev>().setName("Sum & mean").setDescription("Compute the sum, mean and standard deviation of a list of reals");

//*************************************************************************//


class DoubleListMath_Median : public PandaObject
{
public:
	DoubleListMath_Median(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "List of reals to analyse"))
		, nbElements(initData(&nbElements, "# of elements", "Size of the list"))
		, median(initData(&median, "median", "Median value of the list of reals"))
	{
		addInput(&input);

		addOutput(&nbElements);
		addOutput(&median);
	}

	void update()
	{
		QVector<double> list = input.getValue();
		int nb = list.size();
		nbElements.setValue(nb);

		if(nb)
		{
			int n = nb / 2;
			std::nth_element(list.begin(), list.begin()+n, list.end());

			median.setValue(list[n]);
		}
		else
			median.setValue(0);

		this->cleanDirty();
	}

protected:
	Data< QVector<double> > input;
	Data<int> nbElements;
	Data<double> median;
};

int DoubleListMath_MedianClass = RegisterObject("Math/List of reals/Median").setClass<DoubleListMath_Median>().setName("Median").setDescription("Search the median value of a list of reals");


} // namespace Panda


