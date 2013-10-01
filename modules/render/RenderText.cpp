#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <QPainter>

namespace panda {

class RenderText : public Renderer
{
public:
	PANDA_CLASS(RenderText, Renderer)

	RenderText(PandaDocument *parent)
		: Renderer(parent)
		, text(initData(&text, "text", "Text to be drawn"))
		, font(initData(&font, "font", "Font to use for the text rendering"))
		, rect(initData(&rect, "rectangle", "Rectangle in which to draw the text"))
		, color(initData(&color, "color", "Color of the text"))
		, flags(initData(&flags, static_cast<int>(Qt::AlignCenter), "alignment ", "Alignment of the text inside the rectangle"))
	{
		addInput(&text);
		addInput(&font);
		addInput(&rect);
		addInput(&color);
		addInput(&flags);

		font.setWidget("font");
		QFont tmp;
		font.setValue(tmp.toString());

		color.getAccessor().push_back(QColor());
	}

	void render(QPainter* painter)
	{
		painter->save();

		const auto& listText = text.getValue();
		const auto& listRect = rect.getValue();
		const auto& listColor = color.getValue();
		int alignment = flags.getValue();

		int nbText = listText.size();
		int nbRect = listRect.size();
		int nbColor = listColor.size();

		if(nbText && nbRect && nbColor)
		{
			painter->setBrush(Qt::NoBrush);
			painter->setPen(Qt::NoPen);
			QFont theFont;
			theFont.fromString(font.getValue());
			painter->setFont(theFont);

			if(nbText < nbRect) nbText = 1;
			if(nbColor < nbRect) nbColor = 1;

			for(int i=0; i<nbRect; ++i)
			{
				painter->setPen(QPen(listColor[i % nbColor]));

				painter->drawText(listRect[i], alignment, listText[i % nbText]);
			}
		}

		painter->restore();
	}

protected:
	Data< QVector<QString> > text;
	Data< QString > font;
	Data< QVector<QRectF> > rect;
	Data< QVector<QColor> > color;
	Data< int > flags;
};

int RenderTextClass = RegisterObject("Render/Text").setClass<RenderText>().setDescription("Draw some text");

} // namespace panda
