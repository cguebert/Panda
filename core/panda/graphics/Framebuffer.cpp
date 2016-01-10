#include <panda/graphics/Framebuffer.h>
#include <panda/types/Rect.h>

#include <GL/glew.h>
#include <algorithm>

namespace panda
{

namespace graphics
{

class FramebufferId
{
public:
	FramebufferId(GLuint fboId, GLuint texId, GLuint rdBufId) 
		: m_fboId(fboId), m_texId(texId), m_rdBufId(rdBufId) {}

	~FramebufferId() 
	{ 
		glDeleteFramebuffers(1, &m_fboId); 
		glDeleteTextures(1, &m_texId); 
		glDeleteRenderbuffers(1, &m_rdBufId);
	}

	GLuint fboId() { return m_fboId; }
	GLuint texId() { return m_texId; }

private:
	GLuint m_fboId, m_texId, m_rdBufId;
};

//****************************************************************************//

Framebuffer::Framebuffer()
{ }

Framebuffer::Framebuffer(int width, int height, int samples)
	: m_width(width), m_height(height)
{
	GLuint fboId = 0, texId = 0, rdrBufId = 0;
	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);

	samples = std::max(0, samples);

	if (!GLEW_EXT_framebuffer_multisample || !GLEW_EXT_framebuffer_blit)
		samples = 0;

	GLint maxSamples = 0;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
	samples = std::min(samples, maxSamples);

	if (!samples)
	{
		// Generate texture
		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Attach it to currently bound framebuffer object
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
	}
	else
	{
		// Use a multisample render buffer
		glGenRenderbuffers(1, &rdrBufId);
		glBindRenderbuffer(GL_RENDERBUFFER, rdrBufId);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rdrBufId);

		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples); // Update the number of samples
	}

	m_samples = samples;
	m_id = std::make_shared<FramebufferId>(fboId, texId, rdrBufId);
}

Framebuffer::operator bool() const
{
	return (id() != 0);
}

unsigned int Framebuffer::id() const
{
	if (m_id)
		return m_id->fboId();
	return 0;
}

void Framebuffer::bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, id()); 
}

void Framebuffer::release() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

bool Framebuffer::isBound() const
{
	GLint fbo;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
	return id() == fbo;
}

int Framebuffer::width() const
{
	return m_width;
}

int Framebuffer::height() const
{
	return m_height;
}

int Framebuffer::samples() const
{
	return m_samples;
}

unsigned int Framebuffer::texture() const
{
	if (m_id)
		return m_id->texId();
	return 0;
}

Image Framebuffer::toImage() const
{
	return Image();
}

void Framebuffer::blitFramebuffer(Framebuffer& target, Framebuffer& source)
{
	types::Rect targetRect(0.f, 0.f, static_cast<PReal>(target.width()), static_cast<PReal>(target.height()));
	types::Rect sourceRect(0.f, 0.f, static_cast<PReal>(source.width()), static_cast<PReal>(source.height()));
	blitFramebuffer(target, targetRect, source, sourceRect);
}

void Framebuffer::blitFramebuffer(Framebuffer& target, const types::Rect& targetRect,
	Framebuffer& source, const types::Rect& sourceRect)
{
	blitFramebuffer(target.id(), targetRect, source.id(), sourceRect);
}

void Framebuffer::blitFramebuffer(unsigned int targetId, const types::Rect& targetRect,
	unsigned int sourceId, const types::Rect& sourceRect)
{
	if (!GLEW_EXT_framebuffer_blit)
		return;

	const int sx0 = static_cast<int>(sourceRect.left());
	const int sx1 = static_cast<int>(sourceRect.right());
	const int sy0 = static_cast<int>(sourceRect.top());
	const int sy1 = static_cast<int>(sourceRect.bottom());

	const int tx0 = static_cast<int>(targetRect.left());
	const int tx1 = static_cast<int>(targetRect.right());
	const int ty0 = static_cast<int>(targetRect.top());
	const int ty1 = static_cast<int>(targetRect.bottom());

	GLint prev = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetId);
	glBlitFramebuffer(sx0, sy0, sx1, sy1, tx0, ty0, tx1, ty1, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, prev);
}

} // namespace graphics

} // namespace panda
