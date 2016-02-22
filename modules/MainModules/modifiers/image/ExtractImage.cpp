#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
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
				int l = static_cast<int>(rect.left()),  t = static_cast<int>(rect.top()),
					w = static_cast<int>(rect.width()), h = static_cast<int>(rect.height());
				graphics::Image tmpImg(w, h);
				graphics::Image::blitImage(tmpImg, 0, 0, img, l, t, l+w, t+h);
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
					int l = static_cast<int>(rect.left()),  t = static_cast<int>(rect.top()),
						w = static_cast<int>(rect.width()), h = static_cast<int>(rect.height());

					graphics::Size size(w, h);
					if(!size.empty())
						continue;

					auto newFbo = graphics::Framebuffer(size);
					auto sourceSize = fbo->size();
					graphics::RectInt sourceRect(l, sourceSize.height() - t - h, w, h);
					graphics::RectInt targetRect(0, 0, size.width(), size.height());
					graphics::Framebuffer::blitFramebuffer(newFbo, targetRect, *fbo, sourceRect);
					resList[i].setFbo(newFbo);
				}
			}
		}
	}

protected:
	Data< ImageWrapper > image;
	Data< std::vector< Rect > > rectangle;
	Data< std::vector< ImageWrapper > > result;
};

int ModifierImage_ExtractImageClass = RegisterObject<ModifierImage_ExtractImage>("Modifier/Image/Extract image").setDescription("Extract a region of an image to create a new one.");

} // namespace Panda
