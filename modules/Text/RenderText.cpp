#include <GL/glew.h>

#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>
#include <panda/helper/algorithm.h>
#include <panda/helper/Font.h>
#include <panda/helper/system/FileRepository.h>
#include <panda/graphics/Buffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/graphics/VertexArrayObject.h>

#include "FontsCache.h"

#include <glm/gtx/matrix_transform_2d.hpp>

namespace panda {

using types::Color;
using types::Point;
using types::Rect;
using types::Shader;

class RenderText : public Renderer
{
public:
	PANDA_CLASS(RenderText, Renderer)

	RenderText(PandaDocument* parent)
		: Renderer(parent)
		, m_text(initData("text", "Text to be drawn"))
		, m_font(initData("font", "Font to use for the text rendering"))
		, m_rect(initData("rectangle", "Rectangle in which to draw the text"))
		, m_color(initData("color", "Color of the text"))
		, m_alignH(initData(0, "align H", "Horizontal alignment of the text"))
		, m_alignV(initData(3, "align V", "Vertical alignment of the text"))
	{
		addInput(m_text);
		addInput(m_font);
		addInput(m_rect);
		addInput(m_color);
		addInput(m_alignH);
		addInput(m_alignV);

		m_font.setWidget("font");
	
		m_alignH.setWidget("enum");
		m_alignH.setWidgetData("Left;Right;Center");

		m_alignV.setWidget("enum");
		m_alignV.setWidgetData("Top;Bottom;Center;Baseline");

		m_color.getAccessor().push_back(Color::black());

		m_shader.setSourceFromFile(Shader::ShaderType::Vertex, "shaders/Text.v.glsl");
		m_shader.setSourceFromFile(Shader::ShaderType::Fragment, "shaders/Text.f.glsl");

		m_prevFont = helper::Font().toString();
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

		m_colorsVBO.create();
		m_colorsVBO.bind();
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(2);

		m_VAO.release();
	}

	struct LineInfo
	{
		int startIndex = 0, endIndex = 0;
		float right = 0;
	};

	void addText(const std::string& text, Color color, Rect area)
	{
		int startText = m_verticesBuffer.size();
		bool firstChar = true;

		Point pos = area.bottomLeft();
		Rect textArea(pos, Point());
		float baselinePos = pos.y;
		auto texFont = m_texFont->fontPtr();

		std::vector<LineInfo> lines;
		
		for (unsigned int i = 0; i < text.size(); ++i)
		{
			if (text[i] == '\n')
			{
				pos.x = area.left();
				pos.y += texFont->height;
				firstChar = true;
				continue;
			}

			auto glyph = ftgl::texture_font_get_glyph(texFont, &text[i]);
			if (!glyph)
				continue;

			float kerning = 0.f;
			int startGlyph = m_verticesBuffer.size();

			if (firstChar)
			{
				if (!lines.empty())
					lines.back().endIndex = startGlyph;
				lines.push_back(LineInfo());
				auto& line = lines.back();
				line.startIndex = startGlyph;
				line.right = pos.x;
				baselinePos = pos.y;
				firstChar = false;
			}
			else
				kerning = texture_glyph_get_kerning(glyph, &text[i - 1]);

			pos.x += kerning;
			int gw = glyph->width, gh = glyph->height;
			int gox = glyph->offset_x, goy = glyph->offset_y;

			float x0 = pos.x + gox;
			float y0 = pos.y - goy;
			float x1 = x0 + gw;
			float y1 = y0 + gh;

			textArea |= Rect(x0, y0, x1, y1);
			lines.back().right = x1;

			m_verticesBuffer.emplace_back(x0, y0);
			m_verticesBuffer.emplace_back(x0, y1);
			m_verticesBuffer.emplace_back(x1, y1);
			m_verticesBuffer.emplace_back(x1, y0);

			m_texCoordsBuffer.emplace_back(glyph->s0, glyph->t0);
			m_texCoordsBuffer.emplace_back(glyph->s0, glyph->t1);
			m_texCoordsBuffer.emplace_back(glyph->s1, glyph->t1);
			m_texCoordsBuffer.emplace_back(glyph->s1, glyph->t0);

			m_colorsBuffer.emplace_back(color);
			m_colorsBuffer.emplace_back(color);
			m_colorsBuffer.emplace_back(color);
			m_colorsBuffer.emplace_back(color);

			GLuint indices[6] = { 0, 1, 2, 0, 2, 3 };
			for (int j = 0; j < 6; ++j)
				m_indicesBuffer.push_back(startGlyph + indices[j]);

			auto adv = glyph->advance_x;
			pos.x += adv;
		}
		
		if (!lines.empty())
			lines.back().endIndex = m_verticesBuffer.size();

		int alignH = m_alignH.getValue(), alignV = m_alignV.getValue();
		if (alignH != 0)
		{
			float delta = 0;
			auto left = area.left();

			for (const auto& line : lines)
			{
				switch (alignH)
				{
				case 0: // Left
					break;
				case 1: // Right
					delta = area.width() - (line.right - left);
					break;
				case 2: // Center
					delta = (area.width() - (line.right - left)) / 2;
					break;
				}

				if (delta != 0)
				{
					for (int i = line.startIndex; i < line.endIndex; ++i)
						m_verticesBuffer[i].x += delta;
				}
			}
		}

		if (alignV != 3 || pos.y != area.bottom()) // Must move the text
		{
			float delta;
			switch (alignV)
			{
			case 0: // Top
				delta = area.top() - textArea.top();
				break;
			case 1: // Bottom
				delta = area.bottom() - textArea.bottom();
				break;
			case 2: // Center
				delta = area.top() - textArea.top() + (area.height() - textArea.height()) / 2;
				break;
			case 3: // Baseline
				delta = area.bottom() - baselinePos;
				break;
			}

			if (delta != 0)
			{
				for (int i = startText, nb = m_verticesBuffer.size(); i < nb; ++i)
					m_verticesBuffer[i].y += delta;
			}
		}
	}

