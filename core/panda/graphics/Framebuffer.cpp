#include <panda/graphics/Framebuffer.h>
#include <panda/types/Rect.h>

#include <GL/glew.h>
#include <algorithm>

namespace panda
{

namespace graphics
{

struct FramebufferData
{
public:
	~FramebufferData() 
	{ 
		glDeleteFramebuffers(1, &fbo); 
		glDeleteTextures(1, &texture); 
		glDeleteRenderbuffers(1, &colorBuffer);
		glDeleteRenderbuffers(1, &depthBuffer);
		glDeleteRenderbuffers(1, &stencilBuffer);
	}

	Size size;
	FramebufferFormat format;
	GLuint fbo = 0, texture = 0, 
		colorBuffer = 0, depthBuffer = 0, stencilBuffer = 0;
};

//****************************************************************************//

FramebufferFormat::FramebufferFormat()
	: internalFormat(GL_RGBA8)
{
}

//****************************************************************************//

Framebuffer::Framebuffer(Size size, FramebufferFormat format)
	: m_data(std::make_shared<FramebufferData>())
{
	m_data->size = size;
	m_data->format = format;

	int w = size.width(), h = size.height();
	glGenFramebuffers(1, &m_data->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_data->fbo);

	format.samples = std::max(0, format.samples);
	int samples = format.samples;

	if (!GLEW_EXT_framebuffer_multisample || !GLEW_EXT_framebuffer_blit)
		samples = 0;

	GLint maxSamples = 0;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
	samples = std::min(samples, maxSamples);

	if (!samples)
	{
		// Generate texture
		glGenTextures(1, &m_data->texture);
		glBindTexture(GL_TEXTURE_2D, m_data->texture);
		glTexImage2D(GL_TEXTURE_2D, 0, format.internalFormat, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Attach it to currently bound framebuffer object
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_data->texture, 0);
	}
	else
	{
		// Use a multisample render buffer
		glGenRenderbuffers(1, &m_data->colorBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_data->colorBuffer);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, format.internalFormat, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_data->colorBuffer);

		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples); // Update the number of samples
	}

	createAttachments();

	m_data->format.samples = samples; // Set the real number of samples for this FBO
}

void Framebuffer::createAttachments()
{
	GLuint depthBuffer = 0, stencilBuffer = 0;
	auto attachment = m_data->format.attachment;
	auto samples = m_data->format.samples;
	int w = m_data->size.width(), h = m_data->size.height();

	if (attachment == FramebufferFormat::Attachment::DepthAndStencil && GLEW_EXT_packed_depth_stencil)
	{
		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);

		if (samples && GLEW_EXT_framebuffer_multisample)
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, w, h);
		else
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);

		stencilBuffer = depthBuffer;
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilBuffer);
	}

	if (!depthBuffer && (attachment == FramebufferFormat::Attachment::Depth 
		|| attachment == FramebufferFormat::Attachment::DepthAndStencil))
	{
		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		if (samples && GLEW_EXT_framebuffer_multisample)
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT, w, h);
		else
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	}

	if (!stencilBuffer && attachment == FramebufferFormat::Attachment::DepthAndStencil)
	{
		glGenRenderbuffers(1, &stencilBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, stencilBuffer);
		if (samples && GLEW_EXT_framebuffer_multisample)
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_STENCIL_INDEX, w, h);
		else
			glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilBuffer);
	}

	if (depthBuffer && stencilBuffer)
		m_data->format.attachment = FramebufferFormat::Attachment::DepthAndStencil;
	else if (depthBuffer)
		m_data->format.attachment = FramebufferFormat::Attachment::Depth;
	else
		m_data->format.attachment = FramebufferFormat::Attachment::NoAttachment;

	m_data->depthBuffer = depthBuffer;
	m_data->stencilBuffer = stencilBuffer;
}

unsigned int Framebuffer::id() const
{
	if (m_data)
		return m_data->fbo;
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

Size Framebuffer::size() const
{
	if (m_data)
		return m_data->size;
	return Size();
}

FramebufferFormat Framebuffer::format() const
{
	if (m_data)
		return m_data->format;
	return FramebufferFormat();
}

unsigned int Framebuffer::texture() const
{
	if (m_data)
		return m_data->texture;
	return 0;
}

Image Framebuffer::toImage() const
{
	return Image();
}

void Framebuffer::blitFramebuffer(Framebuffer& target, const Framebuffer& source)
{
	types::Rect targetRect(0.f, 0.f, static_cast<PReal>(target.width()), static_cast<PReal>(target.height()));
	types::Rect sourceRect(0.f, 0.f, static_cast<PReal>(source.width()), static_cast<PReal>(source.height()));
	blitFramebuffer(target, targetRect, source, sourceRect);
}

void Framebuffer::blitFramebuffer(Framebuffer& target, const types::Rect& targetRect,
	const Framebuffer& source, const types::Rect& sourceRect)
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
