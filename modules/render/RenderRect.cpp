#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>

#include <QPointF>
#include <QtGui/qopengl.h>

namespace panda {

class RenderRect : public Renderer
{
public:
	PANDA_CLASS(RenderRect, Renderer)

	RenderRect(PandaDocument *parent)
		: Renderer(parent)
		, rect(initData(&rect, "rectangle", "Position and size of the rectangle"))
		, lineWidth(initData(&lineWidth, "lineWidth", "Width of the line"))
		, color(initData(&color, "color", "Color of the rectangle"))
	{
		addInput(&rect);
		addInput(&lineWidth);
		addInput(&color);

		rect.getAccessor().push_back(QRectF(100, 100, 50, 50));
		color.getAccessor().push_back(QColor(0,0,0));
		lineWidth.getAccessor().push_back(0.0);
	}

	void render()
	{
		const QVector<QRectF>& listRect = rect.getValue();
		const QVector<QColor>& listColor = color.getValue();
		const QVector<double>& listWidth = lineWidth.getValue();

		int nbRect = listRect.size();
		int nbColor = listColor.size();
		int nbWidth = listWidth.size();

		if(nbRect && nbColor || nbWidth)
		{
			if(nbColor < nbRect) nbColor = 1;
			if(nbWidth < nbRect) nbWidth = 1;
			double verts[8];

			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_DOUBLE, 0, verts);
			for(int i=0; i<nbRect; ++i)
			{
				QColor valCol = listColor[i % nbColor];
				glColor4ub(valCol.red(), valCol.green(), valCol.blue(), valCol.alpha());

				glLineWidth(listWidth[i % nbWidth]);

				QRectF rect = listRect[i % nbRect];
				verts[0*2+0] = rect.right(); verts[0*2+1] = rect.top();
				verts[1*2+0] = rect.left(); verts[1*2+1] = rect.top();
				verts[2*2+0] = rect.left(); verts[2*2+1] = rect.bottom();
				verts[3*2+0] = rect.right(); verts[3*2+1] = rect.bottom();

				glDrawArrays(GL_LINE_LOOP, 0, 4);
			}
			glDisableClientState(GL_VERTEX_ARRAY);
		}
	}

protected:
	Data< QVector<QRectF> > rect;
	Data< QVector<double> > lineWidth;
	Data< QVector<QColor> > color;
};

int RenderRectClass = RegisterObject<RenderRect>("Render/Rectangle").setDescription("Draw a rectangle");

//*************************************************************************//

class RenderFilledRect : public Renderer
{
public:
	PANDA_CLASS(RenderFilledRect, Renderer)

	RenderFilledRect(PandaDocument *parent)
		: Renderer(parent)
		, rect(initData(&rect, "rectangle", "Position and size of the rectangle"))
		, color(initData(&color, "color", "Color of the rectangle"))
	{
		addInput(&rect);
		addInput(&color);

		rect.getAccessor().push_back(QRectF(100, 100, 50, 50));
		color.getAccessor().push_back(QColor(0,0,0));
	}

	void render()
	{
		const QVector<QRectF>& listRect = rect.getValue();
		const QVector<QColor>& listColor = color.getValue();

		int nbRect = listRect.size();
		int nbColor = listColor.size();

		if(nbRect && nbColor)
		{
			if(nbColor < nbRect) nbColor = 1;
			double verts[8];

			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_DOUBLE, 0, verts);
			for(int i=0; i<nbRect; ++i)
			{
				QColor valCol = listColor[i % nbColor];
				glColor4ub(valCol.red(), valCol.green(), valCol.blue(), valCol.alpha());

				QRectF rect = listRect[i % nbRect];
				verts[0*2+0] = rect.right(); verts[0*2+1] = rect.top();
				verts[1*2+0] = rect.left(); verts[1*2+1] = rect.top();
				verts[2*2+0] = rect.right(); verts[2*2+1] = rect.bottom();
				verts[3*2+0] = rect.left(); verts[3*2+1] = rect.bottom();

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
			glDisableClientState(GL_VERTEX_ARRAY);
		}
	}

protected:
	Data< QVector<QRectF> > rect;
	Data< QVector<QColor> > color;
};

int RenderFilledRectClass = RegisterObject<RenderFilledRect>("Render/Filled rectangle").setDescription("Draw a filled rectangle");


} // namespace panda