	void render()
	{
		const auto& listText = m_text.getValue();
		const auto& listRect = m_rect.getValue();
		const auto& listColor = m_color.getValue();

		int nbText = listText.size();
		int nbRect = listRect.size();
		int nbColor = listColor.size();

		if(nbText && nbRect && nbColor)
		{
			if (m_font.getValue() != m_prevFont)
			{
				auto fontTxt = m_font.getValue();
				m_texFont = FontsCache::instance().getFont(fontTxt);
				m_prevFont = fontTxt;
			}

			if (!m_texFont)
				return;

			if (!m_shader.apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_verticesBuffer.clear();
			m_texCoordsBuffer.clear();
			m_colorsBuffer.clear();
			m_indicesBuffer.clear();

			if (nbText < nbRect) nbText = 1;
			if (nbColor < nbRect) nbColor = 1;
			for (int i = 0; i < nbRect; ++i)
			{
				const auto& text = listText[i % nbText];
				const auto& rect = listRect[i];
				const auto& color = listColor[i % nbColor];

				addText(text, color, rect);
			}

			m_verticesVBO.bind();
			m_verticesVBO.write(m_verticesBuffer);

			m_texCoordsVBO.bind();
			m_texCoordsVBO.write(m_texCoordsBuffer);

			m_colorsVBO.bind();
			m_colorsVBO.write(m_colorsBuffer);

			m_VAO.bind();

			m_shaderProgram.bind();
			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			glBindTexture(GL_TEXTURE_2D, m_texFont->texId());

			glDrawElements(GL_TRIANGLES, m_indicesBuffer.size(), GL_UNSIGNED_INT, m_indicesBuffer.data());

			m_shaderProgram.release();
			m_VAO.release();
		}
	}

protected:
	Data< std::vector<std::string> > m_text;
	Data< std::string > m_font;
	Data< std::vector<Rect> > m_rect;
	Data< std::vector<Color> > m_color;
	Data< int > m_alignH, m_alignV;

	std::string m_prevFont;
	std::vector<Point> m_verticesBuffer, m_texCoordsBuffer;
	std::vector<Color> m_colorsBuffer;
	std::vector<unsigned int> m_indicesBuffer;

	Shader m_shader;
	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_texCoordsVBO, m_colorsVBO;

	std::shared_ptr<TextureFont> m_texFont;
};

int RenderTextClass = RegisterObject<RenderText>("Render/Text").setDescription("Draw some text");

//****************************************************************************//

class RenderText_OnCurve : public Renderer
{
public:
	PANDA_CLASS(RenderText_OnCurve, Renderer)

	RenderText_OnCurve(PandaDocument* parent)
		: Renderer(parent)
		, m_text(initData("text", "Text to be drawn"))
		, m_font(initData("font", "Font to use for the text rendering"))
		, m_position(initData("position", "Position of the letters"))
		, m_rotation(initData("rotation", "Rotation of the letters"))
		, m_color(initData("color", "Color of the text"))
	{
		addInput(m_text);
		addInput(m_font);
		addInput(m_position);
		addInput(m_rotation);
		addInput(m_color);

		m_font.setWidget("font");

		m_rotation.getAccessor().push_back(0.f);
		m_color.getAccessor().push_back(Color::black());

		m_shader.setSourceFromFile(Shader::ShaderType::Vertex, "shaders/Text.v.glsl");
		m_shader.setSourceFromFile(Shader::ShaderType::Fragment, "shaders/Text.f.glsl");

		m_prevFont = helper::Font().toString();
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

		m_colorsVBO.create();
		m_colorsVBO.bind();
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(2);

		m_VAO.release();
	}

