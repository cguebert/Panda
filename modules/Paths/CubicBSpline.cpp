#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Path.h>
#include <array>

namespace panda {

using types::Path;
using types::Point;

class Curve_CubicBSpline : public PandaObject
{
public:
	PANDA_CLASS(Curve_CubicBSpline, PandaObject)

	Curve_CubicBSpline(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "List of control points"))
		, output(initData(&output, "output", "List of points on the spline"))
		, steps(initData(&steps, 6, "steps", "Number of points on each segment"))
		, close(initData(&close, 0, "close", "Is the curve closed or not?"))
	{
		addInput(input);
		addInput(steps);
		addInput(close);
		close.setWidget("checkbox");

		addOutput(output);
	}

	void update()
	{
		const auto& ctrlPts = input.getValue();
		auto outPts = output.getAccessor();
		int nbCtrlPts = ctrlPts.size();
		outPts->clear();
		if(nbCtrlPts < 4)
		{
			cleanDirty();
			return;
		}

		int nbSteps = steps.getValue();
		computeCoefs(nbSteps);

		bool closed = (close.getValue() && ctrlPts.front() == ctrlPts.back());

		if(closed)
		{
			for(int i=1; i<=nbCtrlPts; ++i)
			{
				for(int j=0; j<nbSteps; ++j)
				{
					Point pt;
					vec4 coef = coefs[j];
					for(int k=0; k<4; ++k)
					{
						int index = i-1+k;
						if(index >= nbCtrlPts)
							index -= nbCtrlPts - 1; // We don't use the last point as it's the same as the first
						pt += ctrlPts[index] * coef[k];
					}

					outPts->push_back(pt);
				}
			}
		}
		else
		{
			for(int i=1; i<nbCtrlPts-2; ++i)
			{
				for(int j=0; j<nbSteps; ++j)
				{
					Point pt;
					vec4 coef = coefs[j];
					for(int k=0; k<4; ++k)
						pt += ctrlPts[i-1+k] * coef[k];

					outPts->push_back(pt);
				}
			}

			// Add the last point
			outPts->push_back(ctrlPts[nbCtrlPts-3] * 1/6.0
							+ ctrlPts[nbCtrlPts-2] * 4/6.0
							+ ctrlPts[nbCtrlPts-1] * 1/6.0);
		}

		cleanDirty();
	}

	void computeCoefs(int nbSteps)
	{
		if(coefs.size() == (nbSteps+1))
			return;

		coefs.resize(nbSteps+1);

		vec4 first = {1/(PReal)6.0, 4/(PReal)6.0, 1/(PReal)6.0, 0};
		coefs.front() = first;

		PReal fstep = 1.0 / nbSteps;
		int i=1;
		for(PReal t=fstep; t<1; t+=fstep)
		{
			PReal t2=t*t, t3=t2*t;
			PReal k1=1-3*t+3*t2-t3, k2=4-6*t2+3*t3, k3=1+3*t+3*t2-3*t3;

			vec4 tmp = {1/(PReal)6.0 * k1, 1/(PReal)6.0 * k2, 1/(PReal)6.0 * k3, 1/(PReal)6.0 * t3};
			coefs[i++] = tmp;
		}

		vec4 last = {0, 1/(PReal)6.0, 4/(PReal)6.0, 1/(PReal)6.0};
		coefs.back() = last;
	}

protected:
	Data<Path> input, output;
	Data<int> steps, close;

	typedef std::array<PReal, 4> vec4;
	QVector<vec4> coefs;
};

int Curve_CubicBSplineClass = RegisterObject<Curve_CubicBSpline>("Math/Path/Cubic B-spline").setDescription("Compute a cubic B-spline using the controls points");

} // namespace Panda


