#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Rect.h>

#include <panda/graphics/Framebuffer.h>
#include <panda/graphics/Image.h>

#include <cmath>

namespace panda {

using types::Point;
using types::Rect;
using types::ImageWrapper;

class ModifierImage_ExtractImage : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_ExtractImage, PandaObject)

	ModifierImage_ExtractImage(PandaDocument *doc)
		: PandaObject(doc)
		, image(initData("image", "The original image"))
		, rectangle(initData("rectangle", "Region to extract from the image"))
		, result(initData("result", "Image created by the operation"))
	{
		addInput(image);
		addInput(rectangle);

		addOutput(result);
	}

	void update()
	{
		const ImageWrapper& imgWrapper = image.getValue();
		const std::vector<Rect>& rectList = rectangle.getValue();
		auto resList = result.getAccessor();

		int nb = rectList.size();
		resList.resize(nb);

		if(imgWrapper.hasImage())
		{
			const auto& img = imgWrapper.getImage();
			for(int i=0; i<nb; ++i)
			{
				const Rect rect = rectList[i];
				graphics::Image tmpImg(std::floor(rect.width()), std::floor(rect.height()));
				graphics::Image::blitImage(tmpImg, 0, 0, img, rect.left(), rect.top(), rect.right(), rect.bottom());
				resList[i].setImage(tmpImg);
			}
		}
		else if(imgWrapper.hasTexture())
		{
			auto fbo = imgWrapper.getFbo();
			if(fbo)
			{
				for(int i=0; i<nb; ++i)
				{
					const Rect rect = rectList[i];
					graphics::Size size(std::floor(rect.width()), std::floor(rect.height()));
					if(!size.empty())
						continue;

					auto newFbo = graphics::Framebuffer(size);

					int l = std::floor(rect.left()), t = std::floor(rect.top()),
						w = std::floor(rect.width()), h = std::floor(rect.height());
					auto sourceSize = fbo->size();
					Rect sourceRect(l, sourceSize.height() - t - h, w, h);
					Rect targetRect(0, 0, size.width(), size.height());
					graphics::Framebuffer::blitFramebuffer(newFbo, targetRect, *fbo, sourceRect);
					resList[i].setFbo(newFbo);
				}
			}
		}

		cleanDirty();
	}

protected:
	Data< ImageWrapper > image;
	Data< std::vector< Rect > > rectangle;
	Data< std::vector< ImageWrapper > > result;
};

int ModifierImage_ExtractImageClass = RegisterObject<ModifierImage_ExtractImage>("Modifier/Image/Extract image").setDescription("Extract a region of an image to create a new one.");

} // namespace Panda
