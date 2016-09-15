#include <GL/glew.h>

#include <panda/document/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/helper/Font.h>
#include <panda/types/Point.h>
#include <panda/types/Rect.h>

#include "FontsCache.h"

namespace panda {

using types::Point;
using types::Rect;

class Text_Position : public PandaObject
{
public:
	PANDA_CLASS(Text_Position, PandaObject)

	Text_Position(PandaDocument* parent)
		: PandaObject(parent)
		, m_text(initData("text", "Text to be drawn"))
		, m_font(initData("font", "Font to use for the text rendering"))
		, m_position(initData("position", "Position (center, baseline) of each letter in the text"))
	{
		addInput(m_text);
		addInput(m_font);

		addOutput(m_position);

		m_font.setWidget("font");
	
		m_prevFont = helper::Font().toString();

		setUpdateOnMainThread(true);
	}

	void update()
	{
		const auto& text = m_text.getValue();
		auto position = m_position.getAccessor();

		position.clear();

		if (text.empty())
			return;

		if (m_font.getValue() != m_prevFont)
		{
			auto fontTxt = m_font.getValue();
			m_texFont = FontsCache::instance().getFont(fontTxt);
			m_prevFont = fontTxt;
		}

		if (!m_texFont)
			return;

		bool firstChar = true;

		Point pos;
		Rect textArea(pos, Point());
		auto texFont = m_texFont->fontPtr();
		
		for (unsigned int i = 0; i < text.size(); ++i)
		{
			if (text[i] == '\n')
			{
				pos.x = 0;
				pos.y += texFont->height;
				firstChar = true;
				continue;
			}

			auto glyph = ftgl::texture_font_get_glyph(texFont, &text[i]);
			if (!glyph)
				continue;

			float kerning = 0.f;

			if (firstChar)
				firstChar = false;
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

			position.push_back(pos + Point(gw / 2.f, 0.f));

			pos.x += glyph->advance_x;
		}
	}

protected:
	Data< std::string > m_text;
	Data< std::string > m_font;
	Data< std::vector<Point> > m_position;

	std::string m_prevFont;

	std::shared_ptr<TextureFont> m_texFont;
};

int Text_PositionClass = RegisterObject<Text_Position>("Generator/Point/Text position").setDescription("Get the position of each letter in a text");

//****************************************************************************//

} // namespace panda
