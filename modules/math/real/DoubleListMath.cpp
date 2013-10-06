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
	PANDA_CLASS(DoubleListMath_SumMeanStdDev, PandaObject)

	DoubleListMath_SumMeanStdDev(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "List of reals to analyse"))
		, nbElements(initData(&nbElements, "# of elements", "Size of the list"))
		, sum(initData(&sum, "sum", "Sum of the list of reals"))
		, mean(initData(&mean, "mean", "Mean value of the list of reals"))
		, stdDev(initData(&stdDev, "std dev", "Standard deviation of the list of reals"))
		, vMin(initData(&vMin, "min", "Minimum value of the list of reals"))
		, vMax(initData(&vMax, "max", "Maximum value of the list of reals"))
	{
		addInput(&input);

		addOutput(&nbElements);
		addOutput(&sum);
		addOutput(&mean);
		addOutput(&stdDev);
		addOutput(&vMin);
		addOutput(&vMax);
	}

	void update()
	{
		const QVector<double>& list = input.getValue();
		int nb = list.size();
		nbElements.setValue(nb);

		if(nb)
		{
			double tMin = list[0], tMax = list[0];
			double E=0, E2=0;
			for(int i=0; i<nb; ++i)
			{
				double v = list[i];
				E += v;
				E2 += v*v;
				tMin = qMin(tMin, v);
				tMax = qMax(tMax, v);
			}

			sum.setValue(E);
			E /= nb;
			E2 /= nb;
			mean.setValue(E);
			stdDev.setValue(sqrt(E2 - E*E));
			vMin.setValue(tMin);
			vMax.setValue(tMax);
		}
		else
		{
			sum.setValue(0.0);
			mean.setValue(0.0);
			stdDev.setValue(0.0);
			vMin.setValue(0.0);
			vMax.setValue(0.0);
		}

		this->cleanDirty();
	}

protected:
	Data< QVector<double> > input;
	Data<int> nbElements;
	Data<double> sum, mean, stdDev, vMin, vMax;
};

int DoubleListMath_SumMeanStdDevClass = RegisterObject<DoubleListMath_SumMeanStdDev>("Math/List of reals/Sum, mean and deviation").setName("Sum & mean").setDescription("Compute the sum, mean and standard deviation of a list of reals");

//*************************************************************************//

class DoubleListMath_Median : public PandaObject
{
public:
	PANDA_CLASS(DoubleListMath_Median, PandaObject)

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

int DoubleListMath_MedianClass = RegisterObject<DoubleListMath_Median>("Math/List of reals/Median").setDescription("Search the median value of a list of reals");

//*************************************************************************//

class DoubleListMath_Sort : public PandaObject
{
public:
	PANDA_CLASS(DoubleListMath_Sort, PandaObject)

	DoubleListMath_Sort(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "List of reals to sort"))
		, output(initData(&output, "output", "Sorted list of reals"))
		, sortedIndices(initData(&sortedIndices, "indices", "Indices corresponding to the sorted list"))
	{
		addInput(&input);

		addOutput(&output);
		addOutput(&sortedIndices);
	}

	void update()
	{
		const QVector<double>& list = input.getValue();
		QMap<double, int> tmpMap;
		int nb = list.size();

		for(int i=0; i<nb; ++i)
			tmpMap.insertMulti(list[i], i);

		output.setValue(tmpMap.keys().toVector());
		sortedIndices.setValue(tmpMap.values().toVector());

		this->cleanDirty();
	}

protected:
	Data< QVector<double> > input, output;
	Data< QVector<int> > sortedIndices;
};

int DoubleListMath_SortClass = RegisterObject<DoubleListMath_Sort>("Math/List of reals/Sort").setDescription("Sort a list of reals");


} // namespace Panda


