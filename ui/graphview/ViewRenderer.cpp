#include <ui/graphview/ViewRenderer.h>
#include <ui/graphview/graphics/DrawList.h>

#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

#include <cassert>

using panda::types::Point;

static ViewRenderer* g_viewRenderer = nullptr;

namespace
{
	class OpenGLStateSaver
	{
	public:
		OpenGLStateSaver(QOpenGLFunctions* functions)
			: m_functions(functions)
		{
			glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
			glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
			glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
			glGetIntegerv(GL_BLEND_SRC, &last_blend_src);
			glGetIntegerv(GL_BLEND_DST, &last_blend_dst);
			glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
			glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
			glGetIntegerv(GL_VIEWPORT, last_viewport);
			last_enable_blend = glIsEnabled(GL_BLEND);
			last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
			last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
			last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
		}

		~OpenGLStateSaver()
		{
			m_functions->glUseProgram(last_program);
			glBindTexture(GL_TEXTURE_2D, last_texture);
			m_functions->glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
			m_functions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
			m_functions->glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
			glBlendFunc(last_blend_src, last_blend_dst);
			if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
			if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
			if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
			if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
			glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
		}

	private:
		QOpenGLFunctions* m_functions;
		GLint last_program, last_texture, last_array_buffer, last_element_array_buffer, last_blend_src,
			last_blend_dst, last_blend_equation_rgb, last_blend_equation_alpha, last_viewport[4];
		GLboolean last_enable_blend, last_enable_cull_face, last_enable_depth_test, last_enable_scissor_test;
	};
}

ViewRenderer::ViewRenderer()
{
	g_viewRenderer = this;
	for(int i = 0; i < 4; ++i)
		m_viewBounds[i] = 0;
}

ViewRenderer::~ViewRenderer()
{
	g_viewRenderer = nullptr;
}

void ViewRenderer::initialize()
{
	QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();

	// For now, empty texture (white)
	std::vector<unsigned char> pixels(4, 0xFF);
	QImage img(pixels.data(), 1, 1, QImage::Format::Format_ARGB32);
	m_fontTexture = std::make_unique<QOpenGLTexture>(img);

	const GLchar* vertex_shader =
		"#version 330\n"
		"uniform mat4 ProjMtx;\n"
		"layout (location = 0) in vec2 Position;\n"
		"layout (location = 1) in vec2 UV;\n"
		"layout (location = 2) in vec4 Color;\n"
		"out vec2 Frag_UV;\n"
		"out vec4 Frag_Color;\n"
		"void main()\n"
		"{\n"
		"	Frag_UV = UV;\n"
		"	Frag_Color = Color;\n"
		"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
		"}\n";

	const GLchar* fragment_shader =
		"#version 330\n"
		"uniform sampler2D Texture;\n"
		"in vec2 Frag_UV;\n"
		"in vec4 Frag_Color;\n"
		"out vec4 Out_Color;\n"
		"void main()\n"
		"{\n"
		"	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
		"}\n";

	m_shader = std::make_unique<QOpenGLShaderProgram>();
	m_shader->addShaderFromSourceCode(QOpenGLShader::ShaderTypeBit::Vertex, vertex_shader);
	m_shader->addShaderFromSourceCode(QOpenGLShader::ShaderTypeBit::Fragment, fragment_shader);
	m_shader->link();

	m_locationTex = m_shader->uniformLocation("Texture");
	m_locationProjMtx = m_shader->uniformLocation("ProjMtx");

	m_VAO = std::make_unique<QOpenGLVertexArrayObject>();
	m_VAO->create();
	m_VAO->bind();

	m_VBO = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
	m_VBO->create();
	m_VBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
	m_VBO->bind();
	
	f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(DrawVert), (GLvoid*)offsetof(DrawVert, pos));
	f->glEnableVertexAttribArray(0);

	f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(DrawVert), (GLvoid*)offsetof(DrawVert, uv));
	f->glEnableVertexAttribArray(1);

	f->glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(DrawVert), (GLvoid*)offsetof(DrawVert, col));
	f->glEnableVertexAttribArray(2);
	
	m_VBO->release();
	
	m_EBO = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::IndexBuffer);
	m_EBO->create();
	m_EBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
	
	m_VAO->release();
}

void ViewRenderer::resize(int w, int h)
{
	m_width = w;
	m_height = h;
}

void ViewRenderer::setView(float left, float top, float right, float bottom)
{
	m_viewBounds[0] = left;
	m_viewBounds[1] = top;
	m_viewBounds[2] = right;
	m_viewBounds[3] = bottom;
}

void ViewRenderer::newFrame()
{
	m_drawLists.clear();
}

void ViewRenderer::render()
{
	QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
	OpenGLStateSaver state(f);

	f->glEnable(GL_BLEND);
	f->glBlendEquation(GL_FUNC_ADD);
	f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	f->glDisable(GL_CULL_FACE);
	f->glDisable(GL_DEPTH_TEST);
	f->glActiveTexture(GL_TEXTURE0);

	glViewport(0, 0, m_width, m_height);
	QMatrix4x4 ortho;
	ortho.ortho(m_viewBounds[0], m_viewBounds[2], m_viewBounds[3], m_viewBounds[1], -1, 1);

	m_shader->bind();
	m_shader->setUniformValue(m_locationTex, 0);
	m_shader->setUniformValue(m_locationProjMtx, ortho);

	m_VAO->bind();

	int vboSize = 0, eboSize = 0;
	for (const DrawList* cmd_list : m_drawLists)
	{
		if (cmd_list->vtxBuffer().empty() || cmd_list->idxBuffer().empty())
			continue;

		const DrawIdx* idx_buffer_offset = nullptr;

		m_VBO->bind();
		int vtxSize = cmd_list->vtxBuffer().size() * sizeof(DrawVert);
		if (vtxSize > vboSize)
		{
			m_VBO->allocate(vtxSize);
			vboSize = vtxSize;
		}
		m_VBO->write(0, &cmd_list->vtxBuffer().front(), vtxSize);

		m_EBO->bind();
		int idxSize = cmd_list->idxBuffer().size() * sizeof(DrawIdx);
		if (idxSize > eboSize)
		{
			m_EBO->allocate(idxSize);
			eboSize = idxSize;
		}
		m_EBO->write(0, &cmd_list->idxBuffer().front(), idxSize);

		for (const DrawCmd& pcmd : cmd_list->cmdBuffer())
		{
			glBindTexture(GL_TEXTURE_2D, pcmd.textureId);
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(pcmd.elemCount), GL_UNSIGNED_INT, idx_buffer_offset);
			idx_buffer_offset += pcmd.elemCount;
		}
	}

	m_shader->release();
	m_VAO->release();
}

unsigned int ViewRenderer::defaultTextureId()
{
	if (g_viewRenderer && g_viewRenderer->m_fontTexture)
		return g_viewRenderer->m_fontTexture->textureId();
	return 0;
}
