#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QPointF>

namespace panda {

class ModifierPoints_PointsInRect : public PandaObject
{
public:
	PANDA_CLASS(ModifierPoints_PointsInRect, PandaObject)

	ModifierPoints_PointsInRect(PandaDocument *doc)
		: PandaObject(doc)
		, rectangle(initData(&rectangle, "rectangle", "Rectangle used to select valid points"))
		, inputPoints(initData(&inputPoints, "input", "List of points to test"))
		, outputPoints(initData(&outputPoints, "output", "List of valid points" ))
		, outputIndices(initData(&outputIndices, "indices", "Indices of the valid points"))
	{
		addInput(&rectangle);
		addInput(&inputPoints);

		addOutput(&outputPoints);
		addOutput(&outputIndices);
	}

	void update()
	{
		const QRectF& rect = rectangle.getValue();

		const QVector<QPointF>& inPts = inputPoints.getValue();
		auto outPts = outputPoints.getAccessor();
		auto outId = outputIndices.getAccessor();
		int nb = inPts.size();
		outPts.clear();
		outId.clear();

		for(int i=0; i<nb; ++i)
		{
			QPointF pt = inPts[i];
			if(rect.contains(pt))
			{
				outPts.push_back(pt);
				outId.push_back(i);
			}
		}

		cleanDirty();
	}

protected:
	Data< QRectF > rectangle;
	Data< QVector<QPointF> > inputPoints, outputPoints;
	Data< QVector<int> > outputIndices;
};

int ModifierPoints_PointsInRectClass = RegisterObject<ModifierPoints_PointsInRect>("Modifier/Point/Points in rectangle").setDescription("Select points that are inside a rectangle");

//*************************************************************************//

class ModifierPoints_PointsInDisk : public PandaObject
{
public:
	PANDA_CLASS(ModifierPoints_PointsInDisk, PandaObject)

	ModifierPoints_PointsInDisk(PandaDocument *doc)
		: PandaObject(doc)
		, center(initData(&center, "center", "Center of the disk used to select valid points"))
		, radius(initData(&radius, (PReal)50, "radius", "Radius of the disk used to select valid points"))
		, inputPoints(initData(&inputPoints, "input", "List of points to test"))
		, outputPoints(initData(&outputPoints, "output", "List of valid points" ))
		, outputIndices(initData(&outputIndices, "indices", "Indices of the valid points"))
	{
		addInput(&center);
		addInput(&radius);
		addInput(&inputPoints);

		addOutput(&outputPoints);
		addOutput(&outputIndices);
	}

	void update()
	{
		const QPointF& c = center.getValue();
		double r = radius.getValue();
		double r2 = r*r;

		const QVector<QPointF>& inPts = inputPoints.getValue();
		auto outPts = outputPoints.getAccessor();
		auto outId = outputIndices.getAccessor();
		int nb = inPts.size();
		outPts.clear();
		outId.clear();

		for(int i=0; i<nb; ++i)
		{
			QPointF pt = inPts[i];
			double dx = pt.x()-c.x(), dy = pt.y()-c.y();
			double d2 = dx*dx+dy*dy;
			if(d2 < r2)
			{
				outPts.push_back(pt);
				outId.push_back(i);
			}
		}

		cleanDirty();
	}

protected:
	Data< QPointF > center;
	Data< PReal > radius;
	Data< QVector<QPointF> > inputPoints, outputPoints;
	Data< QVector<int> > outputIndices;
};

int ModifierPoints_PointsInDiskClass = RegisterObject<ModifierPoints_PointsInDisk>("Modifier/Point/Points in disk").setDescription("Select points that are inside a disk");


} // namespace Panda
