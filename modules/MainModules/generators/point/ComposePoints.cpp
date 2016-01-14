#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

using types::Point;

class GeneratorPoints_Compose : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPoints_Compose, PandaObject)

	GeneratorPoints_Compose(PandaDocument *doc)
		: PandaObject(doc)
		, ptX(initData("x", "Horizontal position of the point"))
		, ptY(initData("y", "Vertical position of the point"))
		, point(initData("point", "Point created from the 2 coordinates"))
	{
		addInput(ptX);
		addInput(ptY);

		addOutput(point);
	}

	void update()
	{
		const std::vector<PReal>	&xVal = ptX.getValue(),
								&yVal = ptY.getValue();
		auto pts = point.getAccessor();

		int nbx = xVal.size(), nby = yVal.size();
		if (nbx && nby)
		{
			int nb = qMax(nbx, nby);
			if (nbx < nby) nbx = 1;
			else if (nby < nbx) nby = 1;
			pts.resize(nb);
			for (int i = 0; i < nb; ++i)
				pts[i] = Point(xVal[i%nbx], yVal[i%nby]);
		}
		else
			pts.clear();

		cleanDirty();
	}

protected:
	Data< std::vector<PReal> > ptX, ptY;
	Data< std::vector<Point> > point;
};

int GeneratorPoints_ComposeClass = RegisterObject<GeneratorPoints_Compose>("Generator/Point/Compose point").setName("Reals to point").setDescription("Create a point from 2 reals");

//****************************************************************************//

class GeneratorPoints_Decompose : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPoints_Decompose, PandaObject)

	GeneratorPoints_Decompose(PandaDocument *doc)
		: PandaObject(doc)
		, ptX(initData("x", "Horizontal position of the point"))
		, ptY(initData("y", "Vertical position of the point"))
		, point(initData("point", "Point from which to extract the 2 coordinates"))
	{
		addInput(point);

		addOutput(ptX);
		addOutput(ptY);
	}

	void update()
	{
		auto xVal = ptX.getAccessor();
		auto yVal = ptY.getAccessor();
		const std::vector<Point>& pts = point.getValue();

		int nb = pts.size();
		xVal.resize(nb);
		yVal.resize(nb);
		for(int i=0; i<nb; ++i)
		{
			const Point pt = pts[i];
			xVal[i] = pt.x;
			yVal[i] = pt.y;
		}

		cleanDirty();
	}

protected:
	Data< std::vector<PReal> > ptX, ptY;
	Data< std::vector<Point> > point;
};

int GeneratorPoints_DecomposeClass = RegisterObject<GeneratorPoints_Decompose>("Generator/Point/Decompose point").setName("Point to reals").setDescription("Extract the coordinates of a point");


} // namespace Panda
