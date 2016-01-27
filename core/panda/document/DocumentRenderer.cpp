#include <GL/glew.h>

#include <panda/PandaDocument.h>
#include <panda/document/DocumentRenderer.h>
#include <panda/graphics/Framebuffer.h>
#include <panda/graphics/Model.h>
#include <panda/object/Layer.h>
#include <panda/helper/system/FileRepository.h>

#ifdef PANDA_LOG_EVENTS
#include <panda/UpdateLogger.h>
#endif

namespace panda 
{

DocumentRenderer::DocumentRenderer(PandaDocument& document)
	: m_document(document)
{
}

void DocumentRenderer::initializeGL()
{
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	  fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

// Shader used to blend one layer on top of the previous one
	m_mergeLayersShader.addShaderFromMemory(graphics::ShaderType::Vertex,
		helper::system::DataRepository.loadFile("shaders/mergeLayers.v.glsl"));
	m_mergeLayersShader.addShaderFromMemory(graphics::ShaderType::Fragment,
		helper::system::DataRepository.loadFile("shaders/mergeLayers.f.glsl"));
	m_mergeLayersShader.link();
	m_mergeLayersShader.bind();

	m_mergeLayersShader.setUniformValue("texS", 0);
	m_mergeLayersShader.setUniformValue("texD", 1);

	m_mergeLayersShader.release();

// Model to render the rectangle for the layers
	std::vector<GLfloat> verts = { 0, 0, 1, 0, 0, 1, 1, 1 };
	std::vector<GLfloat> texCoords = { 0, 1, 1, 1, 0, 0, 1, 0};
	m_rectModel.setVertices(verts);
	m_rectModel.setTexCoords(texCoords);
	m_rectModel.create();
}

void DocumentRenderer::resizeGL(int w, int h)
{
	auto renderSize = graphics::Size(w, h);
	if (!m_renderFBO || m_renderFBO.size() != renderSize)
	{
		m_renderFBO = graphics::Framebuffer(renderSize);
		m_secondRenderFBO = graphics::Framebuffer(renderSize);

		GLfloat fw = static_cast<float>(w), fh = static_cast<float>(h);
		std::vector<GLfloat> verts = { 0, 0, fw, 0, 0, fh, fw, fh };
		m_rectModel.setVertices(verts);

		glViewport(0, 0, w, h);
		m_MVP.ortho(0, fw, fh, 0, -10, 10);
	}
}

void DocumentRenderer::renderGL()
{
	m_document.getDefaultLayer()->updateIfDirty();

	for(auto& obj : m_document.getObjects())
	{
		if(dynamic_cast<BaseLayer*>(obj.get()))
			obj->updateIfDirty();
	}

#ifdef PANDA_LOG_EVENTS
	{
		helper::ScopedEvent log1("prepareRender");
#endif

	types::Color col = m_document.getBackgroundColor();
	glClearColor(col.r, col.g, col.b, col.a);

	auto renderSize = m_document.getRenderSize();
	glViewport(0, 0, renderSize.width(), renderSize.height());

	m_secondRenderFBO.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_secondRenderFBO.release();

	m_renderFBO.bind();

#ifdef PANDA_LOG_EVENTS
	}
#endif

	m_mergeLayersShader.bind();

	m_mergeLayersShader.setUniformValueMat4("MVP", m_MVP.data());

#ifdef PANDA_LOG_EVENTS
	{
	helper::ScopedEvent log("merge default Layer");
#endif

	m_mergeLayersShader.setUniformValue("opacity", 1.0f);
	m_mergeLayersShader.setUniformValue("mode", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_document.getDefaultLayer()->getTextureId());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_secondRenderFBO.texture());
	glActiveTexture(GL_TEXTURE0);

	m_rectModel.render();

	m_renderFBO.release();

#ifdef PANDA_LOG_EVENTS
	}
#endif

	bool inverse = false;
	for(auto& obj : m_document.getObjects())
	{
		BaseLayer* layer = dynamic_cast<BaseLayer*>(obj.get());
		if(layer)
		{
			float opacity = (float)layer->getOpacity();
			if(!opacity)
				continue;

#ifdef PANDA_LOG_EVENTS
			helper::ScopedEvent log2("merge Layer");
#endif

			m_mergeLayersShader.setUniformValue("opacity", opacity);
			m_mergeLayersShader.setUniformValue("mode", layer->getCompositionMode());

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, layer->getTextureId());

			inverse = !inverse;
			if(inverse)
			{
				m_secondRenderFBO.bind();
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, m_renderFBO.texture());
			}
			else
			{
				m_renderFBO.bind();
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, m_secondRenderFBO.texture());
			}

			m_rectModel.render();
			glActiveTexture(GL_TEXTURE0);

			if(inverse)
				m_secondRenderFBO.release();
			else
				m_renderFBO.release();
		}
	}

	m_mergeLayersShader.release();

	if(inverse)
	{
#ifdef PANDA_LOG_EVENTS
		helper::ScopedEvent log3("blit FBO");
#endif
		graphics::Framebuffer::blitFramebuffer(m_renderFBO, m_secondRenderFBO);
	}
}

} // namespace panda

