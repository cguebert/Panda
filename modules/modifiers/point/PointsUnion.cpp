#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Point.h>

namespace panda {

using types::ImageWrapper;
using types::Point;

class ModifierPoints_Union : public PandaObject
{
public:
	PANDA_CLASS(ModifierPoints_Union, PandaObject)

	ModifierPoints_Union(PandaDocument *doc)
		: PandaObject(doc)
		, image(initData(&image, "image", "Image used to select valid points"))
		, inputPoints(initData(&inputPoints, "input", "List of points to test"))
		, outputPoints(initData(&outputPoints, "output", "List of valid points" ))
		, outputIndices(initData(&outputIndices, "indices", "Indices of the valid points"))
	{
		addInput(&image);
		addInput(&inputPoints);

		addOutput(&outputPoints);
		addOutput(&outputIndices);
	}

	void update()
	{
		const QImage& img = image.getValue().getImage();

		const QVector<Point>& inPts = inputPoints.getValue();
		auto outPts = outputPoints.getAccessor();
		auto outId = outputIndices.getAccessor();
		int nb = inPts.size();
		outPts.clear();
		outId.clear();

		if(!img.isNull())
		{
			for(int i=0; i<nb; ++i)
			{
				Point pt = inPts[i];
				QPoint iPt(qRound(pt.x), qRound(pt.y));
				if(img.valid(iPt))
				{
					QRgb color = img.pixel(iPt);
					if(qGray(color) > 128)
					{
						outPts.push_back(pt);
						outId.push_back(i);
					}
				}
			}
		}

		cleanDirty();
	}

protected:
	Data<ImageWrapper> image;
	Data< QVector<Point> > inputPoints, outputPoints;
	Data< QVector<int> > outputIndices;
};

int ModifierPoints_UnionClass = RegisterObject<ModifierPoints_Union>("Modifier/Point/Union").setName("Points union").setDescription("Remove points that are in dark areas of the image");

} // namespace Panda
