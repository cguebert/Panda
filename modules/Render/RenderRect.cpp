#include <GL/glew.h>

#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>
#include <panda/graphics/Buffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/graphics/VertexArrayObject.h>

namespace panda {

using types::Color;
using types::ImageWrapper;
using types::Rect;
using types::Shader;

class RenderRect : public Renderer
{
public:
	PANDA_CLASS(RenderRect, Renderer)

	RenderRect(PandaDocument* parent)
		: Renderer(parent)
		, m_rect(initData("rectangle", "Position and size of the rectangle"))
		, m_lineWidth(initData("lineWidth", "Width of the line"))
		, m_color(initData("color", "Color of the rectangle"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_rect);
		addInput(m_lineWidth);
		addInput(m_color);
		addInput(m_shader);

		m_rect.getAccessor().push_back(Rect(100, 100, 150, 150));
		m_color.getAccessor().push_back(Color::black());
		m_lineWidth.getAccessor().push_back(0.0);

		m_shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = m_shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/PT_attColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/PT_attColor_noTex.f.glsl");
	}

	void initGL()
	{
		m_VAO.create();
		m_VAO.bind();

		m_verticesVBO.create();
		m_verticesVBO.bind();
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(0);

		m_colorsVBO.create();
		m_colorsVBO.bind();
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(1);

		m_VAO.release();
	}

	void update()
	{
		const std::vector<Rect>& listRect = m_rect.getValue();
		const std::vector<Color>& listColor = m_color.getValue();
		const std::vector<PReal>& listWidth = m_lineWidth.getValue();

		m_verticesBuffer.clear();
		m_colorBuffer.clear();

		m_firstBuffer.clear();
		m_countBuffer.clear();

		if (!listRect.empty() && !listColor.empty() && !listWidth.empty())
		{
			int nbRect = listRect.size();
			int nbColor = listColor.size();
			int nbWidth = listWidth.size();

			if(nbWidth < nbRect) nbWidth = 1;

			m_firstBuffer.resize(nbRect);
			for (int i = 0; i < nbRect; ++i)
				m_firstBuffer[i] = i * 4;

			m_countBuffer.assign(nbRect, 10);

			m_verticesBuffer.reserve(nbRect * 10);
			for(int i = 0; i < nbRect; ++i)
			{
				const auto& rect = listRect[i];
				PReal maxWidth = std::min(rect.width(), rect.height()) - 0.5f;
				PReal width = helper::bound(1.f, listWidth[i % nbWidth], maxWidth);
				PReal w = width / 2;

				m_verticesBuffer.emplace_back(rect.right() + w, rect.top() - w);
				m_verticesBuffer.emplace_back(rect.right() - w, rect.top() + w);
				m_verticesBuffer.emplace_back(rect.left() - w,  rect.top() - w);
				m_verticesBuffer.emplace_back(rect.left() + w,  rect.top() + w);
				m_verticesBuffer.emplace_back(rect.left() - w,  rect.bottom() + w);
				m_verticesBuffer.emplace_back(rect.left() + w,  rect.bottom() - w);
				m_verticesBuffer.emplace_back(rect.right() + w, rect.bottom() + w);
				m_verticesBuffer.emplace_back(rect.right() - w, rect.bottom() - w);
				m_verticesBuffer.emplace_back(rect.right() + w, rect.top() - w);
				m_verticesBuffer.emplace_back(rect.right() - w, rect.top() + w);
			}

			m_colorBuffer.reserve(nbRect * 10);
			if (nbColor < nbRect)
			{
				m_colorBuffer.assign(nbRect * 8, listColor[0]);
			}
			else
			{
				for (const auto& color : listColor)
				{
					for (int i = 0; i < 10; ++i)
						m_colorBuffer.push_back(color);
				}
			}
		}
	}

