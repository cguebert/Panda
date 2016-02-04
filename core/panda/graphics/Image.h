#ifndef GRAPHICS_IMAGE_H
#define GRAPHICS_IMAGE_H

#include <panda/graphics/Size.h>
#include <panda/graphics/PointInt.h>

#include <memory>
#include <vector>

namespace panda
{

namespace graphics
{

struct ImageData;

class PANDA_CORE_API Image
{
public:
	Image() = default;
	Image(const Image& img) = default;

	Image(int width, int height); // Create empty image
	Image(Size size);

	Image(int width, int height, const std::vector<unsigned char>& contents);
	Image(Size size, const std::vector<unsigned char>& contents);

	Image(int width, int height, const unsigned char* contents);
	Image(Size size, const unsigned char* contents);

	explicit operator bool() const;
	bool empty() const;
	void clear();

	Size size() const;
	int width() const;
	int height() const;

	unsigned char* data();
	const unsigned char* data() const;

	bool valid(PointInt pt) const;
	bool valid(int x, int y) const;

	unsigned char* pixel(PointInt pt);
	unsigned char* pixel(int x, int y);
	const unsigned char* pixel(PointInt pt) const;
	const unsigned char* pixel(int x, int y) const;

	void setPixel(PointInt pt, unsigned int color);
	void setPixel(int x, int y, unsigned int color);

	Image clone() const;
	Image mirrored() const;

	void mirror();

	static void blitImage(Image& target, PointInt targetPos, const Image& source, PointInt sourcePos, Size sourceSize);
	static void blitImage(Image& target, int tX, int tY, const Image& source, int sX1, int sY1, int sX2, int sY2);

	bool operator==(const Image& img) const;
	bool operator!=(const Image& img) const;

private:
	std::shared_ptr<ImageData> m_data;
};

inline Image::Image(int width, int height)
	: Image(Size(width, height)) {}

inline Image::Image(int width, int height, const std::vector<unsigned char>& contents)
	: Image(Size(width, height), contents) { }

inline Image::Image(int width, int height, const unsigned char* contents)
	: Image(Size(width, height), contents) { }

inline Image::operator bool() const
{ return m_data != nullptr; }

inline bool Image::empty() const
{ return m_data == nullptr; }

inline void Image::clear()
{ m_data.reset(); }

inline int Image::width() const
{ return size().width(); }

inline int Image::height() const
{ return size().height(); }

inline bool Image::valid(PointInt pt) const
{ return valid(pt.x, pt.y); }

inline const unsigned char* Image::pixel(PointInt pt) const
{ return pixel(pt.x, pt.y); }

inline unsigned char* Image::pixel(PointInt pt)
{ return pixel(pt.x, pt.y); }

inline void Image::setPixel(PointInt pt, unsigned int color)
{ setPixel(pt.x, pt.y, color); }

inline void Image::blitImage(Image& target, PointInt targetPos, const Image& source, PointInt sourcePos, Size sourceSize)
{ blitImage(target, targetPos.x, targetPos.y, source, sourcePos.x, sourcePos.y, sourcePos.x + sourceSize.width(), sourcePos.y + sourceSize.height()); }

//****************************************************************************//

inline unsigned char red(const unsigned char* px)
{ return px[0]; }

inline unsigned char green(const unsigned char* px)
{ return px[1]; }

inline unsigned char blue(const unsigned char* px)
{ return px[2]; }

inline unsigned char alpha(const unsigned char* px)
{ return px[3]; }

inline int gray(int r, int g, int b)
{ return (r * 11 + g * 16 + b * 5) / 32; }

inline int gray(const unsigned char* px)
{ return gray(red(px), green(px), blue(px)); }

} // namespace graphics

} // namespace panda

#endif // GRAPHICS_IMAGE_H