#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>

#include <QPointF>
#include <QtGui/qopengl.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace panda {

class RenderCircle : public Renderer
{
public:
	PANDA_CLASS(RenderCircle, Renderer)

	RenderCircle(PandaDocument *parent)
		: Renderer(parent)
		, center(initData(&center, "center", "Center position of the circle"))
		, radius(initData(&radius, "radius", "Radius of the circle" ))
		, lineWidth(initData(&lineWidth, "lineWidth", "Width of the line"))
		, color(initData(&color, "color", "Color of the circle"))
	{
		addInput(&center);
		addInput(&radius);
		addInput(&lineWidth);
		addInput(&color);

		center.getAccessor().push_back(QPointF(100, 100));
		radius.getAccessor().push_back(5.0);
		lineWidth.getAccessor().push_back(0.0);
		color.getAccessor().push_back(QColor(0,0,0));
	}

	void render()
	{
		const QVector<QPointF>& listCenter = center.getValue();
		const QVector<double>& listRadius = radius.getValue();
		const QVector<double>& listWidth = lineWidth.getValue();
		const QVector<QColor>& listColor = color.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();
		int nbWidth = listWidth.size();
		int nbColor = listColor.size();

		if(nbCenter && nbRadius && nbColor && nbWidth)
		{
			if(nbRadius < nbCenter) nbRadius = 1;
			if(nbWidth < nbCenter) nbWidth = 1;
			if(nbColor < nbCenter) nbColor = 1;
			std::vector<double> vertices;

			glEnableClientState(GL_VERTEX_ARRAY);
			for(int i=0; i<nbCenter; ++i)
			{
				QColor valCol = listColor[i % nbColor];
				glColor4ub(valCol.red(), valCol.green(), valCol.blue(), valCol.alpha());

				glLineWidth(listWidth[i % nbWidth]);

				double valRadius = listRadius[i % nbRadius];
				int nbSeg = static_cast<int>(floor(valRadius * M_PI * 2));
				if(nbSeg < 3) continue;
				vertices.resize((nbSeg + 2) * 2);

				QPointF valCenter = listCenter[i];
				for(int i=0; i<nbSeg; i++)
				{
					double t = i / static_cast<double>(nbSeg) * 2 * M_PI;
					vertices[i*2  ] = valCenter.x() + cos(t) * valRadius;
					vertices[i*2+1] = valCenter.y() + sin(t) * valRadius;
				}
				glVertexPointer(2, GL_DOUBLE, 0, vertices.data());
				glDrawArrays(GL_LINE_LOOP, 0, nbSeg);
			}
			glDisableClientState(GL_VERTEX_ARRAY);
		}
	}

protected:
	Data< QVector<QPointF> > center;
	Data< QVector<double> > radius, lineWidth;
	Data< QVector<QColor> > color;
};

int RenderCircleClass = RegisterObject<RenderCircle>("Render/Circle").setDescription("Draw a circle");

} // namespace panda
