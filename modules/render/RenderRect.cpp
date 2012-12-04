#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QPointF>
#include <panda/Renderer.h>
#include <QPainter>

namespace panda {

class RenderRect : public Renderer
{
public:
	PANDA_CLASS(RenderRect, Renderer)

	RenderRect(PandaDocument *parent)
		: Renderer(parent)
		, rect(initData(&rect, "rectangle", "Position and size of the rectangle"))
		, color(initData(&color, "color", "Color of the plain rectangle"))
	{
		addInput(&rect);
		addInput(&color);

		rect.beginEdit()->append(QRectF(100, 100, 50, 50));
		rect.endEdit();

		color.beginEdit()->append(QColor(0,0,0));
		color.endEdit();
	}

	void render(QPainter* painter)
	{
		painter->save();

		const QVector<QRectF>& listRect = rect.getValue();
		const QVector<QColor>& listColor = color.getValue();

		int nbRect = listRect.size();
		int nbColor = listColor.size();

		if(nbRect && nbColor)
		{
			if(nbColor < nbRect) nbColor = 1;

			for(int i=0; i<nbRect; ++i)
			{
				painter->setBrush(QBrush(listColor[i % nbColor]));
				painter->setPen(Qt::NoPen);
				painter->drawRect(listRect[i]);
			}
		}

		painter->restore();
	}

protected:
	Data< QVector<QRectF> > rect;
	Data< QVector<QColor> > color;
};

int RenderRectClass = RegisterObject("Render/Rectangle").setClass<RenderRect>().setDescription("Draw a plain rectangle");

} // namespace panda
