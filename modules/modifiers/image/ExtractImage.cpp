#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QImage>
#include <QPainter>

namespace panda {

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
		const QImage& img = image.getValue();
		const QVector<QRectF>& rectList = rectangle.getValue();
		auto resList = result.getAccessor();

		int nb = rectList.size();
		resList.resize(nb);
		for(int i=0; i<nb; ++i)
		{
			const QRect rect = rectList[i].toRect();
			resList[i] = QImage(rect.width(), rect.height(), QImage::Format_ARGB32);
			QPainter painter(&resList[i]);
			painter.drawImage(0, 0, img, rect.left(), rect.top(), rect.width(), rect.height());
		}

		this->cleanDirty();
	}

protected:
	Data< QImage > image;
	Data< QVector< QRectF > > rectangle;
	Data< QVector< QImage > > result;
};

int ModifierImage_ExtractImageClass = RegisterObject<ModifierImage_ExtractImage>("Modifier/Image/Extract image").setDescription("Extract a region of an image to create a new one.");

} // namespace Panda
