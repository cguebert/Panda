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
			for(int i=0; i<nb-1; ++i)
			{
				QPointF pt = QPointF(dir.x()*ca+dir.y()*sa, dir.y()*ca-dir.x()*sa);
				Topology::PointID newPtId = topo->addPoint(center + pt);
				topo->addPolygon(ptId, newPtId, prevPtsId[1]);

				dir = pt;
				ptId = newPtId;
			}
			topo->addPolygon(ptId, prevPtsId[2], prevPtsId[1]);
			break;
		}
		case 2: // Square cap
		{
			QPointF dir2 = QPointF(dir.y(), -dir.x());
			Topology::PointID addPtsId[3];
			addPtsId[0] = topo->addPoint(pts[0] + dir + dir2);
			addPtsId[1] = topo->addPoint(pts[0]	      + dir2);
			addPtsId[2] = topo->addPoint(pts[0] - dir + dir2);

			topo->addPolygon(prevPtsId[0], addPtsId[0], prevPtsId[1]);
			topo->addPolygon(prevPtsId[1], addPtsId[0], addPtsId[1]);
			topo->addPolygon(addPtsId[1], addPtsId[2], prevPtsId[1]);
			topo->addPolygon(prevPtsId[1], addPtsId[2], prevPtsId[2]);
			break;
		}
		} // end switch

		// Line
		for(int i=1; i<nbPts-1; ++i)
		{
			double side = helper::cross(normals[i-1], normals[i]);
			if(fabs(side) < 1e-3)	// Don't create a join (nor points) if these 2 segments are aligned
				continue;

			// Interior of the curvature
			Topology::PointID nextPtsId[3];
			dir = normals[i-1] + normals[i];
			dir = w * (normals[i-1] / helper::dot(dir, normals[i-1])
				+ normals[i] / helper::dot(dir, normals[i]) );
			double norm2Dir = helper::norm2(dir)
				 , norm2Seg1 = helper::norm2(pts[i]-pts[i-1])
				 , norm2Seg2 = helper::norm2(pts[i+1]-pts[i]);
			bool hasInteriorPt = true;
			if(norm2Dir > norm2Seg1 || norm2Dir > norm2Seg2)
				hasInteriorPt = false;	// I don't know what to do in this degenerated case!

			// Main extrusion (without the exterior of the curvature)
			if(side > 0)
			{
				nextPtsId[1] = topo->addPoint(pts[i]);
				topo->addPolygon(nextPtsId[1], prevPtsId[1], prevPtsId[2]);
				topo->addPolygon(nextPtsId[1], prevPtsId[0], prevPtsId[1]);

				if(hasInteriorPt)
				{
					nextPtsId[2] = topo->addPoint(pts[i] - dir);
					topo->addPolygon(nextPtsId[1], prevPtsId[2], nextPtsId[2]);
				}
				else
				{
					Topology::PointID addPtId = topo->addPoint(pts[i] - normals[i-1] * w);
					topo->addPolygon(nextPtsId[1], prevPtsId[2], addPtId);
					nextPtsId[2] = topo->addPoint(pts[i] - normals[i] * w);
				}

			}
			else // side < 0
			{
				nextPtsId[1] = topo->addPoint(pts[i]);
				topo->addPolygon(prevPtsId[0], prevPtsId[1], nextPtsId[1]);
				topo->addPolygon(nextPtsId[1], prevPtsId[1], prevPtsId[2]);

				if(hasInteriorPt)
				{
					nextPtsId[0] = topo->addPoint(pts[i] + dir);
					topo->addPolygon(nextPtsId[0], prevPtsId[0], nextPtsId[1]);
				}
				else
				{
					Topology::PointID addPtId = topo->addPoint(pts[i] + normals[i-1] * w);
					topo->addPolygon(addPtId, prevPtsId[0], nextPtsId[1]);
					nextPtsId[0] = topo->addPoint(pts[i] + normals[i] * w);
				}
			}

			 // Do a Bevel join instead of a miter join if the angle is too small
			int tmpJoin = join;
			if(helper::dot(normals[i-1], normals[i]) < -0.9 && join == 0)
				tmpJoin = 2;

			// Join
			switch(tmpJoin)
			{
			default:
			case 0: // Miter join
			{
				if(side > 0)
				{
					nextPtsId[0] = topo->addPoint(pts[i] + dir);
					topo->addPolygon(nextPtsId[0], prevPtsId[0], nextPtsId[1]);
				}
				else // side < 0
				{
					nextPtsId[2] = topo->addPoint(pts[i] - dir);
					topo->addPolygon(nextPtsId[2], nextPtsId[1], prevPtsId[2]);
				}

				break;
			}
			case 1: // Round join
			{
				if(side > 0)
				{
					double angle = acos(helper::dot(normals[i-1], normals[i]));
					Topology::PointID addPtId = topo->addPoint(pts[i] + normals[i-1] * w);
					nextPtsId[0] = topo->addPoint(pts[i] + normals[i] * w);

					topo->addPolygon(addPtId, prevPtsId[0], nextPtsId[1]);

					int nb = static_cast<int>(floor(w * angle));
					angle /= nb;
					double ca = cos(angle), sa = sin(angle);
					QPointF center = pts[i];
					QPointF r = normals[i] * w;
					Topology::PointID ptId = nextPtsId[0];
					for(int j=0; j<nb-1; ++j)
					{
						QPointF nr = QPointF(r.x()*ca+r.y()*sa, r.y()*ca-r.x()*sa);
						Topology::PointID newPtId = topo->addPoint(center + nr);
						topo->addPolygon(ptId, newPtId, nextPtsId[1]);

						r = nr;
						ptId = newPtId;
					}
					topo->addPolygon(ptId, addPtId, nextPtsId[1]);
				}
				else // side < 0
				{
					double angle = acos(helper::dot(normals[i-1], normals[i]));
					nextPtsId[2] = topo->addPoint(pts[i] - normals[i] * w);
					Topology::PointID addPtId = topo->addPoint(pts[i] - normals[i-1] * w);

					topo->addPolygon(addPtId, nextPtsId[1], prevPtsId[2]);

					int nb = static_cast<int>(floor(w * angle));
					angle /= nb;
					double ca = cos(angle), sa = sin(angle);
					QPointF center = pts[i];
					QPointF r = normals[i-1] * w;
					Topology::PointID ptId = addPtId;
					for(int j=0; j<nb-1; ++j)
					{
						QPointF nr = QPointF(r.x()*ca+r.y()*sa, r.y()*ca-r.x()*sa);
						Topology::PointID newPtId = topo->addPoint(center - nr);
						topo->addPolygon(ptId, newPtId, nextPtsId[1]);

						r = nr;
						ptId = newPtId;
					}
					topo->addPolygon(ptId, nextPtsId[2], nextPtsId[1]);
				}
				break;
			}
			case 2: // Bevel join
			{
				if(side > 0)
				{
					Topology::PointID addPtId = topo->addPoint(pts[i] + normals[i-1] * w);
					nextPtsId[0] = topo->addPoint(pts[i] + normals[i] * w);

					topo->addPolygon(nextPtsId[0], addPtId, nextPtsId[1]);
					topo->addPolygon(addPtId, prevPtsId[0], nextPtsId[1]);
				}
				else // side < 0
				{
					nextPtsId[2] = topo->addPoint(pts[i] - normals[i] * w);
					Topology::PointID addPtId = topo->addPoint(pts[i] - normals[i-1] * w);

					topo->addPolygon(nextPtsId[2], nextPtsId[1], addPtId);
					topo->addPolygon(addPtId, nextPtsId[1], prevPtsId[2]);
				}
				break;
			}
			} // end switch

			for(int j=0; j<3; ++j)
				prevPtsId[j] = nextPtsId[j];
		}

		// Last segment
		Topology::PointID nextPtsId[3];
		dir = normals[nbPts-2] * w;
		nextPtsId[0] = topo->addPoint(pts[nbPts-1] + dir);
		nextPtsId[1] = topo->addPoint(pts[nbPts-1]);
		nextPtsId[2] = topo->addPoint(pts[nbPts-1] - dir);
		topo->addPolygon(nextPtsId[0], prevPtsId[0], nextPtsId[1]);
		topo->addPolygon(prevPtsId[0], prevPtsId[1], nextPtsId[1]);
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
			for(int i=0; i<nb-1; ++i)
			{
				QPointF pt = QPointF(dir.x()*ca+dir.y()*sa, dir.y()*ca-dir.x()*sa);
				Topology::PointID newPtId = topo->addPoint(center + pt);
				topo->addPolygon(ptId, newPtId, nextPtsId[1]);

				dir = pt;
				ptId = newPtId;
			}
			topo->addPolygon(ptId, nextPtsId[0], nextPtsId[1]);
			break;
		}
		case 2: // Square cap
		{
			QPointF dir2 = QPointF(dir.y(), -dir.x());
			Topology::PointID addPtsId[3];
			addPtsId[0] = topo->addPoint(pts[nbPts-1] + dir - dir2);
			addPtsId[1] = topo->addPoint(pts[nbPts-1]	    - dir2);
			addPtsId[2] = topo->addPoint(pts[nbPts-1] - dir - dir2);

			topo->addPolygon(addPtsId[0], nextPtsId[0], addPtsId[1]);
			topo->addPolygon(nextPtsId[0], nextPtsId[1], addPtsId[1]);
			topo->addPolygon(addPtsId[1], nextPtsId[1], nextPtsId[2]);
			topo->addPolygon(addPtsId[1], nextPtsId[2], addPtsId[2]);
			break;
		}
		} // end switch

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


