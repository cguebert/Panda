#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

#include <cmath>
#include <algorithm>

namespace panda {

class DoubleListMath_SumMeanStdDev : public PandaObject
{
public:
	PANDA_CLASS(DoubleListMath_SumMeanStdDev, PandaObject)

	DoubleListMath_SumMeanStdDev(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("input", "List of reals to analyse"))
		, nbElements(initData("# of elements", "Size of the list"))
		, sum(initData("sum", "Sum of the list of reals"))
		, mean(initData("mean", "Mean value of the list of reals"))
		, stdDev(initData("std dev", "Standard deviation of the list of reals"))
		, vMin(initData("min", "Minimum value of the list of reals"))
		, vMax(initData("max", "Maximum value of the list of reals"))
	{
		addInput(input);

		addOutput(nbElements);
		addOutput(sum);
		addOutput(mean);
		addOutput(stdDev);
		addOutput(vMin);
		addOutput(vMax);
	}

	void update()
	{
		const std::vector<PReal>& list = input.getValue();
		int nb = list.size();
		nbElements.setValue(nb);

		if(nb)
		{
			PReal tMin = list[0], tMax = list[0];
			PReal E=0, E2=0;
			for(int i=0; i<nb; ++i)
			{
				PReal v = list[i];
				E += v;
				E2 += v*v;
				tMin = std::min(tMin, v);
				tMax = std::max(tMax, v);
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

		cleanDirty();
	}

protected:
	Data< std::vector<PReal> > input;
	Data<int> nbElements;
	Data<PReal> sum, mean, stdDev, vMin, vMax;
};

int DoubleListMath_SumMeanStdDevClass = RegisterObject<DoubleListMath_SumMeanStdDev>("Math/List of reals/Sum, mean and deviation").setName("Sum & mean").setDescription("Compute the sum, mean and standard deviation of a list of reals");

//****************************************************************************//

class DoubleListMath_Median : public PandaObject
{
public:
	PANDA_CLASS(DoubleListMath_Median, PandaObject)

	DoubleListMath_Median(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("input", "List of reals to analyse"))
		, nbElements(initData("# of elements", "Size of the list"))
		, median(initData("median", "Median value of the list of reals"))
	{
		addInput(input);

		addOutput(nbElements);
		addOutput(median);
	}

	void update()
	{
		std::vector<PReal> list = input.getValue();
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

		cleanDirty();
	}

protected:
	Data< std::vector<PReal> > input;
	Data<int> nbElements;
	Data<PReal> median;
};

int DoubleListMath_MedianClass = RegisterObject<DoubleListMath_Median>("Math/List of reals/Median").setDescription("Search the median value of a list of reals");

//****************************************************************************//

class DoubleListMath_Accumulate : public PandaObject
{
public:
	PANDA_CLASS(DoubleListMath_Accumulate, PandaObject)

	DoubleListMath_Accumulate(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "List of reals to sum"))
		, m_output(initData("output", "Accumulated sums"))
	{
		addInput(m_input);

		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto output = m_output.getAccessor();
		int nb = input.size();
		output.resize(nb);

		if(nb)
		{
			output[0] = input[0];
			for(int i=1; i<nb; ++i)
				output[i] = output[i-1] + input[i];
		}

		cleanDirty();
	}

protected:
	Data< std::vector<PReal> > m_input, m_output;
};

int DoubleListMath_AccumulateClass = RegisterObject<DoubleListMath_Accumulate>("Math/List of reals/Accumulate").setDescription("Compute the accumulated sums of a list of values");

} // namespace Panda