	void render()
	{
		const std::vector<PReal>& listWidth = m_lineWidth.getValue();
		int nbWidth = listWidth.size();

		if(!m_verticesBuffer.empty() && !m_colorBuffer.empty() && nbWidth)
		{
			if(!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_verticesVBO.bind();
			m_verticesVBO.write(m_verticesBuffer);

			m_colorsVBO.bind();
			m_colorsVBO.write(m_colorBuffer);
			
			m_VAO.bind();

			m_shaderProgram.bind();
			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			glMultiDrawArrays(GL_TRIANGLE_STRIP, m_firstBuffer.data(), m_countBuffer.data(), m_countBuffer.size());

			m_shaderProgram.release();
			m_VAO.release();
		}
	}

protected:
	Data< std::vector<Rect> > m_rect;
	Data< std::vector<PReal> > m_lineWidth;
	Data< std::vector<Color> > m_color;
	Data< Shader > m_shader;

	std::vector<types::Point> m_verticesBuffer;
	std::vector<types::Color> m_colorBuffer;
	std::vector<GLint> m_firstBuffer;
	std::vector<GLsizei> m_countBuffer;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_colorsVBO;
};

int RenderRectClass = RegisterObject<RenderRect>("Render/Line/Rectangle")
		.setName("Rectangle").setDescription("Draw a rectangle");

//****************************************************************************//

class RenderFilledRect : public Renderer
{
public:
	PANDA_CLASS(RenderFilledRect, Renderer)

	RenderFilledRect(PandaDocument* parent)
		: Renderer(parent)
		, m_rect(initData("rectangle", "Position and size of the rectangle"))
		, m_color(initData("color", "Color of the rectangle"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_rect);
		addInput(m_color);
		addInput(m_shader);

		m_rect.getAccessor().push_back(Rect(100, 100, 150, 150));
		m_color.getAccessor().push_back(Color::black());

		m_shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = m_shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/PT_uniColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/PT_uniColor_noTex.f.glsl");
	}

	void initGL()
	{
		m_VAO.create();
		m_VAO.bind();

		m_verticesVBO.create();
		m_verticesVBO.bind();
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(0);

		m_texCoordsVBO.create();
		m_texCoordsVBO.bind();
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(1);

		m_VAO.release();
	}

	void update()
	{
		const std::vector<Rect>& listRect = m_rect.getValue();
		const std::vector<Color>& listColor = m_color.getValue();

		m_verticesBuffer.clear();
		m_texCoordsBuffer.clear();
		if (!listRect.empty() && !listColor.empty())
		{
			int nbRect = listRect.size();
			m_verticesBuffer.reserve(nbRect * 4);
			for (const auto& rect : listRect)
			{
				m_verticesBuffer.emplace_back(rect.right(), rect.top());
				m_verticesBuffer.emplace_back(rect.left(),  rect.top());
				m_verticesBuffer.emplace_back(rect.right(), rect.bottom());
				m_verticesBuffer.emplace_back(rect.left(),  rect.bottom());
			}

			m_texCoordsBuffer.reserve(nbRect * 4);
			for (const auto& rect : listRect)
			{
				m_texCoordsBuffer.emplace_back(1.f, 1.f);
				m_texCoordsBuffer.emplace_back(0.f, 1.f);
				m_texCoordsBuffer.emplace_back(1.f, 0.f);
				m_texCoordsBuffer.emplace_back(0.f, 0.f);
			}
		}
	}

	void render()
	{
		const std::vector<Color>& listColor = m_color.getValue();
		int nbColor = listColor.size();

		if(!m_verticesBuffer.empty() && !m_texCoordsBuffer.empty() && nbColor)
		{
			if(!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			glEnable(GL_LINE_SMOOTH);

			m_verticesVBO.bind();
			m_verticesVBO.write(m_verticesBuffer);

			m_texCoordsVBO.bind();
			m_texCoordsVBO.write(m_texCoordsBuffer);
			
			m_VAO.bind();

			m_shaderProgram.bind();
			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			int colorLocation = m_shaderProgram.uniformLocation("color");
			int nbRect = m_verticesBuffer.size() / 4;
			for(int i=0; i < nbRect; ++i)
			{
				m_shaderProgram.setUniformValueArray(colorLocation, listColor[i % nbColor].data(), 1, 4);

				glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
			}

			m_shaderProgram.release();

			m_VAO.release();

			glDisable(GL_LINE_SMOOTH);
		}
	}

protected:
	Data< std::vector<Rect> > m_rect;
	Data< std::vector<Color> > m_color;
	Data< Shader > m_shader;

	std::vector<types::Point> m_verticesBuffer, m_texCoordsBuffer;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_texCoordsVBO;
};

int RenderFilledRectClass = RegisterObject<RenderFilledRect>("Render/Filled/Rectangle")
		.setName("Filled rectangle").setDescription("Draw a filled rectangle");

//****************************************************************************//

class RenderTexturedRect : public Renderer
{
public:
	PANDA_CLASS(RenderTexturedRect, Renderer)

