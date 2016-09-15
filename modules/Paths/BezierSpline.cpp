#include <panda/document/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Path.h>

namespace panda {

using types::Point;
using types::Path;

class Curve_BezierSpline : public PandaObject
{
public:
	PANDA_CLASS(Curve_BezierSpline, PandaObject)

	Curve_BezierSpline(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("input", "List of control points"))
		, output(initData("output", "List of points on the spline"))
		, steps(initData(4, "steps", "Number of points on each segment"))
	{
		addInput(input);
		addInput(steps);

		addOutput(output);
	}

	void update()
	{
		const auto& ctrlPts = input.getValue().points;
		auto outAcc = output.getAccessor();
		auto& outPts = outAcc.wref().points;
		int nbCtrlPts = ctrlPts.size();
		if(nbCtrlPts > 2)
		{
			int nbSteps = steps.getValue();
			int nbPts = (nbCtrlPts - 1) * nbSteps + 1;
			computeCoefficients(nbCtrlPts, nbPts);
			outPts.resize(nbPts);

			for(int i=0; i<nbPts; ++i)
			{
				Point pt;
				for(int j=0; j<nbCtrlPts; ++j)
					pt += coefs[i][j] * ctrlPts[j];
				outPts[i] = pt;
			}
		}
		else
			outPts.clear();
	}

	void computeCoefficients(int c, int p)
	{
		prepareBinomials(c);

		if(coefs.size() != p || coefs[0].size() != c)
		{
			coefs.resize(p);
			float s = 1 / static_cast<float>(p - 1);
			for(int i=0; i<p; ++i)
			{
				coefs[i].resize(c);
				float t = i * s;
				for(int j=0; j<c; ++j)
					coefs[i][j] = bernstein(j, c-1, t);
			}
		}
	}

	void prepareBinomials(int n)
	{
		if(binomials.size() == n)
			return;

		binomials.resize(n);
		binomials[0] = 1;
		for(int i=1; i<n; i++)
		{
			binomials[i] = 1;
			for(int j = i-1; j>0; j--)
				binomials[j] += binomials[j-1];
		}
	}

	float bernstein(int i, int n, float t)
	{
		return binomials[i] * pow(t, i) * pow(1-t, n-i);
	}

protected:
	Data<Path> input, output;
	Data<int> steps;

	std::vector<float> binomials;
	std::vector< std::vector<float> > coefs;
};

int Curve_BezierSplineClass = RegisterObject<Curve_BezierSpline>("Math/Path/Bezier spline").setDescription("Compute a bezier spline using the controls points");

} // namespace Panda


