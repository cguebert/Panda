#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>

#include <QPainter>

namespace panda {

class RenderImage : public Renderer
{
public:
	PANDA_CLASS(RenderImage, Renderer)

	RenderImage(PandaDocument *parent)
		: Renderer(parent)
		, image(initData(&image, "image", "Image to render on screen" ))
		, center(initData(&center, "center", "Center position of the image"))
	{
		addInput(&image);
		addInput(&center);

		image.setDisplayed(false);

		center.beginEdit()->append(QPointF(100, 100));
		center.endEdit();
	}

	void render(QPainter* painter)
	{
		const QVector<QImage>& listImage = image.getValue();
		const QVector<QPointF>& listCenter = center.getValue();

		int nbImage = listImage.size();
		int nbCenter = listCenter.size();

		if(nbImage && nbCenter)
		{
			if(nbImage < nbCenter) nbImage = 1;
			for(int i=0; i<nbCenter; ++i)
			{
				painter->drawImage(listCenter[i], listImage[i % nbImage]);
			}
		}
	}

protected:
	Data< QVector<QImage> > image;
	Data< QVector<QPointF> > center;
};

int RenderImageClass = RegisterObject("Render/Image").setClass<RenderImage>().setDescription("Renders an image");

} // namespace panda
