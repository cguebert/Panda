#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Mesh.h>
#include <panda/types/Animation.h>
#include <panda/types/Path.h>

#include <cmath>

namespace panda {

using types::Point;
using types::Mesh;
using types::Animation;
using types::Path;

class PointListMath_Extrude : public PandaObject
{
public:
	PANDA_CLASS(PointListMath_Extrude, PandaObject)

	PointListMath_Extrude(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "List of control points"))		
		, width(initData(&width, "width", "Width of the line"))
		, capStyle(initData(&capStyle, "cap", "Style of the caps"))
		, joinStyle(initData(&joinStyle, "join", "Style of the joins"))
		, output(initData(&output, "output", "Mesh created from the extrusion"))
		, coordUV(initData(&coordUV, "UV coords", "UV coords of the points in the mesh"))
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
		addOutput(&coordUV);

		width.getAccessor()->add(0, 10.0);
	}

	void update()
	{
		const QVector<Point>& pts = input.getValue();
		const Animation<PReal>& widthAnim = width.getValue();
		PReal w = widthAnim.get(0) / 2;
		int cap = capStyle.getValue();
		int join = joinStyle.getValue();
		auto mesh = output.getAccessor();
		auto UV = coordUV.getAccessor();
		mesh->clear();
		UV.clear();

		int nbPts = pts.size();
		if(!nbPts || !widthAnim.size())
			return;

		QVector<Point> normals;
		QVector<PReal> abscissa;
		normals.resize(nbPts-1);
		abscissa.resize(nbPts);
		abscissa[0] = 0;
		if(cap) // add the width of the start cap to the first abscissa
			abscissa[0] = widthAnim.get(0) / 2;
		for(int i=0; i<nbPts-1; ++i)
		{
			const Point &pt1=pts[i], &pt2=pts[i+1];
			normals[i] = Point(pt2.y-pt1.y, pt1.x-pt2.x).normalized();
			abscissa[i+1] = abscissa[i] + (pt2-pt1).norm();
		}
		PReal length = abscissa.back();
		if(cap) // add the width of the end cap to the total length
			length += widthAnim.get(1) / 2;
		if(length < 1e-3)
			return;
		for(auto& a : abscissa)
			a /= length;

		// Start cap
		Mesh::PointID prevPtsId[3];
		Point dir = normals[0] * w;
		prevPtsId[0] = mesh->addPoint(pts[0] + dir);
		prevPtsId[1] = mesh->addPoint(pts[0]);
		prevPtsId[2] = mesh->addPoint(pts[0] - dir);
		UV.push_back(Point(abscissa[0], 1));
		UV.push_back(Point(abscissa[0], 0.5));
		UV.push_back(Point(abscissa[0], 0));
		switch(cap)
		{
		default:
		case 0:	// Butt cap (nothing to do)
			break;
		case 1:	// Round cap
		{
			int nb = static_cast<int>(floor(w * M_PI));
			PReal angle = M_PI / nb; // We do a half turn
			PReal ca = cos(angle), sa = sin(angle);
			Point center = pts[0];
			Point uvPt(0, 1), uvCenter(abscissa[0], 0.5), uvMult(-abscissa[0], 0.5);
			Mesh::PointID ptId = prevPtsId[0];
			for(int i=0; i<nb-1; ++i)
			{
				dir = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
				uvPt = Point(uvPt.x*ca+uvPt.y*sa, uvPt.y*ca-uvPt.x*sa);
				Mesh::PointID newPtId = mesh->addPoint(center + dir);
				UV.push_back(uvCenter + uvPt.linearProduct(uvMult));
				mesh->addTriangle(ptId, newPtId, prevPtsId[1]);

				ptId = newPtId;
			}
			mesh->addTriangle(ptId, prevPtsId[2], prevPtsId[1]);
			break;
		}
		case 2: // Square cap
		{
			Point dir2 = Point(dir.y, -dir.x);
			Mesh::PointID addPtsId[3];
			addPtsId[0] = mesh->addPoint(pts[0] + dir + dir2);
			addPtsId[1] = mesh->addPoint(pts[0]	      + dir2);
			addPtsId[2] = mesh->addPoint(pts[0] - dir + dir2);
			UV.push_back(Point(0, 1));
			UV.push_back(Point(0, 0.5));
			UV.push_back(Point(0, 0));

			mesh->addTriangle(prevPtsId[0], addPtsId[0], prevPtsId[1]);
			mesh->addTriangle(prevPtsId[1], addPtsId[0], addPtsId[1]);
			mesh->addTriangle(addPtsId[1], addPtsId[2], prevPtsId[1]);
			mesh->addTriangle(prevPtsId[1], addPtsId[2], prevPtsId[2]);
			break;
		}
		} // end switch

		// Line
		for(int i=1; i<nbPts-1; ++i)
		{
			PReal side = normals[i-1].cross(normals[i]);
			if(fabs(side) < 1e-3)	// Don't create a join (nor points) if these 2 segments are aligned
				continue;

			w = widthAnim.get(abscissa[i]) / 2;

			// Interior of the curvature
			Mesh::PointID nextPtsId[3];
			dir = normals[i-1] + normals[i];
			dir = w * (normals[i-1] / dir.dot(normals[i-1])
				+ normals[i] / dir.dot(normals[i]) );
			PReal norm2Dir = dir.norm2()
				 , norm2Seg1 = (pts[i]-pts[i-1]).norm2()
				 , norm2Seg2 = (pts[i+1]-pts[i]).norm2();
			bool hasInteriorPt = true;
			if(norm2Dir > norm2Seg1 || norm2Dir > norm2Seg2)
				hasInteriorPt = false;	// I don't know what to do in this degenerated case!

			// Main extrusion (without the exterior of the curvature)
			nextPtsId[1] = mesh->addPoint(pts[i]);
			UV.push_back(Point(abscissa[i], 0.5));
			mesh->addTriangle(nextPtsId[1], prevPtsId[0], prevPtsId[1]);
			mesh->addTriangle(nextPtsId[1], prevPtsId[1], prevPtsId[2]);

			if(side > 0)
			{
				if(hasInteriorPt)
				{
					nextPtsId[2] = mesh->addPoint(pts[i] - dir);
					UV.push_back(Point(abscissa[i], 0));
					mesh->addTriangle(nextPtsId[1], prevPtsId[2], nextPtsId[2]);
				}
				else
				{
					Mesh::PointID addPtId = mesh->addPoint(pts[i] - normals[i-1] * w);
					UV.push_back(Point(abscissa[i], 0));
					mesh->addTriangle(nextPtsId[1], prevPtsId[2], addPtId);
					nextPtsId[2] = mesh->addPoint(pts[i] - normals[i] * w);
					UV.push_back(Point(abscissa[i], 0));
				}

			}
			else // side < 0
			{
				if(hasInteriorPt)
				{
					nextPtsId[0] = mesh->addPoint(pts[i] + dir);
					UV.push_back(Point(abscissa[i], 1));
					mesh->addTriangle(nextPtsId[0], prevPtsId[0], nextPtsId[1]);
				}
				else
				{
					Mesh::PointID addPtId = mesh->addPoint(pts[i] + normals[i-1] * w);
					UV.push_back(Point(abscissa[i], 1));
					mesh->addTriangle(addPtId, prevPtsId[0], nextPtsId[1]);
					nextPtsId[0] = mesh->addPoint(pts[i] + normals[i] * w);
					UV.push_back(Point(abscissa[i], 1));
				}
			}

			 // Do a Bevel join instead of a miter join if the angle is too small
			int tmpJoin = join;
			if(normals[i-1].dot(normals[i]) < -0.9 && join == 0)
				tmpJoin = 2;

			// Join
			switch(tmpJoin)
			{
			default:
			case 0: // Miter join
			{
				if(side > 0)
				{
					nextPtsId[0] = mesh->addPoint(pts[i] + dir);
					UV.push_back(Point(abscissa[i], 1));
					mesh->addTriangle(nextPtsId[0], prevPtsId[0], nextPtsId[1]);
				}
				else // side < 0
				{
					nextPtsId[2] = mesh->addPoint(pts[i] - dir);
					UV.push_back(Point(abscissa[i], 0));
					mesh->addTriangle(nextPtsId[2], nextPtsId[1], prevPtsId[2]);
				}

				break;
			}
			case 1: // Round join
			{
				if(side > 0)
				{
					PReal angle = acos(normals[i-1].dot(normals[i]));
					Mesh::PointID addPtId = mesh->addPoint(pts[i] + normals[i-1] * w);
					nextPtsId[0] = mesh->addPoint(pts[i] + normals[i] * w);
					UV.push_back(Point(abscissa[i], 1));
					UV.push_back(Point(abscissa[i], 1));

					mesh->addTriangle(addPtId, prevPtsId[0], nextPtsId[1]);

					int nb = static_cast<int>(floor(w * angle));
					angle /= nb;
					PReal ca = cos(angle), sa = sin(angle);
					Point center = pts[i];
					Point r = normals[i] * w;
					Mesh::PointID ptId = nextPtsId[0];
					for(int j=0; j<nb-1; ++j)
					{
						Point nr = Point(r.x*ca+r.y*sa, r.y*ca-r.x*sa);
						Mesh::PointID newPtId = mesh->addPoint(center + nr);
						UV.push_back(Point(abscissa[i], 1));
						mesh->addTriangle(ptId, newPtId, nextPtsId[1]);

						r = nr;
						ptId = newPtId;
					}
					mesh->addTriangle(ptId, addPtId, nextPtsId[1]);
				}
				else // side < 0
				{
					PReal angle = acos(normals[i-1].dot(normals[i]));
					nextPtsId[2] = mesh->addPoint(pts[i] - normals[i] * w);
					Mesh::PointID addPtId = mesh->addPoint(pts[i] - normals[i-1] * w);
					UV.push_back(Point(abscissa[i], 0));
					UV.push_back(Point(abscissa[i], 0));

					mesh->addTriangle(addPtId, nextPtsId[1], prevPtsId[2]);

					int nb = static_cast<int>(floor(w * angle));
					angle /= nb;
					PReal ca = cos(angle), sa = sin(angle);
					Point center = pts[i];
					Point r = normals[i-1] * w;
					Mesh::PointID ptId = addPtId;
					for(int j=0; j<nb-1; ++j)
					{
						Point nr = Point(r.x*ca+r.y*sa, r.y*ca-r.x*sa);
						Mesh::PointID newPtId = mesh->addPoint(center - nr);
						UV.push_back(Point(abscissa[i], 0));
						mesh->addTriangle(ptId, newPtId, nextPtsId[1]);

						r = nr;
						ptId = newPtId;
					}
					mesh->addTriangle(ptId, nextPtsId[2], nextPtsId[1]);
				}
				break;
			}
			case 2: // Bevel join
			{
				if(side > 0)
				{
					Mesh::PointID addPtId = mesh->addPoint(pts[i] + normals[i-1] * w);
					nextPtsId[0] = mesh->addPoint(pts[i] + normals[i] * w);
					UV.push_back(Point(abscissa[i], 1));
					UV.push_back(Point(abscissa[i], 1));

					mesh->addTriangle(nextPtsId[0], addPtId, nextPtsId[1]);
					mesh->addTriangle(addPtId, prevPtsId[0], nextPtsId[1]);
				}
				else // side < 0
				{
					nextPtsId[2] = mesh->addPoint(pts[i] - normals[i] * w);
					Mesh::PointID addPtId = mesh->addPoint(pts[i] - normals[i-1] * w);
					UV.push_back(Point(abscissa[i], 0));
					UV.push_back(Point(abscissa[i], 0));

					mesh->addTriangle(nextPtsId[2], nextPtsId[1], addPtId);
					mesh->addTriangle(addPtId, nextPtsId[1], prevPtsId[2]);
				}
				break;
			}
			} // end switch

			for(int j=0; j<3; ++j)
				prevPtsId[j] = nextPtsId[j];
		}

		// Last segment
		w = widthAnim.get(abscissa[nbPts-1]) / 2;
		Mesh::PointID nextPtsId[3];
		dir = normals[nbPts-2] * w;
		nextPtsId[0] = mesh->addPoint(pts[nbPts-1] + dir);
		nextPtsId[1] = mesh->addPoint(pts[nbPts-1]);
		nextPtsId[2] = mesh->addPoint(pts[nbPts-1] - dir);
		UV.push_back(Point(abscissa[nbPts-1], 1));
		UV.push_back(Point(abscissa[nbPts-1], 0.5));
		UV.push_back(Point(abscissa[nbPts-1], 0));
		mesh->addTriangle(nextPtsId[0], prevPtsId[0], nextPtsId[1]);
		mesh->addTriangle(prevPtsId[0], prevPtsId[1], nextPtsId[1]);
		mesh->addTriangle(nextPtsId[1], prevPtsId[1], prevPtsId[2]);
		mesh->addTriangle(nextPtsId[1], prevPtsId[2], nextPtsId[2]);

		// End cap
		switch(cap)
		{
		default:
		case 0:	// Butt cap (nothing to do)
			break;
		case 1:	// Round cap
		{
			int nb = static_cast<int>(floor(w * M_PI));
			PReal angle = M_PI / nb; // We do a half turn
			PReal ca = cos(angle), sa = sin(angle);
			Point center = pts[nbPts-1];
			dir = -dir;
			Mesh::PointID ptId = nextPtsId[2];
			Point uvPt(0, 1), uvCenter(abscissa[nbPts-1], 0.5), uvMult(abscissa[0], -0.5);
			for(int i=0; i<nb-1; ++i)
			{
				dir = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
				uvPt = Point(uvPt.x*ca+uvPt.y*sa, uvPt.y*ca-uvPt.x*sa);
				Mesh::PointID newPtId = mesh->addPoint(center + dir);
				UV.push_back(uvCenter + uvPt.linearProduct(uvMult));
				mesh->addTriangle(ptId, newPtId, nextPtsId[1]);

				ptId = newPtId;
			}
			mesh->addTriangle(ptId, nextPtsId[0], nextPtsId[1]);
			break;
		}
		case 2: // Square cap
		{
			Point dir2 = Point(dir.y, -dir.x);
			Mesh::PointID addPtsId[3];
			addPtsId[0] = mesh->addPoint(pts[nbPts-1] + dir - dir2);
			addPtsId[1] = mesh->addPoint(pts[nbPts-1]	    - dir2);
			addPtsId[2] = mesh->addPoint(pts[nbPts-1] - dir - dir2);
			UV.push_back(Point(1, 1));
			UV.push_back(Point(1, 0.5));
			UV.push_back(Point(1, 0));

			mesh->addTriangle(addPtsId[0], nextPtsId[0], addPtsId[1]);
			mesh->addTriangle(nextPtsId[0], nextPtsId[1], addPtsId[1]);
			mesh->addTriangle(addPtsId[1], nextPtsId[1], nextPtsId[2]);
			mesh->addTriangle(addPtsId[1], nextPtsId[2], addPtsId[2]);
			break;
		}
		} // end switch

		cleanDirty();
	}

protected:
	Data< Path > input;
	Data< Animation<PReal> > width;
	Data< int > capStyle, joinStyle;
	Data< Mesh > output;
	Data< QVector<Point> > coordUV;
};

int PointListMath_ExtrudeClass = RegisterObject<PointListMath_Extrude>("Math/Path/Extrude")
		.setDescription("Add width to a line");

} // namespace Panda


