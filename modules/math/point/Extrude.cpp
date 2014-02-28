#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Topology.h>
#include <panda/helper/Point.h>

namespace panda {

using types::Topology;

class PointListMath_Extrude : public PandaObject
{
public:
	PANDA_CLASS(PointListMath_Extrude, PandaObject)

	PointListMath_Extrude(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "List of control points"))
		, output(initData(&output, "output", "Topology created from the extrusion"))
		, width(initData(&width, 10.0, "width", "Width of the line"))
		, capStyle(initData(&capStyle, "cap", "Style of the caps"))
		, joinStyle(initData(&joinStyle, "join", "Style of the joins"))
	{
		addInput(&input);
		addInput(&width);
		addInput(&capStyle);
		addInput(&joinStyle);

		capStyle.setWidget("enum");
		capStyle.setWidgetData("Butt cap;Round cap;Square cap");
		joinStyle.setWidget("enum");
		joinStyle.setWidgetData("Miter join;Round join;Bevel join");

		addOutput(&output);
	}

	void update()
	{
		const QVector<QPointF>& pts = input.getValue();
		const double w = width.getValue() / 2;
		auto topo = output.getAccessor();
		topo->clear();

		int nbPts = pts.size();
		if(!nbPts || w < 1)
			return;

		QVector<QPointF> normals;
		normals.resize(nbPts-1);
		for(int i=0; i<nbPts-1; ++i)
		{
			const QPointF &pt1=pts[i], &pt2=pts[i+1];
			normals[i] = helper::normalize(QPointF(pt2.y()-pt1.y(), pt1.x()-pt2.x()));
		}

		// Start cap
		Topology::PointID prevPtsId[3];
		prevPtsId[0] = topo->addPoint(pts[0] - normals[0] * w);
		prevPtsId[1] = topo->addPoint(pts[0]);
		prevPtsId[2] = topo->addPoint(pts[0] + normals[0] * w);

		// Line
		for(int i=1; i<nbPts-1; ++i)
		{
			// Main extrusion
			// For the time being, do a miter join
			QPointF dir = normals[i-1] + normals[i];
			dir = w / helper::dot(dir, normals[i-1]) * normals[i-1]
				+ w / helper::dot(dir, normals[i]) * normals[i];
			Topology::PointID nextPtsId[3];
			nextPtsId[0] = topo->addPoint(pts[i] - dir);
			nextPtsId[1] = topo->addPoint(pts[i]);
			nextPtsId[2] = topo->addPoint(pts[i] + dir);

			// Create the triangles
			topo->addPolygon(nextPtsId[0], prevPtsId[0], prevPtsId[1]);
			topo->addPolygon(nextPtsId[0], prevPtsId[1], nextPtsId[1]);
			topo->addPolygon(nextPtsId[1], prevPtsId[1], prevPtsId[2]);
			topo->addPolygon(nextPtsId[1], prevPtsId[2], nextPtsId[2]);

			// Join
			for(int j=0; j<3; ++j)
				prevPtsId[j] = nextPtsId[j];
		}

		// Last segment
		Topology::PointID nextPtsId[3];
		nextPtsId[0] = topo->addPoint(pts[nbPts-1] - normals[nbPts-2] * w);
		nextPtsId[1] = topo->addPoint(pts[nbPts-1]);
		nextPtsId[2] = topo->addPoint(pts[nbPts-1] + normals[nbPts-2] * w);
		topo->addPolygon(nextPtsId[0], prevPtsId[0], prevPtsId[1]);
		topo->addPolygon(nextPtsId[0], prevPtsId[1], nextPtsId[1]);
		topo->addPolygon(nextPtsId[1], prevPtsId[1], prevPtsId[2]);
		topo->addPolygon(nextPtsId[1], prevPtsId[2], nextPtsId[2]);

		// End cap

		cleanDirty();
	}

protected:
	Data< QVector<QPointF> > input;
	Data< Topology > output;
	Data< double > width;
	Data< int > capStyle, joinStyle;
};

int PointListMath_ExtrudeClass = RegisterObject<PointListMath_Extrude>("Math/List of points/Extrude")
		.setDescription("Add width to a line");

} // namespace Panda


