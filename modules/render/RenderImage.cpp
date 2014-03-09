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
		, rotation(initData(&rotation, "rotation", "Rotation of the image"))
		, drawCentered(initData(&drawCentered, 0, "drawCentered", "If non zero use the center of the image, else use the top-left corner"))
	{
		addInput(&image);
		addInput(&center);
		addInput(&rotation);
		addInput(&drawCentered);

		center.getAccessor().push_back(QPointF(0, 0));
	}
/*
	void render(QPainter* painter)
	{
		const QVector<QImage>& listImage = image.getValue();
		const QVector<QPointF>& listCenter = center.getValue();
		const QVector<double>& listRotation = rotation.getValue();

		bool centered = (drawCentered.getValue() != 0);

		int nbImage = listImage.size();
		int nbCenter = listCenter.size();
		int nbRotation = listRotation.size();

		if(nbImage && nbCenter)
		{
			if(nbImage < nbCenter) nbImage = 1;
			if(nbRotation && nbRotation < nbCenter) nbRotation = 1;

			if(nbRotation)
			{
				if(centered)
				{
					for(int i=0; i<nbCenter; ++i)
					{
						QSize s = listImage[i % nbImage].size();
						painter->save();
						painter->translate(listCenter[i].x(),
										   listCenter[i].y());
						painter->rotate(listRotation[i % nbRotation]);
						painter->drawImage(-s.width()/2, -s.height()/2, listImage[i % nbImage]);
						painter->restore();
					}
				}
				else
				{
					for(int i=0; i<nbCenter; ++i)
					{
						painter->save();
						painter->translate(listCenter[i]);
						painter->rotate(listRotation[i % nbRotation]);
						painter->drawImage(0, 0, listImage[i % nbImage]);
						painter->restore();
					}
				}
			}
			else
			{
				if(centered)
				{
					for(int i=0; i<nbCenter; ++i)
					{
						QSize s = listImage[i % nbImage].size();
						painter->drawImage(listCenter[i].x() - s.width()/2,
										   listCenter[i].y() - s.height()/2,
										   listImage[i % nbImage]);
					}
				}
				else
				{
					for(int i=0; i<nbCenter; ++i)
						painter->drawImage(listCenter[i], listImage[i % nbImage]);
				}
			}
		}
	}
*/
	void render()
	{
	}

protected:
	Data< QVector<QImage> > image;
	Data< QVector<QPointF> > center;
	Data< QVector<double> > rotation;
	Data< int > drawCentered;
};

int RenderImageClass = RegisterObject<RenderImage>("Render/Image").setDescription("Renders an image");

} // namespace panda
