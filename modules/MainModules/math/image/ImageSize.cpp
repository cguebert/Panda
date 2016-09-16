#include <panda/object/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Point.h>

namespace panda {

using types::ImageWrapper;
using types::Point;

class ImageMath_GetSize : public PandaObject
{
public:
	PANDA_CLASS(ImageMath_GetSize, PandaObject)

	ImageMath_GetSize(PandaDocument *doc)
		: PandaObject(doc)
		, image(initData("image", "Image to analyse"))
		, size(initData("size", "size of the image"))
	{
		addInput(image);

		addOutput(size);
	}

	void update()
	{
		const std::vector<ImageWrapper>& listImage = image.getValue();
		auto listSize = size.getAccessor();

		unsigned int nb = listImage.size();
		listSize.clear();
		listSize.resize(nb);

		for(unsigned int i=0; i<nb; ++i)
		{
			const ImageWrapper& imgWrapper = listImage[i];
			auto size = imgWrapper.size();
			listSize[i] = Point(static_cast<float>(size.width()), static_cast<float>(size.height()));
		}
	}

protected:
	Data< std::vector<ImageWrapper> > image;
	Data< std::vector<Point> > size;
};

int ImageMath_GetSizeClass = RegisterObject<ImageMath_GetSize>("Math/Image/Image size").setDescription("Compute the size of the image");

} // namespace Panda


