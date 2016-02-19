#include <modules/Images/utils.h>

#include <algorithm>
#include <sstream>

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

std::string buildSaveFilterString()
{
	const int nbFIF = FreeImage_GetFIFCount();
	std::string filter;
	// Build a string for 'All image files'
/*	filter += "All image files (";
	bool first = true;
	for (int i = 0; i < nbFIF; ++i)
	{
		auto fif = static_cast<FREE_IMAGE_FORMAT>(i);
		if (FreeImage_FIFSupportsWriting(fif))
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
		if (FreeImage_FIFSupportsWriting(fif)) 
		{
			filter += ";;";
			filter += FreeImage_GetFIFDescription(fif);
			filter += " (" + convertExtensionList(FreeImage_GetFIFExtensionList(fif)) + ")";
		}
	}

	return filter;
}

}

namespace panda
{

std::string getFilterString(const std::vector<FREE_IMAGE_FORMAT>& formats)
{
	std::string filter;
	bool first = true;
	for (const auto& fif : formats)
	{
		if(!first)
			filter += ";;";
		first = false;
		filter += FreeImage_GetFIFDescription(fif);
		filter += " (" + convertExtensionList(FreeImage_GetFIFExtensionList(fif)) + ")";
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

std::string getSaveFilterString()
{
	static std::string filter;
	if (filter.empty())
		filter = buildSaveFilterString();
	return filter;
}

graphics::Image loadImage(const std::string& fileName)
{
	graphics::Image img;
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
					img = convertToImage(dib);
					FreeImage_Unload(dib);
				}
			}
		}
	}

	return img;
}

void saveImage(const graphics::Image& img, const std::string& fileName)
{
	if (img && !img.size().empty() && !fileName.empty())
	{
		auto cpath = fileName.c_str();
		auto fif = FreeImage_GetFIFFromFilename(cpath);
		if (fif != FIF_UNKNOWN
			&& FreeImage_FIFSupportsWriting(fif)
			&& FreeImage_FIFSupportsExportBPP(fif, 32))
		{
			auto dib = convertFromImage(img);
			FreeImage_Save(fif, dib, cpath, 0);
			FreeImage_Unload(dib);
		}
	}
}

graphics::Image convertToImage(FIBITMAP* dib)
{
	if (!dib)
		return graphics::Image();

	auto width = FreeImage_GetWidth(dib);
	auto height = FreeImage_GetHeight(dib);
	auto data = FreeImage_GetBits(dib);

	return graphics::Image(width, height, data);
}

FIBITMAP* convertFromImage(const graphics::Image& img)
{
	int w = img.width(), h = img.height();
	auto dib = FreeImage_Allocate(w, h, 32);
	auto data = FreeImage_GetBits(dib);

	std::memcpy(data, img.data(), w * h * 4);
	return dib;
}

} // namespace Panda
