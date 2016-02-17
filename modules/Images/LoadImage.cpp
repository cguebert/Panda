#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/graphics/Image.h>

#include <sstream>
#include <FreeImage.h>

namespace
{

std::string convertExtensionList(const std::string& list)
{
	std::stringstream ss(list);
	std::string ret, item;
	while (std::getline(ss, item, ','))
		ret += (ret.empty() ? "*." : " *.") + item;

	return ret;
}

std::string buildOpenFilterString()
{
	const int nbFIF = FreeImage_GetFIFCount();
	std::string filter;
	// Build a string for 'All image files'
/*	filter += "All image files (";
	bool first = true;
	for (int i = 0; i < nbFIF; ++i)
	{
		auto fif = static_cast<FREE_IMAGE_FORMAT>(i);
		if (FreeImage_FIFSupportsReading(fif))
		{
			if (!first)
				filter += " ";
			first = false;
			filter += convertExtensionList(FreeImage_GetFIFExtensionList(fif));
		}
	}
	filter += ");;";
	*/
	// Build a string for 'All files'
	filter += "All Files (*)";

	// Build a string for each format
	for (int i = 0; i < nbFIF; ++i)
	{
		auto fif = static_cast<FREE_IMAGE_FORMAT>(i);
		if (fif == FIF_RAW)
			continue;

		if (FreeImage_FIFSupportsReading(fif)) 
		{
			filter += ";;";
			filter += FreeImage_GetFIFDescription(fif);
			filter += " (" + convertExtensionList(FreeImage_GetFIFExtensionList(fif)) + ")";
		}
	}

	return filter;
}

std::string getOpenFilterString()
{
	static std::string filter;
	if (filter.empty())
		filter = buildOpenFilterString();
	return filter;
}

panda::graphics::Image loadImage(const std::string& fileName)
{
	panda::graphics::Image img;
	if(!fileName.empty())
	{
		auto cpath = fileName.c_str();
		auto fif = FreeImage_GetFileType(cpath, 0);
		if (fif == FIF_UNKNOWN)
			fif = FreeImage_GetFIFFromFilename(cpath);
		if (fif != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fif))
		{
			int flags = 0;
			if (fif == FIF_JPEG)
				flags = JPEG_EXIFROTATE;
			auto dib = FreeImage_Load(fif, cpath, flags);

			if (dib && FreeImage_HasPixels(dib))
			{
				auto type = FreeImage_GetImageType(dib);
				auto bpp = FreeImage_GetBPP(dib);
					
				if (type != FIT_BITMAP || bpp != 32)
				{
					auto converted = FreeImage_ConvertTo32Bits(dib);
					FreeImage_Unload(dib);
					dib = converted; // Can be null
				}

				if (dib)
				{
					auto width = FreeImage_GetWidth(dib);
					auto height = FreeImage_GetHeight(dib);
					auto data = FreeImage_GetBits(dib);

					img = panda::graphics::Image(width, height, data);
					FreeImage_Unload(dib);
				}
			}
		}
	}

	return img;
}

}

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
		m_fileName.setWidgetData(getOpenFilterString());

		addOutput(m_image);
	}

	void update()
	{
		auto img = loadImage(m_fileName.getValue());
		if(img)
			m_image.getAccessor()->setImage(img);
		else
			m_image.getAccessor()->clear();
		cleanDirty();
	}

protected:
	Data<std::string> m_fileName;
	Data<ImageWrapper> m_image;
};

int GeneratorImage_LoadClass = RegisterObject<GeneratorImage_Load>("File/Image/Load image").setDescription("Load an image from the disk");

//****************************************************************************//

class GeneratorImage_LoadMultiple : public PandaObject
{
public:
	PANDA_CLASS(GeneratorImage_LoadMultiple, PandaObject)

	GeneratorImage_LoadMultiple(PandaDocument *doc)
		: PandaObject(doc)
		, m_fileName(initData("fileName", "Path of the image to load"))
		, m_image(initData("image", "The image loaded from disk"))
	{
		addInput(m_fileName);
		m_fileName.setWidget("open file");
		m_fileName.setWidgetData(getOpenFilterString());

		addOutput(m_image);
	}

	void update()
	{
		const auto& paths = m_fileName.getValue();
		auto& images = m_image.getAccessor();

		int nb = paths.size();
		images.resize(nb);

		for (int i = 0; i < nb; ++i)
		{
			auto img = loadImage(paths[i]);
			if (img)
				images[i].setImage(img);
			else
				images[i].clear();
		}
	
		cleanDirty();
	}

protected:
	Data<std::vector<std::string>> m_fileName;
	Data<std::vector<ImageWrapper>> m_image;
};

int GeneratorImage_LoadMultipleClass = RegisterObject<GeneratorImage_LoadMultiple>("File/Image/Load images").setDescription("Load multiple images from the disk");

} // namespace Panda
