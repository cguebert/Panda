#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
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
		, m_input(initData("input", "List of control points"))
		, m_output(initData("output", "List of points on the spline"))
		, m_steps(initData(6, "steps", "Number of points on each segment"))
		, m_close(initData(0, "close", "Is the curve closed or not?"))
	{
		addInput(m_input);
		addInput(m_steps);
		addInput(m_close);
		m_close.setWidget("checkbox");

		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto output = m_output.getAccessor();
		output.clear();
		output.resize(input.size());

		int nbSteps = m_steps.getValue();
		computeCoefs(nbSteps);

		for(int l=0, nb=input.size(); l<nb; ++l)
		{
			const auto ctrlPts = input[l];
			int nbCtrlPts = ctrlPts.size();
			if(nbCtrlPts < 4)
				continue;
			auto& outPts = output[l];
			bool closed = (m_close.getValue() && ctrlPts.front() == ctrlPts.back());

			if(closed)
			{
				for(int i=1; i<=nbCtrlPts; ++i)
				{
					for(int j=0; j<nbSteps; ++j)
					{
						Point pt;
						vec4 coef = m_coefs[j];
						for(int k=0; k<4; ++k)
						{
							int index = i-1+k;
							if(index >= nbCtrlPts)
								index -= nbCtrlPts - 1; // We don't use the last point as it's the same as the first
							pt += ctrlPts[index] * coef[k];
						}

						outPts.push_back(pt);
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
						vec4 coef = m_coefs[j];
						for(int k=0; k<4; ++k)
							pt += ctrlPts[i-1+k] * coef[k];

						outPts.push_back(pt);
					}
				}

				// Add the last point
				outPts.push_back(ctrlPts[nbCtrlPts-3] * 1/6.0
								+ ctrlPts[nbCtrlPts-2] * 4/6.0
								+ ctrlPts[nbCtrlPts-1] * 1/6.0);
			}
		}

		cleanDirty();
	}

	void computeCoefs(int nbSteps)
	{
		if(m_coefs.size() == (nbSteps+1))
			return;

		m_coefs.resize(nbSteps+1);

		vec4 first = {1/(float)6.0, 4/(float)6.0, 1/(float)6.0, 0};
		m_coefs.front() = first;

		float fstep = 1.f / nbSteps;
		int i=1;
		for(float t=fstep; t<1; t+=fstep)
		{
			float t2=t*t, t3=t2*t;
			float k1=1-3*t+3*t2-t3, k2=4-6*t2+3*t3, k3=1+3*t+3*t2-3*t3;

			vec4 tmp = {1/(float)6.0 * k1, 1/(float)6.0 * k2, 1/(float)6.0 * k3, 1/(float)6.0 * t3};
			m_coefs[i++] = tmp;
		}

		vec4 last = {0, 1/(float)6.0, 4/(float)6.0, 1/(float)6.0};
		m_coefs.back() = last;
	}

protected:
	Data<std::vector<Path>> m_input, m_output;
	Data<int> m_steps, m_close;

	typedef std::array<float, 4> vec4;
	std::vector<vec4> m_coefs;
};

int Curve_CubicBSplineClass = RegisterObject<Curve_CubicBSpline>("Math/Path/Cubic B-spline").setDescription("Compute a cubic B-spline using the controls points");

} // namespace Panda


