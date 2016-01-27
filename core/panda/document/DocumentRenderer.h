#ifndef DOCUMENTRENDERER_H
#define DOCUMENTRENDERER_H

#include <panda/core.h>
#include <panda/graphics/Framebuffer.h>
#include <panda/graphics/Mat4x4.h>
#include <panda/graphics/Model.h>
#include <panda/graphics/ShaderProgram.h>

namespace panda {

class PANDA_CORE_API DocumentRenderer
{
public:
	DocumentRenderer(PandaDocument& document);

	void initializeGL();
	void resizeGL(int w, int h);
	void renderGL();

	bool renderingMainView() const;
	void setRenderingMainView(bool v);

	const graphics::Mat4x4& MVPMatrix() const;

	graphics::Framebuffer& getFBO();

private:
	PandaDocument& m_document;
	bool m_isRenderingMainView = false; // If true, the context of the main render view has already been made current
	graphics::Framebuffer m_renderFBO, m_secondRenderFBO;
	graphics::ShaderProgram m_mergeLayersShader;
	graphics::Model m_rectModel;
	graphics::Mat4x4 m_MVP;
};

//****************************************************************************//

inline graphics::Framebuffer& DocumentRenderer::getFBO()
{ return m_renderFBO; }

inline const graphics::Mat4x4& DocumentRenderer::MVPMatrix() const
{ return m_MVP; }

inline bool DocumentRenderer::renderingMainView() const
{ return m_isRenderingMainView; }

inline void DocumentRenderer::setRenderingMainView(bool v)
{ m_isRenderingMainView = v; }

} // namespace panda

#endif // DOCUMENTRENDERER_H
