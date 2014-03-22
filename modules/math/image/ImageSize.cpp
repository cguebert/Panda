#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>

namespace panda {

using types::ImageWrapper;
using types::Point;

class ImageMath_GetSize : public PandaObject
{
public:
	PANDA_CLASS(ImageMath_GetSize, PandaObject)

	ImageMath_GetSize(PandaDocument *doc)
		: PandaObject(doc)
		, image(initData(&image, "image", "Image to analyse"))
		, size(initData(&size, "size", "size of the image"))
	{
		addInput(&image);

		addOutput(&size);
	}

	void update()
	{
		const QVector<ImageWrapper>& listImage = image.getValue();
		auto listSize = size.getAccessor();

		unsigned int nb = listImage.size();
		listSize.clear();
		listSize.resize(nb);

		for(unsigned int i=0; i<nb; ++i)
		{
			const QImage img = listImage[i].getImage();
			if(!img.isNull())
			{
				QSize s = img.size();
				listSize[i] = Point(s.width(), s.height());
			}
		}

		cleanDirty();
	}

protected:
	Data< QVector<ImageWrapper> > image;
	Data< QVector<Point> > size;
};

int ImageMath_GetSizeClass = RegisterObject<ImageMath_GetSize>("Math/Image/Image size").setDescription("Compute the size of the image");

} // namespace Panda


