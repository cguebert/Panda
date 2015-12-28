#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Rect.h>

#include <QOpenGLFramebufferObject>
#include <QPainter>
#include <cmath>

namespace panda {

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
			const QImage& img = imgWrapper.getImage();
			for(int i=0; i<nb; ++i)
			{
				const Rect rect = rectList[i];
				QImage tmpImg(std::floor(rect.width()), std::floor(rect.height()), QImage::Format_ARGB32);
				tmpImg.fill(QColor(0,0,0,0));
				QPainter painter(&tmpImg);
				painter.drawImage(0, 0, img, rect.left(), rect.top(), rect.width(), rect.height());
				resList[i].setImage(tmpImg);
			}
		}
		else if(imgWrapper.hasTexture())
		{
			QOpenGLFramebufferObject* fbo = imgWrapper.getFbo();
			if(fbo)
			{
				for(int i=0; i<nb; ++i)
				{
					const Rect rect = rectList[i];
					QSize size = QSize(std::floor(rect.width()), std::floor(rect.height()));
					if(!size.isValid())
						continue;

					auto newFbo = std::make_shared<QOpenGLFramebufferObject>(size);

					int l = std::floor(rect.left()), t = std::floor(rect.top()),
						w = std::floor(rect.width()), h = std::floor(rect.height());
					QSize sourceSize = fbo->size();
					QRect sourceRect = QRect(l, sourceSize.height() - t - h, w, h);
					QRect targetRect = QRect(QPoint(0, 0), size);
					QOpenGLFramebufferObject::blitFramebuffer(newFbo.get(), targetRect, fbo, sourceRect);
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
