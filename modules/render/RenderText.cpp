#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>

#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QFont>

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
		, alignH(initData(&alignH, 2, "align H", "Horizontal alignment of the text"))
		, alignV(initData(&alignV, 2, "align V", "Vertical alignment of the text"))
	{
		addInput(&text);
		addInput(&font);
		addInput(&rect);
		addInput(&color);
		addInput(&alignH);
		addInput(&alignV);

		font.setWidget("font");
		QFont tmp;
		font.setValue(tmp.toString());

		alignH.setWidget("enum");
		alignH.setWidgetData("Left;Right;Center;Justify");

		alignV.setWidget("enum");
		alignV.setWidgetData("Top;Bottom;Center;Baseline");

		color.getAccessor().push_back(QColor());
	}

	void render()
	{
		QOpenGLPaintDevice device(parentDocument->getRenderSize());
		QPainter painter;
		painter.begin(&device);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);

		const auto& listText = text.getValue();
		const auto& listRect = rect.getValue();
		const auto& listColor = color.getValue();

		const int alignHVals[] = {Qt::AlignLeft, Qt::AlignRight, Qt::AlignHCenter, Qt::AlignJustify};
		const int alignVVals[] = {Qt::AlignTop, Qt::AlignBottom, Qt::AlignVCenter, Qt::AlignBaseline};
		int alignHIndex = qBound(0, alignH.getValue(), 3);
		int alignVIndex = qBound(0, alignV.getValue(), 3);
		int alignment = alignHVals[alignHIndex] | alignVVals[alignVIndex];

		int nbText = listText.size();
		int nbRect = listRect.size();
		int nbColor = listColor.size();

		if(nbText && nbRect && nbColor)
		{
			painter.setBrush(Qt::NoBrush);
			painter.setPen(Qt::NoPen);
			QFont theFont;
			theFont.fromString(font.getValue());
			painter.setFont(theFont);

			if(nbText < nbRect) nbText = 1;
			if(nbColor < nbRect) nbColor = 1;

			for(int i=0; i<nbRect; ++i)
			{
				painter.setPen(QPen(listColor[i % nbColor]));
				painter.drawRect(listRect[i]);

				painter.drawText(listRect[i], alignment, listText[i % nbText]);
			}
		}

		painter.end();
	}

protected:
	Data< QVector<QString> > text;
	Data< QString > font;
	Data< QVector<QRectF> > rect;
	Data< QVector<QColor> > color;
	Data< int > alignH, alignV;
};

int RenderTextClass = RegisterObject<RenderText>("Render/Text").setDescription("Draw some text");

} // namespace panda
