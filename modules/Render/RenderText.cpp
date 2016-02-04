#include <GL/glew.h>

#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/Rect.h>
#include <panda/helper/algorithm.h>
#include <panda/helper/Font.h>
#include <panda/helper/system/FileRepository.h>
#include <panda/graphics/Buffer.h>
#include <panda/graphics/VertexArrayObject.h>

#include <freetype-gl/freetype-gl.h>

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

	~RenderText()
	{
		freeFont();
	}

	void freeFont()
	{
		if (m_texFont)
			ftgl::texture_font_delete(m_texFont);
		m_texFont = nullptr;

		if (m_atlas)
			ftgl::texture_atlas_delete(m_atlas);
		m_atlas = nullptr;
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

	void changeFont()
	{
		auto fontTxt = m_font.getValue();
		helper::Font font(fontTxt);
		if (font.path.empty())
			font = helper::Font();

		m_prevFont = fontTxt;

		freeFont();

		auto fontPath = helper::system::DataRepository.findFile(font.path);
		if (fontPath.empty())
			return;

		const char* cache = " !\"#$%&'()*+,-./0123456789:;<=>?"
			"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
			"`abcdefghijklmnopqrstuvwxyz{|}~";

		for (int dim = 512; dim <= 4096; dim *= 2)
		{
			m_atlas = ftgl::texture_atlas_new(dim, dim, 1);
			m_texFont = ftgl::texture_font_new_from_file(m_atlas, static_cast<float>(font.pointSize), fontPath.c_str());

			auto missed = ftgl::texture_font_load_glyphs(m_texFont, cache);
			if (!missed)
				break;

			freeFont();
		}
	}

	void addText(const std::string& text, Color color, Rect area)
	{
		int startText = m_verticesBuffer.size();
		PReal width = 0;
		int maxOffsetY = 0, under = 0;

		Point pos = area.bottomLeft();
		for (unsigned int i = 0; i < text.size(); ++i)
		{
			auto glyph = ftgl::texture_font_get_glyph(m_texFont, &text[i]);
			if (!glyph)
				continue;

			float kerning = 0.f;
			if (i)
				kerning = texture_glyph_get_kerning(glyph, &text[i - 1]);

			pos.x += kerning;
			int gw = glyph->width, gh = glyph->height;
			int gox = glyph->offset_x, goy = glyph->offset_y;
			int u = gh - goy;
			
			if (goy > maxOffsetY) maxOffsetY = goy;
			if (u > under) under = u;
			width += kerning + glyph->offset_x + gw;

			PReal x0 = pos.x + gox;
			PReal y0 = pos.y - goy;
			PReal x1 = x0 + gw;
			PReal y1 = y0 + gh;

			int startGlyph = m_verticesBuffer.size();
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
			width += adv - gw - gox;
		}

		int height = maxOffsetY + under;
		int alignH = m_alignH.getValue(), alignV = m_alignV.getValue();
		if (alignH != 0 || alignV != 3) // Must move the text
		{
			Point delta;
			switch (alignH)
			{
			case 0: // Left
				break;
			case 1: // Right
				delta.x = area.width() - width;
				break;
			case 2: // Center
				delta.x = (area.width() - width) / 2;
				break;
			}

			switch (alignV)
			{
			case 0: // Top
				delta.y = maxOffsetY - area.height();
				break;
			case 1: // Bottom
				delta.y = static_cast<PReal>(-under);
				break;
			case 2: // Center
				delta.y = -under - (area.height() - height) / 2;
				break;
			case 3: // Baseline
				break;
			}

			if (delta != Point::zero())
			{
				for (int i = startText, nb = m_verticesBuffer.size(); i < nb; ++i)
					m_verticesBuffer[i] += delta;
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
			if (!m_shader.apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			if (m_font.getValue() != m_prevFont)
				changeFont();

			if (!m_texFont || !m_atlas)
				return;

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

			glBindTexture(GL_TEXTURE_2D, m_atlas->id);

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

	ftgl::texture_atlas_t* m_atlas = nullptr;
	ftgl::texture_font_t* m_texFont = nullptr;
};

int RenderTextClass = RegisterObject<RenderText>("Render/Text").setDescription("Draw some text");

} // namespace panda
