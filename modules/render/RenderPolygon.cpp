#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Topology.h>
#include <panda/Renderer.h>
#include <QPainter>

namespace panda {

class RenderPolygon : public Renderer
{
public:
	PANDA_CLASS(RenderPolygon, Renderer)

	RenderPolygon(PandaDocument *parent)
		: Renderer(parent)
		, topology(initData(&topology, "topology", "Polygon to render"))
		, color(initData(&color, "color", "Color of the polygon"))
	{
		addInput(&topology);
		addInput(&color);

		color.getAccessor().push_back(QColor());
	}

	void render(QPainter* painter)
	{
		const Topology& topo = topology.getValue();
		const QVector<QColor>& listColor = color.getValue();

		int nbPoly = topo.getNumberOfPolygons();
		int nbColor = listColor.size();

		const QVector<QPointF>& pts = topo.getPoints();

		if(nbPoly && nbColor)
		{
			if(nbColor < nbPoly) nbColor = 1;

			painter->save();
			painter->setPen(Qt::NoPen);

			for(int i=0; i<nbPoly; ++i)
			{
				painter->setBrush(QBrush(listColor[i % nbColor]));

				QPolygonF qpoly;
				const Topology::Polygon& poly = topo.getPolygon(i);
				for(int j=0; j<poly.size(); ++j)
					qpoly << pts[poly[j]];

				painter->drawPolygon(qpoly);
			}

			painter->restore();
		}
	}

protected:
	Data<Topology> topology;
	Data< QVector<QColor> > color;
};

int RenderPolygonClass = RegisterObject("Render/Polygon").setClass<RenderPolygon>().setDescription("Draw a polygon");

} // namespace panda
