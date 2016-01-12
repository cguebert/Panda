#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/graphics/Image.h>

#include <QImage>

namespace panda {

using types::ImageWrapper;

class GeneratorImage_Load : public PandaObject
{
public:
	PANDA_CLASS(GeneratorImage_Load, PandaObject)

	GeneratorImage_Load(PandaDocument *doc)
		: PandaObject(doc)
		, m_fileName(initData("fileName", "Path of the image to load"))
		, m_image(initData("image", "The image loaded from disk"))
	{
		addInput(m_fileName);
		m_fileName.setWidget("open file");

		addOutput(m_image);
	}

	void update()
	{
		std::string fileName = m_fileName.getValue();
		if(!fileName.empty())
		{
			QImage image(QString::fromStdString(fileName));
			if (!image.isNull())
			{
				image.convertToFormat(QImage::Format_ARGB32);
				image = image.rgbSwapped();
				graphics::Size size(image.width(), image.height());
				graphics::Image newImg(size, image.bits());
				m_image.getAccessor()->setImage(newImg);
			}
			else
				m_image.getAccessor()->clear();
		}
		else
			m_image.getAccessor()->clear();
		cleanDirty();
	}

protected:
	Data<std::string> m_fileName;
	Data<ImageWrapper> m_image;
};

int GeneratorImage_LoadClass = RegisterObject<GeneratorImage_Load>("File/Image/Load image").setDescription("Load an image from the disk");

} // namespace Panda
