#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <helper/PointsGrid.h>

namespace panda {

class PointListMath_Nearest : public PandaObject
{
public:
	PANDA_CLASS(PointListMath_Nearest, PandaObject)

	PointListMath_Nearest(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "List of points to analyse"))
		, points(initData(&points, "points", "Points used for the search"))
		, output(initData(&output, "output", "Result of the search"))
		, unique(initData(&unique, 0, "unique", "Set this to 1 if multiple points can not find the same result"))
		, found(initData(&found, "found", "For each input point, this is 1 if a point was found, 0 otherwise"))
		, maxDist(initData(&maxDist, 100.0, "maxDist", "Distance maximum for the search"))
	{
		addInput(&input);
		addInput(&points);
		addInput(&unique);
		addInput(&maxDist);

		addOutput(&output);
		addOutput(&found);
	}

	void update()
	{
		const QVector<QPointF>& inPts = input.getValue();
		const QVector<QPointF>& searchPts = points.getValue();
		int nb = inPts.size();

		if(nb && searchPts.size())
		{
			double maxD = maxDist.getValue();
			QSize size = parentDocument->getRenderSize();
			QRectF area = QRectF(0, 0, size.width()-1, size.height()-1);
			grid.initGrid(area, qMax(maxD, 10.0));
			grid.addPoints(searchPts);

			QVector<QPointF>& outPts = *output.beginEdit();
			QVector<int>& outFound = *found.beginEdit();

			outPts.resize(nb);
			outFound.resize(nb);

			bool removePts = unique.getValue();
			for(int i=0; i<nb; ++i)
			{
				outPts[i] = inPts[i];
				outFound[i] = grid.getNearest(inPts[i], maxD, outPts[i]);
				if(removePts && outFound[i])
				{
					QPointF pt = outPts[i];
					grid.removePoint(pt);
				}
			}

			output.endEdit();
			found.endEdit();
		}

		this->cleanDirty();
	}

protected:
	helper::PointsGrid grid;

	Data< QVector<QPointF> > input, points, output;
	Data<int> unique;
	Data< QVector<int> > found;
	Data<double> maxDist;
};

int PointListMath_NearestClass = RegisterObject("Math/List of points/Nearest point")
		.setClass<PointListMath_Nearest>()
		.setDescription("For each point in the first list, find the nearest point in the second list");

} // namespace Panda

