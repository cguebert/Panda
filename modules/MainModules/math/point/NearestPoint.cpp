#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/helper/PointsGrid.h>

#include <algorithm>

namespace panda {

using types::Point;
using types::Rect;

class PointListMath_Nearest : public PandaObject
{
public:
	PANDA_CLASS(PointListMath_Nearest, PandaObject)

	PointListMath_Nearest(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("input", "List of points to analyse"))
		, points(initData("points", "Points used for the search"))
		, output(initData("output", "Result of the search"))
		, unique(initData(0, "unique", "If true, multiple points can not find the same result"))
		, notSelf(initData(0, "notSelf", "If true, a point can not return itself as the result"))
		, found(initData("found", "For each input point, this is 1 if a point was found, 0 otherwise"))
		, maxDist(initData((float)100.0, "maxDist", "Distance maximum for the search"))
	{
		addInput(input);
		addInput(points);
		addInput(unique);
		addInput(notSelf);
		addInput(maxDist);

		unique.setWidget("checkbox");
		notSelf.setWidget("checkbox");

		addOutput(output);
		addOutput(found);
	}

	void update()
	{
		const std::vector<Point>& inPts = input.getValue();
		const std::vector<Point>& searchPts = points.getValue();
		int nb = inPts.size();

		if(nb && searchPts.size())
		{
			float maxD = maxDist.getValue();
			auto size = m_parentDocument->getRenderSize();
			Rect area = Rect(0, 0, static_cast<float>(size.width()-1), static_cast<float>(size.height()-1));
			grid.initGrid(area, std::max<float>(maxD, 10.0));
			grid.addPoints(searchPts);

			auto outPts = output.getAccessor();
			auto outFound = found.getAccessor();

			outPts.resize(nb);
			outFound.resize(nb);

			bool removePts = unique.getValue() != 0;
			bool filterSelf = notSelf.getValue() != 0;
			for(int i=0; i<nb; ++i)
			{
				int removedSelfNb = 0;
				if(filterSelf && grid.hasPoint(inPts[i]))
					removedSelfNb = grid.removePoint(inPts[i]);

				outPts[i] = inPts[i];
				outFound[i] = grid.getNearest(inPts[i], maxD, outPts[i]);
				if(removePts && outFound[i])
				{
					Point pt = outPts[i];
					grid.removePoint(pt);
				}

				for(int j=0; j<removedSelfNb; ++j)
					grid.addPoint(inPts[i]);
			}
		}

		cleanDirty();
	}

protected:
	helper::PointsGrid grid;

	Data< std::vector<Point> > input, points, output;
	Data<int> unique, notSelf;
	Data< std::vector<int> > found;
	Data<float> maxDist;
};

int PointListMath_NearestClass = RegisterObject<PointListMath_Nearest>("Math/List of points/Nearest point")
		.setDescription("For each point in the first list, find the nearest point in the second list");

} // namespace Panda


