#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>

namespace panda {

using types::ImageWrapper;

class GeneratorImage_Load : public PandaObject
{
public:
	PANDA_CLASS(GeneratorImage_Load, PandaObject)

	GeneratorImage_Load(PandaDocument *doc)
		: PandaObject(doc)
		, m_fileName(initData(&m_fileName, "fileName", "Path of the image to load"))
		, m_image(initData(&m_image, "image", "The image loaded from disk"))
	{
		addInput(m_fileName);
		m_fileName.setWidget("open file");

		addOutput(m_image);
	}

	void update()
	{
		QString fileName = m_fileName.getValue();
		if(!fileName.isEmpty())
		{
			QImage image(fileName);
			if(!image.isNull())
				m_image.getAccessor()->setImage(image.convertToFormat(QImage::Format_ARGB32));
			else
				m_image.getAccessor()->clear();
		}
		else
			m_image.getAccessor()->clear();
		cleanDirty();
	}

protected:
	Data<QString> m_fileName;
	Data<ImageWrapper> m_image;
};

int GeneratorImage_LoadClass = RegisterObject<GeneratorImage_Load>("File/Image/Load image").setDescription("Load an image from the disk");

} // namespace Panda
