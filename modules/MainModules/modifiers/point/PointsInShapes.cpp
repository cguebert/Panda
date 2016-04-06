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
		, output(initData("isInside", "For each input points, 1 if it is inside the rectangle, 0 otherwise"))
	{
		addInput(rectangle);
		addInput(inputPoints);

		addOutput(output);
	}

	void update()
	{
		const Rect& rect = rectangle.getValue();

		const std::vector<Point>& inPts = inputPoints.getValue();
		auto outVal = output.getAccessor();
		int nb = inPts.size();
		outVal.resize(nb);

		for (int i = 0; i < nb; ++i)
			outVal[i] = rect.contains(inPts[i]) ? 1 : 0;
	}

protected:
	Data< Rect > rectangle;
	Data< std::vector<Point> > inputPoints;
	Data< std::vector<int> > output;
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
		, output(initData("isInside", "For each input points, 1 if it is inside the rectangle, 0 otherwise"))
	{
		addInput(center);
		addInput(radius);
		addInput(inputPoints);

		addOutput(output);
	}

	void update()
	{
		const Point& c = center.getValue();
		float r = radius.getValue();
		float r2 = r*r;

		const std::vector<Point>& inPts = inputPoints.getValue();
		auto outVal = output.getAccessor();
		int nb = inPts.size();
		outVal.clear();
		outVal.resize(nb);

		for (int i = 0; i < nb; ++i)
		{
			Point pt = inPts[i];
			float d2 = (pt - c).norm2();
			outVal[i] = (d2 <= r2) ? 1 : 0;
		}
	}

protected:
	Data< Point > center;
	Data< float > radius;
	Data< std::vector<Point> > inputPoints;
	Data< std::vector<int> > output;
};

int ModifierPoints_PointsInDiskClass = RegisterObject<ModifierPoints_PointsInDisk>("Modifier/Point/Points in disk").setDescription("Select points that are inside a disk");

} // namespace Panda
