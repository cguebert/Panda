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

namespace
{

void createAttachments(FramebufferData& data)
{
	GLuint depthBuffer = 0, stencilBuffer = 0;
	auto attachment = data.format.attachment;
	auto samples = data.format.samples;
	int w = data.size.width(), h = data.size.height();

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
		data.format.attachment = FramebufferFormat::Attachment::DepthAndStencil;
	else if (depthBuffer)
		data.format.attachment = FramebufferFormat::Attachment::Depth;
	else
		data.format.attachment = FramebufferFormat::Attachment::NoAttachment;

	data.depthBuffer = depthBuffer;
	data.stencilBuffer = stencilBuffer;
}

void createFrameBuffer(FramebufferData& data)
{
	int w = data.size.width(), h = data.size.height();
	glGenFramebuffers(1, &data.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, data.fbo);

	data.format.samples = std::max(0, data.format.samples);
	int samples = data.format.samples;

	if (!GLEW_EXT_framebuffer_multisample || !GLEW_EXT_framebuffer_blit)
		samples = 0;

	GLint maxSamples = 0;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
	samples = std::min(samples, maxSamples);

	if (!samples)
	{
		GLuint pixelType = GL_UNSIGNED_BYTE;
		if (data.format.internalFormat == GL_RGB10_A2 || data.format.internalFormat == GL_RGB10)
			pixelType = GL_UNSIGNED_INT_2_10_10_10_REV;

		// Generate texture
		glGenTextures(1, &data.texture);
		glBindTexture(GL_TEXTURE_2D, data.texture);
		glTexImage2D(GL_TEXTURE_2D, 0, data.format.internalFormat, w, h, 0, GL_RGBA, pixelType, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Attach it to currently bound framebuffer object
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data.texture, 0);
	}
	else
	{
		// Use a multisample render buffer
		glGenRenderbuffers(1, &data.colorBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, data.colorBuffer);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, data.format.internalFormat, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, data.colorBuffer);

		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples); // Update the number of samples
	}

	createAttachments(data);

	data.format.samples = samples; // Set the real number of samples for this FBO
}

}

Framebuffer::Framebuffer(Size size, FramebufferFormat format)
	: m_data(std::make_shared<FramebufferData>())
{
	m_data->size = size;
	m_data->format = format;

	createFrameBuffer(*m_data);
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
	// Cannot read from a multisample FBO
	if (format().samples)
	{
		Framebuffer tmp(size());
		blitFramebuffer(tmp, *this);
		return tmp.toImage();
	}

	GLint prev = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev);

	if (prev != id())
		glBindFramebuffer(GL_FRAMEBUFFER, id());

	const auto size = m_data->size;
	Image img(size);
	glReadPixels(0, 0, size.width(), size.height(), GL_BGRA, GL_UNSIGNED_BYTE, img.data());

	if (prev != id())
		glBindFramebuffer(GL_FRAMEBUFFER, prev);

	return img;
}

void Framebuffer::toImage(Image& img) const
{
	// Cannot read from a multisample FBO
	if (format().samples)
	{
		Framebuffer tmp(size());
		blitFramebuffer(tmp, *this);
		tmp.toImage(img);
	}

	GLint prev = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev);

	if (prev != id())
		glBindFramebuffer(GL_FRAMEBUFFER, id());

	const auto size = m_data->size;
	if (img.size() != size)
		img = Image(size);
	glReadPixels(0, 0, size.width(), size.height(), GL_RGBA, GL_UNSIGNED_BYTE, img.data());

	if (prev != id())
		glBindFramebuffer(GL_FRAMEBUFFER, prev);
}

void Framebuffer::blitFramebuffer(Framebuffer& target, const Framebuffer& source)
{
	RectInt targetRect(0, 0, target.width(), target.height());
	RectInt sourceRect(0, 0, source.width(), source.height());
	blitFramebuffer(target, targetRect, source, sourceRect);
}

void Framebuffer::blitFramebuffer(Framebuffer& target, const RectInt& targetRect,
	const Framebuffer& source, const RectInt& sourceRect)
{
	blitFramebuffer(target.id(), targetRect, source.id(), sourceRect);
}

void Framebuffer::blitFramebuffer(unsigned int targetId, const RectInt& tr,
	unsigned int sourceId, const RectInt& sr)
{
	if (!GLEW_EXT_framebuffer_blit)
		return;

	GLint prev = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetId);
	glBlitFramebuffer(sr.x1, sr.y1, sr.x2, sr.y2, tr.x1, tr.y1, tr.x2, tr.y2, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, prev);
}

} // namespace graphics

} // namespace panda
