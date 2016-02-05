#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
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
		, rectangle(initData("rectangle", "Rectangle used to select valid points"))
		, inputPoints(initData("input", "List of points to test"))
		, outputPoints(initData("output", "List of valid points" ))
		, outputIndices(initData("indices", "Indices of the valid points"))
	{
		addInput(rectangle);
		addInput(inputPoints);

		addOutput(outputPoints);
		addOutput(outputIndices);
	}

	void update()
	{
		const Rect& rect = rectangle.getValue();

		const std::vector<Point>& inPts = inputPoints.getValue();
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
	Data< std::vector<Point> > inputPoints, outputPoints;
	Data< std::vector<int> > outputIndices;
};

int ModifierPoints_PointsInRectClass = RegisterObject<ModifierPoints_PointsInRect>("Modifier/Point/Points in rectangle").setDescription("Select points that are inside a rectangle");

//****************************************************************************//

class ModifierPoints_PointsInDisk : public PandaObject
{
public:
	PANDA_CLASS(ModifierPoints_PointsInDisk, PandaObject)

	ModifierPoints_PointsInDisk(PandaDocument *doc)
		: PandaObject(doc)
		, center(initData("center", "Center of the disk used to select valid points"))
		, radius(initData((float)50, "radius", "Radius of the disk used to select valid points"))
		, inputPoints(initData("input", "List of points to test"))
		, outputPoints(initData("output", "List of valid points" ))
		, outputIndices(initData("indices", "Indices of the valid points"))
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
		float r = radius.getValue();
		float r2 = r*r;

		const std::vector<Point>& inPts = inputPoints.getValue();
		auto outPts = outputPoints.getAccessor();
		auto outId = outputIndices.getAccessor();
		int nb = inPts.size();
		outPts.clear();
		outId.clear();

		for(int i=0; i<nb; ++i)
		{
			Point pt = inPts[i];
			float d2 = (pt - c).norm2();
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
	Data< float > radius;
	Data< std::vector<Point> > inputPoints, outputPoints;
	Data< std::vector<int> > outputIndices;
};

int ModifierPoints_PointsInDiskClass = RegisterObject<ModifierPoints_PointsInDisk>("Modifier/Point/Points in disk").setDescription("Select points that are inside a disk");


} // namespace Panda
