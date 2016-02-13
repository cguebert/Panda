#include <panda/graphics/Image.h>

#include <algorithm>
#include <cassert>

namespace panda
{

namespace graphics
{

struct ImageData
{
	ImageData() = default;
	ImageData(Size s);
	ImageData(Size s, const std::vector<unsigned char>& c);
	ImageData(Size s, const unsigned char* c);

	Size size;
	std::vector<unsigned char> contents;
};

ImageData::ImageData(Size s)
	: size(s), contents(s.width() * s.height() * 4, 0) { }

ImageData::ImageData(Size s, const std::vector<unsigned char>& c)
	: size(s), contents(c) { }

ImageData::ImageData(Size s, const unsigned char* c)
	: size(s), contents(c, c + s.width() * s.height() * 4) { }

//****************************************************************************//

Image::Image(Size size)
	: m_data(std::make_shared<ImageData>(size))
{ }

Image::Image(Size size, const std::vector<unsigned char>& contents)
	: m_data(std::make_shared<ImageData>(size, contents))
{ }

Image::Image(Size size, const unsigned char* contents)
	: m_data(std::make_shared<ImageData>(size, contents))
{ }

Size Image::size() const
{
	if (m_data)
		return m_data->size;
	return Size();
}

unsigned char* Image::data()
{
	if (m_data)
		return m_data->contents.data();
	return nullptr;
}

const unsigned char* Image::data() const
{
	if (m_data)
		return m_data->contents.data();
	return nullptr;
}

bool Image::valid(int x, int y) const
{
	if (!m_data)
		return false;
	const auto size = m_data->size;
	return x >= 0 && x < size.width() && y >= 0 && y < size.height();
}

const unsigned char* Image::pixel(int x, int y) const
{
	if (!m_data)
		return nullptr;
	int index = y * m_data->size.width() + x;
	return &m_data->contents.data()[index * 4];
}

unsigned char* Image::pixel(int x, int y)
{
	if (!m_data)
		return nullptr;
	int index = y * m_data->size.width() + x;
	return &m_data->contents.data()[index * 4];
}

void Image::setPixel(int x, int y, unsigned int color)
{
	if (!m_data)
		return;

	int index = y * m_data->size.width() + x;
	std::memcpy(&m_data->contents.data()[index * 4], &color, 4);
}

Image Image::clone() const
{
	if (!m_data)
		return Image();
	return Image(m_data->size, m_data->contents);
}

Image Image::mirrored() const
{
	if (!m_data)
		return Image();

	Image img(m_data->size);

	const auto src = data();
	auto dst = img.data();
	int w = m_data->size.width(), h = m_data->size.height();
	int bytesPerLine = w * 4;
	for (int y = 0; y < h; ++y)
		std::memcpy(dst + y * bytesPerLine, src + (h - y - 1) * bytesPerLine, bytesPerLine);

	return img;
}

void Image::mirror()
{
	if (!m_data)
		return;

	auto d = data();
	int w = m_data->size.width(), h = m_data->size.height();
	int bytesPerLine = w * 4;
	std::vector<unsigned char> buf(bytesPerLine);
	for (int y = 0; y < h / 2; ++y)
	{ 
		// Swap 2 lines
		auto l0 = d + y * bytesPerLine;
		auto l1 = d + (h - y - 1) * bytesPerLine;
		std::memcpy(buf.data(), l0, bytesPerLine);
		std::memcpy(l0, l1, bytesPerLine);
		std::memcpy(l1, buf.data(), bytesPerLine);
	}
}

bool Image::operator==(const Image& img) const
{
	if (empty() || img.empty())
		return false;
	return m_data->size == img.m_data->size && m_data->contents == img.m_data->contents;
}

bool Image::operator!=(const Image& img) const
{
	return !(*this == img);
}

void Image::blitImage(Image& target, int tX, int tY, const Image& source, int sX1, int sY1, int sX2, int sY2)
{
	if (!target || !source)
		return;

	if (tX >= target.width() || tY >= target.height())
		return;

	int sw = source.width(), sh = source.height();
	if (sX1 >= sw || sX2 >= sh)
		return;

	sX2 = std::min(sX2, sw);
	sY2 = std::min(sY2, sh);

	int cw = sX2 - sX1, ch = sY2 - sY1;
	for (int y = 0; y < ch; ++y)
	{
		const auto sD = source.pixel(sX1, sY1 + y);
		auto tD = target.pixel(tX, tY + y);
		std::memcpy(tD, sD, cw * 4);
	}
}

} // namespace graphics

} // namespace panda
