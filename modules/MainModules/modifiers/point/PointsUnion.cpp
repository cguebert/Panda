#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Point.h>

#include <panda/graphics/Image.h>

namespace panda {

using types::ImageWrapper;
using types::Point;

class ModifierPoints_Union : public PandaObject
{
public:
	PANDA_CLASS(ModifierPoints_Union, PandaObject)

	ModifierPoints_Union(PandaDocument *doc)
		: PandaObject(doc)
		, image(initData("image", "Image used to select valid points"))
		, inputPoints(initData("input", "List of points to test"))
		, outputPoints(initData("output", "List of valid points" ))
		, outputIndices(initData("indices", "Indices of the valid points"))
	{
		addInput(image);
		addInput(inputPoints);

		addOutput(outputPoints);
		addOutput(outputIndices);

		setUpdateOnMainThread(true);
	}

	void update()
	{
		const auto& img = image.getValue().getImage();

		const std::vector<Point>& inPts = inputPoints.getValue();
		auto outPts = outputPoints.getAccessor();
		auto outId = outputIndices.getAccessor();
		int nb = inPts.size();
		outPts.clear();
		outId.clear();

		if(img)
		{
			for(int i=0; i<nb; ++i)
			{
				Point pt = inPts[i];
				graphics::PointInt iPt(std::lround(pt.x), std::lround(pt.y));
				if(img.valid(iPt))
				{
					if(graphics::gray(img.pixel(iPt)) > 128)
					{
						outPts.push_back(pt);
						outId.push_back(i);
					}
				}
			}
		}
	}

protected:
	Data<ImageWrapper> image;
	Data< std::vector<Point> > inputPoints, outputPoints;
	Data< std::vector<int> > outputIndices;
};

int ModifierPoints_UnionClass = RegisterObject<ModifierPoints_Union>("Modifier/Point/Union").setName("Points union").setDescription("Remove points that are in dark areas of the image");

} // namespace Panda
