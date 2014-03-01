#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Topology.h>
#include <panda/helper/Point.h>

#define _USE_MATH_DEFINES
#include <math.h>

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
		int cap = capStyle.getValue();
		int join = joinStyle.getValue();
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
		QPointF dir = normals[0] * w;
		prevPtsId[0] = topo->addPoint(pts[0] + dir);
		prevPtsId[1] = topo->addPoint(pts[0]);
		prevPtsId[2] = topo->addPoint(pts[0] - dir);
		switch(cap)
		{
		default:
		case 0:	// Butt cap (nothing to do)
			break;
		case 1:	// Round cap
		{
			int nb = static_cast<int>(floor(w * M_PI));
			double angle = M_PI / nb; // We do a half turn
			double ca = cos(angle), sa = sin(angle);
			QPointF center = pts[0];
			Topology::PointID ptId = prevPtsId[0];
			for(int i=0; i<nb; ++i)
			{
				QPointF pt = QPointF(dir.x()*ca+dir.y()*sa, dir.y()*ca-dir.x()*sa);
				Topology::PointID newPtId = topo->addPoint(center + pt);
				topo->addPolygon(ptId, newPtId, prevPtsId[1]);

				dir = pt;
				ptId = newPtId;
			}
			break;
		}
		case 2: // Square cap
		{
			QPointF dir2 = QPointF(dir.y(), -dir.x());
			Topology::PointID addPtsId[3];
			addPtsId[0] = topo->addPoint(pts[0] + dir + dir2);
			addPtsId[1] = topo->addPoint(pts[0]	      + dir2);
			addPtsId[2] = topo->addPoint(pts[0] - dir + dir2);

			topo->addPolygon(prevPtsId[0], addPtsId[0], addPtsId[1]);
			topo->addPolygon(prevPtsId[0], addPtsId[1], prevPtsId[1]);
			topo->addPolygon(prevPtsId[1], addPtsId[1], addPtsId[2]);
			topo->addPolygon(prevPtsId[1], addPtsId[2], prevPtsId[2]);
			break;
		}
		} // switch

		// Line
		for(int i=1; i<nbPts-1; ++i)
		{
			Topology::PointID nextPtsId[3];
			// Main extrusion
			dir = normals[i-1] + normals[i];
			dir = w / helper::dot(dir, normals[i-1]) * normals[i-1]
				+ w / helper::dot(dir, normals[i]) * normals[i];
			double side = helper::cross(normals[i-1], normals[i]);

			// Join
			switch(join)
			{
			default:
			case 0: // Miter join
			{
				// TODO : do a Bevel join if the angle is too small
				nextPtsId[0] = topo->addPoint(pts[i] + dir);
				nextPtsId[1] = topo->addPoint(pts[i]);
				nextPtsId[2] = topo->addPoint(pts[i] - dir);

				// Create the triangles
				topo->addPolygon(nextPtsId[0], prevPtsId[0], prevPtsId[1]);
				topo->addPolygon(nextPtsId[0], prevPtsId[1], nextPtsId[1]);
				topo->addPolygon(nextPtsId[1], prevPtsId[1], prevPtsId[2]);
				topo->addPolygon(nextPtsId[1], prevPtsId[2], nextPtsId[2]);

				for(int j=0; j<3; ++j)
					prevPtsId[j] = nextPtsId[j];
				break;
			}
	/*		case 1: // Round join
			{
				for(int j=0; j<3; ++j)
					prevPtsId[j] = nextPtsId[j];
				break;
			}
	*/		case 2: // Bevel join
			{
				if(side > 0)
				{
					Topology::PointID addPtId;
					addPtId = topo->addPoint(pts[i] + normals[i-1] * w);
					nextPtsId[0] = topo->addPoint(pts[i] + normals[i] * w);
					nextPtsId[1] = topo->addPoint(pts[i]);
					nextPtsId[2] = topo->addPoint(pts[i] - dir);

					topo->addPolygon(nextPtsId[0], addPtId, nextPtsId[1]);
					topo->addPolygon(addPtId, prevPtsId[0], prevPtsId[1]);
					topo->addPolygon(addPtId, prevPtsId[1], nextPtsId[1]);
					topo->addPolygon(nextPtsId[1], prevPtsId[1], prevPtsId[2]);
					topo->addPolygon(nextPtsId[1], prevPtsId[2], nextPtsId[2]);

					for(int j=0; j<3; ++j)
						prevPtsId[j] = nextPtsId[j];
				}
				else if(side < 0)
				{
					Topology::PointID addPtId;
					nextPtsId[0] = topo->addPoint(pts[i] + dir);
					nextPtsId[1] = topo->addPoint(pts[i]);
					nextPtsId[2] = topo->addPoint(pts[i] - normals[i] * w);
					addPtId = topo->addPoint(pts[i] - normals[i-1] * w);

					topo->addPolygon(nextPtsId[2], nextPtsId[1], addPtId);
					topo->addPolygon(nextPtsId[0], prevPtsId[0], prevPtsId[1]);
					topo->addPolygon(nextPtsId[0], prevPtsId[1], nextPtsId[1]);
					topo->addPolygon(nextPtsId[1], prevPtsId[1], prevPtsId[2]);
					topo->addPolygon(nextPtsId[1], prevPtsId[2], addPtId);

					for(int j=0; j<3; ++j)
						prevPtsId[j] = nextPtsId[j];
				}
				break;
			}
			}
		}

		// Last segment
		Topology::PointID nextPtsId[3];
		dir = normals[nbPts-2] * w;
		nextPtsId[0] = topo->addPoint(pts[nbPts-1] + dir);
		nextPtsId[1] = topo->addPoint(pts[nbPts-1]);
		nextPtsId[2] = topo->addPoint(pts[nbPts-1] - dir);
		topo->addPolygon(nextPtsId[0], prevPtsId[0], prevPtsId[1]);
		topo->addPolygon(nextPtsId[0], prevPtsId[1], nextPtsId[1]);
		topo->addPolygon(nextPtsId[1], prevPtsId[1], prevPtsId[2]);
		topo->addPolygon(nextPtsId[1], prevPtsId[2], nextPtsId[2]);

		// End cap
		switch(cap)
		{
		default:
		case 0:	// Butt cap (nothing to do)
			break;
		case 1:	// Round cap
		{
			int nb = static_cast<int>(floor(w * M_PI));
			double angle = M_PI / nb; // We do a half turn
			double ca = cos(angle), sa = sin(angle);
			QPointF center = pts[nbPts-1];
			dir = -dir;
			Topology::PointID ptId = nextPtsId[2];
			for(int i=0; i<nb; ++i)
			{
				QPointF pt = QPointF(dir.x()*ca+dir.y()*sa, dir.y()*ca-dir.x()*sa);
				Topology::PointID newPtId = topo->addPoint(center + pt);
				topo->addPolygon(ptId, newPtId, nextPtsId[1]);

				dir = pt;
				ptId = newPtId;
			}
			break;
		}
		case 2: // Square cap
		{
			QPointF dir2 = QPointF(dir.y(), -dir.x());
			Topology::PointID addPtsId[3];
			addPtsId[0] = topo->addPoint(pts[nbPts-1] + dir - dir2);
			addPtsId[1] = topo->addPoint(pts[nbPts-1]	    - dir2);
			addPtsId[2] = topo->addPoint(pts[nbPts-1] - dir - dir2);

			topo->addPolygon(addPtsId[0], nextPtsId[0], nextPtsId[1]);
			topo->addPolygon(addPtsId[0], nextPtsId[1], addPtsId[1]);
			topo->addPolygon(addPtsId[1], nextPtsId[1], nextPtsId[2]);
			topo->addPolygon(addPtsId[1], nextPtsId[2], addPtsId[2]);
			break;
		}
		} // switch

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


