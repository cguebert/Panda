#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Rect.h>

namespace panda {

using types::Point;
using types::Rect;

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
		addInput(rectangle);
		addInput(inputPoints);

		addOutput(outputPoints);
		addOutput(outputIndices);
	}

	void update()
	{
		const Rect& rect = rectangle.getValue();

		const QVector<Point>& inPts = inputPoints.getValue();
		auto outPts = outputPoints.getAccessor();
		auto outId = outputIndices.getAccessor();
		int nb = inPts.size();
		outPts.clear();
		outId.clear();

		for(int i=0; i<nb; ++i)
		{
			Point pt = inPts[i];
			if(rect.contains(pt))
			{
				outPts.push_back(pt);
				outId.push_back(i);
			}
		}

		cleanDirty();
	}

protected:
	Data< Rect > rectangle;
	Data< QVector<Point> > inputPoints, outputPoints;
	Data< QVector<int> > outputIndices;
};

int ModifierPoints_PointsInRectClass = RegisterObject<ModifierPoints_PointsInRect>("Modifier/Point/Points in rectangle").setDescription("Select points that are inside a rectangle");

//****************************************************************************//

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
		addInput(center);
		addInput(radius);
		addInput(inputPoints);

		addOutput(outputPoints);
		addOutput(outputIndices);
	}

	void update()
	{
		const Point& c = center.getValue();
		PReal r = radius.getValue();
		PReal r2 = r*r;

		const QVector<Point>& inPts = inputPoints.getValue();
		auto outPts = outputPoints.getAccessor();
		auto outId = outputIndices.getAccessor();
		int nb = inPts.size();
		outPts.clear();
		outId.clear();

		for(int i=0; i<nb; ++i)
		{
			Point pt = inPts[i];
			PReal d2 = (pt - c).norm2();
			if(d2 < r2)
			{
				outPts.push_back(pt);
				outId.push_back(i);
			}
		}

		cleanDirty();
	}

protected:
	Data< Point > center;
	Data< PReal > radius;
	Data< QVector<Point> > inputPoints, outputPoints;
	Data< QVector<int> > outputIndices;
};

int ModifierPoints_PointsInDiskClass = RegisterObject<ModifierPoints_PointsInDisk>("Modifier/Point/Points in disk").setDescription("Select points that are inside a disk");


} // namespace Panda
