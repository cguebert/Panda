#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

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
						if(thumbnail)
							img = convertTo32bitsImage(thumbnail);
						FreeImage_Unload(dib); // thumbnail is a child of dib, and freed by it
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
	.setDescription("Load the thumbnail from multiple images (null output if it does not exist)");

//****************************************************************************//

class GeneratorImage_LoadOrMakeThumbnails : public PandaObject
{
public:
	PANDA_CLASS(GeneratorImage_LoadOrMakeThumbnails, PandaObject)

	GeneratorImage_LoadOrMakeThumbnails(PandaDocument *doc)
		: PandaObject(doc)
		, m_fileName(initData("fileName", "Path of the image to load"))
		, m_image(initData("image", "The image loaded from disk"))
		, m_size(initData(100, "size", "Maximum size of the created thumbnails"))
		, m_loadExisting(initData(true, "loadExisting", "If a thumbnail is bundled in the file, use it"))
	{
		addInput(m_fileName);
		addInput(m_size);
		addInput(m_loadExisting);

		m_loadExisting.setWidget("checkbox");
		m_fileName.setWidget("open file");
		m_fileName.setWidgetData(getOpenFilterString());

		addOutput(m_image);

		setUpdateOnMainThread(true);
	}

	void update()
	{
		const auto& paths = m_fileName.getValue();
		auto& images = m_image.getAccessor();
		const int size = m_size.getValue();
		bool useExisting = (m_loadExisting.getValue() != 0);

		int nb = paths.size();
		images.resize(nb);

		for (int i = 0; i < nb; ++i)
		{
			const auto& path = paths[i];
			auto& output = images[i];
			if(path.empty())
			{
				output.clear();
				continue;
			}

			auto cpath = path.c_str();
			auto fif = FreeImage_GetFileType(cpath, 0);
			if (fif == FIF_UNKNOWN)
				fif = FreeImage_GetFIFFromFilename(cpath);
			if (fif == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(fif))
			{
				output.clear();
				continue;
			}

			// First try loading the thumbnail
			if (useExisting)
			{
				if (FreeImage_FIFSupportsNoPixels(fif))
				{
					auto dib = FreeImage_Load(fif, cpath, FIF_LOAD_NOPIXELS);
					if (dib)
					{
						auto thumbnail = FreeImage_GetThumbnail(dib);
						if (thumbnail)
						{
							output.setImage(convertTo32bitsImage(thumbnail));
							FreeImage_Unload(dib); // thumbnail is a child of dib, and freed by it
							continue;
						}
					}
				}
			}

			FIBITMAP* dib = nullptr;
			// Make the thumbnail
			if (fif == FIF_JPEG)
			{
				FITAG* tag = nullptr;
				// Downsampling while loading
				dib = FreeImage_Load(fif, cpath, size << 16);
			}
			else
			{
				int flag = 0;
				if (fif == FIF_RAW)
					flag = RAW_PREVIEW;
				dib = FreeImage_Load(fif, cpath, flag);
			}

			if (dib)
			{
				auto thumbnail = FreeImage_MakeThumbnail(dib, size, TRUE);
				output.setImage(convertTo32bitsImage(thumbnail));
				FreeImage_Unload(dib); // thumbnail is a child of dib, and freed by it
				continue;
			}

			output.clear();
		}
	
		cleanDirty();
	}

protected:
	Data<std::vector<std::string>> m_fileName;
	Data<std::vector<ImageWrapper>> m_image;
	Data<int> m_size, m_loadExisting;
};

int GeneratorImage_LoadOrMakeThumbnailsClass = RegisterObject<GeneratorImage_LoadOrMakeThumbnails>("File/Image/Load or make thumbnails")
	.setDescription("Load of make the thumbnail for multiple images");

} // namespace Panda
