#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

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
		const QVector<QImage>& listImage = image.getValue();
		QVector<QPointF>& listSize = *size.beginEdit();

		unsigned int nb = listImage.size();
		listSize.clear();
		listSize.resize(nb);

		for(unsigned int i=0; i<nb; ++i)
		{
			if(!listImage[i].isNull())
			{
				QSize s = listImage[i].size();
				listSize[i] = QPointF(s.width(), s.height());
			}
		}

		size.endEdit();
		this->cleanDirty();
	}

protected:
	Data< QVector<QImage> > image;
	Data< QVector<QPointF> > size;
};

int ImageMath_GetSizeClass = RegisterObject("Math/Image/Image size").setClass<ImageMath_GetSize>().setDescription("Compute the size of the image");

} // namespace Panda


