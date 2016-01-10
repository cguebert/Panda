#ifndef GRAPHICS_FRAMEBUFFER_H
#define GRAPHICS_FRAMEBUFFER_H

#include <panda/core.h>
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
class FramebufferId;

class PANDA_CORE_API Framebuffer
{
public:
	Framebuffer();
	Framebuffer(int width, int height, int samples = 0);

	explicit operator bool() const; // Returns true if an OpenGL FBO has been created for this object

	unsigned int id() const;
	void bind() const;
	void release() const;
	bool isBound() const;

	int width() const;
	int height() const;
	int samples() const;

	unsigned int texture() const;

	Image toImage() const;

	static void blitFramebuffer(Framebuffer& target, Framebuffer& source);

	static void blitFramebuffer(Framebuffer& target, const types::Rect& targetRect,
		Framebuffer& source, const types::Rect& sourceRect);

	static void blitFramebuffer(unsigned int targetId, const types::Rect& targetRect,
		unsigned int sourceId, const types::Rect& sourceRect);

protected:
	std::shared_ptr<FramebufferId> m_id;
	int m_width = 0, m_height = 0, m_samples = 0;
};

} // namespace graphics

} // namespace panda

#endif // GRAPHICS_FRAMEBUFFER_H
