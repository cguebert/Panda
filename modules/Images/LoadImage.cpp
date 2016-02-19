#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/graphics/Image.h>

#include <modules/Images/utils.h>

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

		setUpdateOnMainThread(true);
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

		setUpdateOnMainThread(true);
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

//****************************************************************************//

class GeneratorImage_LoadThumbnails : public PandaObject
{
public:
	PANDA_CLASS(GeneratorImage_LoadThumbnails, PandaObject)

	GeneratorImage_LoadThumbnails(PandaDocument *doc)
		: PandaObject(doc)
		, m_fileName(initData("fileName", "Path of the image to load"))
		, m_image(initData("image", "The image loaded from disk"))
	{
		addInput(m_fileName);
		m_fileName.setWidget("open file");
		m_fileName.setWidgetData(getThumbnailFilterString());

		addOutput(m_image);

		setUpdateOnMainThread(true);
	}

	static std::string getThumbnailFilterString()
	{
		static std::string filter;
		if (filter.empty())
		{
			filter = "All Files (*);;";
			filter += getFilterString({ FIF_JPEG, FIF_PSD, FIF_EXR, FIF_TARGA, FIF_TIFF });
		}

		return filter;
	}

	void update()
	{
		const auto& paths = m_fileName.getValue();
		auto& images = m_image.getAccessor();

		int nb = paths.size();
		images.resize(nb);

		for (int i = 0; i < nb; ++i)
		{
			graphics::Image img;
			if(!paths[i].empty())
			{
				auto cpath = paths[i].c_str();
				auto fif = FreeImage_GetFileType(cpath, 0);
				if (fif == FIF_UNKNOWN)
					fif = FreeImage_GetFIFFromFilename(cpath);
				if (fif != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fif))
				{
					auto dib = FreeImage_Load(fif, cpath, FIF_LOAD_NOPIXELS);

					if (dib)
					{
						auto thumbnail = FreeImage_GetThumbnail(dib);
						if (thumbnail && FreeImage_HasPixels(thumbnail))
						{
							auto type = FreeImage_GetImageType(thumbnail);
							auto bpp = FreeImage_GetBPP(thumbnail);

							if (type != FIT_BITMAP || bpp != 32)
							{
								auto converted = FreeImage_ConvertTo32Bits(thumbnail);
								if (converted)
								{
									img = convertToImage(converted);
									FreeImage_Unload(converted);
								}
							}
							else
								img = convertToImage(thumbnail);
						}

						FreeImage_Unload(dib);
					}
				}
			}

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

int GeneratorImage_LoadThumbnailClass = RegisterObject<GeneratorImage_LoadThumbnails>("File/Image/Load thumbnails")
	.setDescription("Load the thumbnail from multiple images");

} // namespace Panda