	RenderTexturedRect(PandaDocument* parent)
		: Renderer(parent)
		, m_rect(initData("rectangle", "Position and size of the rectangle"))
		, m_texture(initData("texture", "Texture to apply to the rectangle"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_rect);
		addInput(m_texture);
		addInput(m_shader);

		m_rect.getAccessor().push_back(Rect(100, 100, 150, 150));

		m_shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = m_shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/PT_noColor_Tex.f.glsl");
	}

	void initGL()
	{
		m_VAO.create();
		m_VAO.bind();

		m_verticesVBO.create();
		m_verticesVBO.bind();
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(0);

		m_texCoordsVBO.create();
		m_texCoordsVBO.bind();
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(1);

		m_VAO.release();
	}

	void update()
	{
		const std::vector<Rect>& listRect = m_rect.getValue();

		m_verticesBuffer.clear();
		m_texCoordsBuffer.clear();

		m_firstBuffer.clear();
		m_countBuffer.clear();

		if (!listRect.empty())
		{
			int nbRect = listRect.size();

			m_firstBuffer.resize(nbRect);
			for (int i = 0; i < nbRect; ++i)
				m_firstBuffer[i] = i * 4;

			m_countBuffer.assign(nbRect, 4);

			m_verticesBuffer.reserve(nbRect * 4);
			for (const auto& rect : listRect)
			{
				m_verticesBuffer.emplace_back(rect.right(), rect.top());
				m_verticesBuffer.emplace_back(rect.left(),  rect.top());
				m_verticesBuffer.emplace_back(rect.right(), rect.bottom());
				m_verticesBuffer.emplace_back(rect.left(),  rect.bottom());
			}

			m_texCoordsBuffer.reserve(nbRect * 4);
			for (int i = 0; i < nbRect; ++i)
			{
				m_texCoordsBuffer.emplace_back(1.f, 1.f);
				m_texCoordsBuffer.emplace_back(0.f, 1.f);
				m_texCoordsBuffer.emplace_back(1.f, 0.f);
				m_texCoordsBuffer.emplace_back(0.f, 0.f);
			}
		}
	}

	void render()
	{
		int texId = m_texture.getValue().getTextureId();
		if(!m_verticesBuffer.empty() && !m_texCoordsBuffer.empty() && texId)
		{
			if(!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();
			
			glEnable(GL_LINE_SMOOTH);

			m_verticesVBO.bind();
			m_verticesVBO.write(m_verticesBuffer);

			m_texCoordsVBO.bind();
			m_texCoordsVBO.write(m_texCoordsBuffer);
			
			m_VAO.bind();

			m_shaderProgram.bind();
			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			glBindTexture(GL_TEXTURE_2D, texId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			m_shaderProgram.setUniformValue("tex0", 0);

			glMultiDrawArrays(GL_TRIANGLE_STRIP, m_firstBuffer.data(), m_countBuffer.data(), m_countBuffer.size());

			m_shaderProgram.release();

			m_VAO.release();

			glDisable(GL_LINE_SMOOTH);
		}
	}

protected:
	Data< std::vector<Rect> > m_rect;
	Data< ImageWrapper > m_texture;
	Data< Shader > m_shader;

	std::vector<types::Point> m_verticesBuffer, m_texCoordsBuffer;
	std::vector<GLint> m_firstBuffer;
	std::vector<GLsizei> m_countBuffer;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_texCoordsVBO;
};

int RenderTexturedRectClass = RegisterObject<RenderTexturedRect>("Render/Textured/Rectangle")
		.setName("Textured rectangle").setDescription("Draw a textured rectangle");


} // namespace panda
