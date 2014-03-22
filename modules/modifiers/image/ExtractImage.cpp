#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Rect.h>
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
		, image(initData(&image, "image", "The original image"))
		, rectangle(initData(&rectangle, "rectangle", "Region to extract from the image"))
		, result(initData(&result, "result", "Image created by the operation"))
	{
		addInput(&image);
		addInput(&rectangle);

		addOutput(&result);
	}

	void update()
	{
		const QImage& img = image.getValue().getImage();
		const QVector<Rect>& rectList = rectangle.getValue();
		auto resList = result.getAccessor();

		int nb = rectList.size();
		resList.resize(nb);
		for(int i=0; i<nb; ++i)
		{
			const Rect rect = rectList[i];
			QImage tmpImg(std::floor(rect.width()), std::floor(rect.height()), QImage::Format_ARGB32);
			tmpImg.fill(QColor(0,0,0,0));
			QPainter painter(&tmpImg);
			painter.drawImage(0, 0, img, rect.left(), rect.top(), rect.width(), rect.height());
			resList[i].setImage(tmpImg);
		}

		cleanDirty();
	}

protected:
	Data< ImageWrapper > image;
	Data< QVector< Rect > > rectangle;
	Data< QVector< ImageWrapper > > result;
};

int ModifierImage_ExtractImageClass = RegisterObject<ModifierImage_ExtractImage>("Modifier/Image/Extract image").setDescription("Extract a region of an image to create a new one.");

} // namespace Panda