	void addGlyph(ftgl::texture_glyph_t* glyph, Point pos, float rot, Color color)
	{
		int startText = m_verticesBuffer.size();

		int gw = glyph->width, gh = glyph->height;
		int gox = glyph->offset_x, goy = glyph->offset_y;

		float x0 = pos.x + gox - gw /2;
		float y0 = pos.y - goy;
		float x1 = x0 + gw;
		float y1 = y0 + gh;

		int startGlyph = m_verticesBuffer.size();

		if (rot == 0)
		{
			m_verticesBuffer.emplace_back(x0, y0);
			m_verticesBuffer.emplace_back(x0, y1);
			m_verticesBuffer.emplace_back(x1, y1);
			m_verticesBuffer.emplace_back(x1, y0);
		}
		else
		{
			static const float pi180 = 0.01745329f;
			rot *= pi180;
			glm::mat3x3 mat;
			mat = glm::translate(mat, glm::vec2(pos.x, pos.y));
			mat = glm::rotate(mat, rot);
			mat = glm::translate(mat, glm::vec2(-pos.x, -pos.y));

			auto pt = mat * glm::vec3(x0, y0, 1);
			m_verticesBuffer.emplace_back(pt.x, pt.y);
			pt = mat * glm::vec3(x0, y1, 1);
			m_verticesBuffer.emplace_back(pt.x, pt.y);
			pt = mat * glm::vec3(x1, y1, 1);
			m_verticesBuffer.emplace_back(pt.x, pt.y);
			pt = mat * glm::vec3(x1, y0, 1);
			m_verticesBuffer.emplace_back(pt.x, pt.y);
		}

		m_texCoordsBuffer.emplace_back(glyph->s0, glyph->t0);
		m_texCoordsBuffer.emplace_back(glyph->s0, glyph->t1);
		m_texCoordsBuffer.emplace_back(glyph->s1, glyph->t1);
		m_texCoordsBuffer.emplace_back(glyph->s1, glyph->t0);

		m_colorsBuffer.emplace_back(color);
		m_colorsBuffer.emplace_back(color);
		m_colorsBuffer.emplace_back(color);
		m_colorsBuffer.emplace_back(color);

		GLuint indices[6] = { 0, 1, 2, 0, 2, 3 };
		for (int j = 0; j < 6; ++j)
			m_indicesBuffer.push_back(startGlyph + indices[j]);
	}

	void render()
	{
		const auto& text = m_text.getValue();
		const auto& listPos = m_position.getValue();
		const auto& listRot = m_rotation.getValue();
		const auto& listColor = m_color.getValue();

		if (!text.empty() && !listPos.empty() && !listRot.empty() && !listColor.empty())
		{
			if (m_font.getValue() != m_prevFont)
			{
				auto fontTxt = m_font.getValue();
				m_texFont = FontsCache::instance().getFont(fontTxt);
				m_prevFont = fontTxt;
			}

			if (!m_texFont)
				return;

			if (!m_shader.apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			int nbPos = listPos.size();
			int nbRot = listRot.size();
			int nbColor = listColor.size();

			m_verticesBuffer.clear();
			m_texCoordsBuffer.clear();
			m_colorsBuffer.clear();
			m_indicesBuffer.clear();

			if (nbRot < nbPos) nbRot = 1;
			if (nbColor < nbPos) nbColor = 1;
			int glyphIt = 0;
			auto texFont = m_texFont->fontPtr();
			for (int i = 0, len = text.size(); glyphIt < len && glyphIt < nbPos; ++i)
			{
				auto glyph = ftgl::texture_font_get_glyph(texFont, &text[i]);
				if (!glyph)
					continue;

				addGlyph(glyph, listPos[glyphIt], listRot[i % nbRot], listColor[i % nbColor]);
				++glyphIt;
			}

			m_verticesVBO.bind();
			m_verticesVBO.write(m_verticesBuffer);

			m_texCoordsVBO.bind();
			m_texCoordsVBO.write(m_texCoordsBuffer);

			m_colorsVBO.bind();
			m_colorsVBO.write(m_colorsBuffer);

			m_VAO.bind();

			m_shaderProgram.bind();
			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			glBindTexture(GL_TEXTURE_2D, m_texFont->texId());

			glDrawElements(GL_TRIANGLES, m_indicesBuffer.size(), GL_UNSIGNED_INT, m_indicesBuffer.data());

			m_shaderProgram.release();
			m_VAO.release();
		}
	}

protected:
	Data< std::string > m_text, m_font;
	Data< std::vector<Point> > m_position;
	Data< std::vector<float> > m_rotation;
	Data< std::vector<Color> > m_color;

	std::string m_prevFont;
	std::vector<Point> m_verticesBuffer, m_texCoordsBuffer;
	std::vector<Color> m_colorsBuffer;
	std::vector<unsigned int> m_indicesBuffer;

	Shader m_shader;
	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_texCoordsVBO, m_colorsVBO;

	std::shared_ptr<TextureFont> m_texFont;
};

int RenderText_OnCurveClass = RegisterObject<RenderText_OnCurve>("Render/Text on curve").setDescription("Draw some on a curve (each letter its own position)");


} // namespace panda
