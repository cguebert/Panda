#ifndef GRAPHICS_FRAMEBUFFER_H
#define GRAPHICS_FRAMEBUFFER_H

#include <panda/graphics/Image.h>

#include <memory>
#include <vector>

namespace panda
{

namespace types
{
class Rect;
}

namespace graphics
{
// To destroy the framebuffer object only when every copy is destroyed
struct FramebufferData;

class PANDA_CORE_API FramebufferFormat
{
public:
	FramebufferFormat();

	enum class Attachment { NoAttachment, Depth, DepthAndStencil };

	int samples = 0;
	Attachment attachment = Attachment::NoAttachment;
	unsigned int internalFormat; // Default is GL_RGBA8
};

class PANDA_CORE_API Framebuffer
{
public:
	Framebuffer() = default;
	Framebuffer(Size size, FramebufferFormat format = FramebufferFormat());
	Framebuffer(int width, int height, FramebufferFormat format = FramebufferFormat());

	explicit operator bool() const; // Returns true if an OpenGL FBO has been created for this object

	unsigned int id() const;
	void bind() const;
	void release() const;
	bool isBound() const;

	Size size() const;
	int width() const;
	int height() const;

	FramebufferFormat format() const;

	unsigned int texture() const;

	Image toImage() const;

	static void blitFramebuffer(Framebuffer& target, const Framebuffer& source);

	static void blitFramebuffer(Framebuffer& target, const types::Rect& targetRect,
		const Framebuffer& source, const types::Rect& sourceRect);

	static void blitFramebuffer(unsigned int targetId, const types::Rect& targetRect,
		unsigned int sourceId, const types::Rect& sourceRect);

private:
	std::shared_ptr<FramebufferData> m_data;
};

inline Framebuffer::Framebuffer(int width, int height, FramebufferFormat format)
	: Framebuffer(Size(width, height), format) { }

inline Framebuffer::operator bool() const
{ return (id() != 0); }

inline int Framebuffer::width() const
{ return size().width(); }

inline int Framebuffer::height() const
{ return size().height(); }

} // namespace graphics

} // namespace panda

#endif // GRAPHICS_FRAMEBUFFER_H
