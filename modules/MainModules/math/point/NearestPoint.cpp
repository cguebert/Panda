#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/helper/PointsGrid.h>

namespace panda {

using types::Point;
using types::Rect;

class PointListMath_Nearest : public PandaObject
{
public:
	PANDA_CLASS(PointListMath_Nearest, PandaObject)

	PointListMath_Nearest(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "List of points to analyse"))
		, points(initData(&points, "points", "Points used for the search"))
		, output(initData(&output, "output", "Result of the search"))
		, unique(initData(&unique, 0, "unique", "Set to 1 if multiple points can not find the same result"))
		, notSelf(initData(&notSelf, 0, "notSelf", "Set to 1 if a point can not return itself as the result"))
		, found(initData(&found, "found", "For each input point, this is 1 if a point was found, 0 otherwise"))
		, maxDist(initData(&maxDist, (PReal)100.0, "maxDist", "Distance maximum for the search"))
	{
		addInput(input);
		addInput(points);
		addInput(unique);
		addInput(notSelf);
		addInput(maxDist);

		addOutput(output);
		addOutput(found);
	}

	void update()
	{
		const QVector<Point>& inPts = input.getValue();
		const QVector<Point>& searchPts = points.getValue();
		int nb = inPts.size();

		if(nb && searchPts.size())
		{
			PReal maxD = maxDist.getValue();
			QSize size = m_parentDocument->getRenderSize();
			Rect area = Rect(0, 0, size.width()-1, size.height()-1);
			grid.initGrid(area, qMax<PReal>(maxD, 10.0));
			grid.addPoints(searchPts);

			auto outPts = output.getAccessor();
			auto outFound = found.getAccessor();

			outPts.resize(nb);
			outFound.resize(nb);

			bool removePts = unique.getValue();
			bool filterSelf = notSelf.getValue();
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

	Data< QVector<Point> > input, points, output;
	Data<int> unique, notSelf;
	Data< QVector<int> > found;
	Data<PReal> maxDist;
};

int PointListMath_NearestClass = RegisterObject<PointListMath_Nearest>("Math/List of points/Nearest point")
		.setDescription("For each point in the first list, find the nearest point in the second list");

} // namespace Panda


