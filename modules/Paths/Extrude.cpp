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

struct ExtrudeHelper
{
	ExtrudeHelper(const Path& pts, Mesh& mesh, Path& UV, int cap, int join)
		: pts(pts), mesh(mesh), UV(UV), cap(cap), join(join) {}
	void prepare(const Animation<PReal>& widthAnim);
	void startCap();
	void lines();
	void endCap();

	const Path& pts;
	Mesh& mesh;
	Path& UV;
	std::vector<Point> normals;
	std::vector<PReal> abscissa, halfWidth;
	int cap, join, nbPts;
	Mesh::PointID prevPtsId[3];
};

//****************************************************************************//

class PointListMath_Extrude : public PandaObject
{
public:
	PANDA_CLASS(PointListMath_Extrude, PandaObject)

	PointListMath_Extrude(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "List of control points"))
		, m_width(initData("width", "Width of the line"))
		, m_capStyle(initData("cap", "Style of the caps"))
		, m_joinStyle(initData("join", "Style of the joins"))
		, m_output(initData("output", "Mesh created from the extrusion"))
		, m_coordUV(initData("UV coords", "UV coords of the points in the mesh"))
	{
		addInput(m_input);
		addInput(m_width);
		addInput(m_capStyle);
		addInput(m_joinStyle);

		m_capStyle.setWidget("enum");
		m_capStyle.setWidgetData("Butt cap;Round cap;Square cap");
		m_joinStyle.setWidget("enum");
		m_joinStyle.setWidgetData("Miter join;Round join;Bevel join");

		addOutput(m_output);
		addOutput(m_coordUV);

		m_width.getAccessor()->add(0, 10.0);
	}

	void extrudePath(const Path& pts, Mesh& mesh, Path& UV)
	{
		const Animation<PReal>& widthAnim = m_width.getValue();
		int nbPts = pts.size();
		if(!nbPts || !widthAnim.size())
			return;

		ExtrudeHelper extrudeHelper{pts, mesh, UV, m_capStyle.getValue(), m_joinStyle.getValue()};
		extrudeHelper.prepare(widthAnim);
		if(extrudeHelper.abscissa.back() > 1e-3)
		{
			extrudeHelper.startCap();
			extrudeHelper.lines();
			extrudeHelper.endCap();
		}
	}

	void update()
	{
		const auto& input = m_input.getValue();
		const int nb = input.size();
		auto output = m_output.getAccessor();
		auto UV = m_coordUV.getAccessor();
		output.clear();
		output.resize(nb);
		UV.clear();
		UV.resize(nb);

		for(int i=0; i<nb; ++i)
			extrudePath(input[i], output[i], UV[i]);

		cleanDirty();
	}

protected:
	Data< QVector<Path> > m_input;
	Data< Animation<PReal> > m_width;
	Data< int > m_capStyle, m_joinStyle;
	Data< QVector<Mesh> > m_output;
	Data< QVector<Path> > m_coordUV; // Hack: vector of vector of vec2
};

//****************************************************************************//

void ExtrudeHelper::prepare(const Animation<PReal>& widthAnim)
{
	nbPts = pts.size();
	normals.resize(nbPts-1);
	abscissa.resize(nbPts);
	halfWidth.resize(nbPts);

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
	const PReal firstAbs = abscissa[0];
	const PReal realLength = length - firstAbs;

	for(int i=0; i<nbPts; ++i)
		halfWidth[i] = widthAnim.get((abscissa[i] - firstAbs) / realLength) / 2;

	if(cap) // add the width of the end cap to the total length
		length += widthAnim.get(1) / 2;
	if(length < 1e-3)
		return;
	for(auto& a : abscissa)
		a /= length;
}

void ExtrudeHelper::startCap()
{
	Point dir = normals[0] * halfWidth[0];
	prevPtsId[0] = mesh.addPoint(pts[0] + dir);
	prevPtsId[1] = mesh.addPoint(pts[0]);
	prevPtsId[2] = mesh.addPoint(pts[0] - dir);
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
			int nb = static_cast<int>(floor(halfWidth[0] * M_PI));
			PReal angle = M_PI / nb; // We do a half turn
			PReal ca = cos(angle), sa = sin(angle);
			Point center = pts[0];
			Point uvPt(0, 1), uvCenter(abscissa[0], 0.5), uvMult(-abscissa[0], 0.5);
			Mesh::PointID ptId = prevPtsId[0];
			for(int i=0; i<nb-1; ++i)
			{
				dir = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
				uvPt = Point(uvPt.x*ca+uvPt.y*sa, uvPt.y*ca-uvPt.x*sa);
				Mesh::PointID newPtId = mesh.addPoint(center + dir);
				UV.push_back(uvCenter + uvPt.linearProduct(uvMult));
				mesh.addTriangle(ptId, newPtId, prevPtsId[1]);

				ptId = newPtId;
			}
			mesh.addTriangle(ptId, prevPtsId[2], prevPtsId[1]);
			break;
		}
		case 2: // Square cap
		{
			Point dir2 = Point(dir.y, -dir.x);
			Mesh::PointID addPtsId[3];
			addPtsId[0] = mesh.addPoint(pts[0] + dir + dir2);
			addPtsId[1] = mesh.addPoint(pts[0]	     + dir2);
			addPtsId[2] = mesh.addPoint(pts[0] - dir + dir2);
			UV.push_back(Point(0, 1));
			UV.push_back(Point(0, 0.5));
			UV.push_back(Point(0, 0));

			mesh.addTriangle(prevPtsId[0], addPtsId[0], prevPtsId[1]);
			mesh.addTriangle(prevPtsId[1], addPtsId[0], addPtsId[1]);
			mesh.addTriangle(addPtsId[1], addPtsId[2], prevPtsId[1]);
			mesh.addTriangle(prevPtsId[1], addPtsId[2], prevPtsId[2]);
			break;
		}
	} // end switch
}

void ExtrudeHelper::lines()
{
	for(int i=1; i<nbPts-1; ++i)
	{
		PReal side = normals[i-1].cross(normals[i]);
		if(fabs(side) < 1e-3)	// Don't create a join (nor points) if these 2 segments are aligned
			continue;

		const PReal w = halfWidth[i];

		// Interior of the curvature
		Mesh::PointID nextPtsId[3];
		Point dir = normals[i-1] + normals[i];
		dir = w * (normals[i-1] / dir.dot(normals[i-1])
				+ normals[i] / dir.dot(normals[i]) );
		PReal norm2Dir = dir.norm2()
			 , norm2Seg1 = (pts[i]-pts[i-1]).norm2()
			 , norm2Seg2 = (pts[i+1]-pts[i]).norm2();
		bool hasInteriorPt = true;
	//	if(norm2Dir > norm2Seg1 && norm2Dir > norm2Seg2)
	//		hasInteriorPt = false;	// I don't know what to do in this degenerated case!

		// Main extrusion (without the exterior of the curvature)
		nextPtsId[1] = mesh.addPoint(pts[i]);
		UV.push_back(Point(abscissa[i], 0.5));
		mesh.addTriangle(nextPtsId[1], prevPtsId[0], prevPtsId[1]);
		mesh.addTriangle(nextPtsId[1], prevPtsId[1], prevPtsId[2]);

		if(side > 0)
		{
			if(hasInteriorPt)
			{
				nextPtsId[2] = mesh.addPoint(pts[i] - dir);
				UV.push_back(Point(abscissa[i], 0));
				mesh.addTriangle(nextPtsId[1], prevPtsId[2], nextPtsId[2]);
			}
			else
			{
				Mesh::PointID addPtId = mesh.addPoint(pts[i] - normals[i-1] * w);
				UV.push_back(Point(abscissa[i], 0));
				mesh.addTriangle(nextPtsId[1], prevPtsId[2], addPtId);
				nextPtsId[2] = mesh.addPoint(pts[i] - normals[i] * w);
				UV.push_back(Point(abscissa[i], 0));
			}

		}
		else // side < 0
		{
			if(hasInteriorPt)
			{
				nextPtsId[0] = mesh.addPoint(pts[i] + dir);
				UV.push_back(Point(abscissa[i], 1));
				mesh.addTriangle(nextPtsId[0], prevPtsId[0], nextPtsId[1]);
			}
			else
			{
				Mesh::PointID addPtId = mesh.addPoint(pts[i] + normals[i-1] * w);
				UV.push_back(Point(abscissa[i], 1));
				mesh.addTriangle(addPtId, prevPtsId[0], nextPtsId[1]);
				nextPtsId[0] = mesh.addPoint(pts[i] + normals[i] * w);
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
					nextPtsId[0] = mesh.addPoint(pts[i] + dir);
					UV.push_back(Point(abscissa[i], 1));
					mesh.addTriangle(nextPtsId[0], prevPtsId[0], nextPtsId[1]);
				}
				else // side < 0
				{
					nextPtsId[2] = mesh.addPoint(pts[i] - dir);
					UV.push_back(Point(abscissa[i], 0));
					mesh.addTriangle(nextPtsId[2], nextPtsId[1], prevPtsId[2]);
				}

				break;
			}
			case 1: // Round join
			{
				if(side > 0)
				{
					PReal angle = acos(normals[i-1].dot(normals[i]));
					Mesh::PointID addPtId = mesh.addPoint(pts[i] + normals[i-1] * w);
					nextPtsId[0] = mesh.addPoint(pts[i] + normals[i] * w);
					UV.push_back(Point(abscissa[i], 1));
					UV.push_back(Point(abscissa[i], 1));

					mesh.addTriangle(addPtId, prevPtsId[0], nextPtsId[1]);

					int nb = static_cast<int>(floor(w * angle));
					angle /= nb;
					PReal ca = cos(angle), sa = sin(angle);
					Point center = pts[i];
					Point r = normals[i] * w;
					Mesh::PointID ptId = nextPtsId[0];
					for(int j=0; j<nb-1; ++j)
					{
						Point nr = Point(r.x*ca+r.y*sa, r.y*ca-r.x*sa);
						Mesh::PointID newPtId = mesh.addPoint(center + nr);
						UV.push_back(Point(abscissa[i], 1));
						mesh.addTriangle(ptId, newPtId, nextPtsId[1]);

						r = nr;
						ptId = newPtId;
					}
					mesh.addTriangle(ptId, addPtId, nextPtsId[1]);
				}
				else // side < 0
				{
					PReal angle = acos(normals[i-1].dot(normals[i]));
					nextPtsId[2] = mesh.addPoint(pts[i] - normals[i] * w);
					Mesh::PointID addPtId = mesh.addPoint(pts[i] - normals[i-1] * w);
					UV.push_back(Point(abscissa[i], 0));
					UV.push_back(Point(abscissa[i], 0));

					mesh.addTriangle(addPtId, nextPtsId[1], prevPtsId[2]);

					int nb = static_cast<int>(floor(w * angle));
					angle /= nb;
					PReal ca = cos(angle), sa = sin(angle);
					Point center = pts[i];
					Point r = normals[i-1] * w;
					Mesh::PointID ptId = addPtId;
					for(int j=0; j<nb-1; ++j)
					{
						Point nr = Point(r.x*ca+r.y*sa, r.y*ca-r.x*sa);
						Mesh::PointID newPtId = mesh.addPoint(center - nr);
						UV.push_back(Point(abscissa[i], 0));
						mesh.addTriangle(ptId, newPtId, nextPtsId[1]);

						r = nr;
						ptId = newPtId;
					}
					mesh.addTriangle(ptId, nextPtsId[2], nextPtsId[1]);
				}
				break;
			}
			case 2: // Bevel join
			{
				if(side > 0)
				{
					Mesh::PointID addPtId = mesh.addPoint(pts[i] + normals[i-1] * w);
					nextPtsId[0] = mesh.addPoint(pts[i] + normals[i] * w);
					UV.push_back(Point(abscissa[i], 1));
					UV.push_back(Point(abscissa[i], 1));

					mesh.addTriangle(nextPtsId[0], addPtId, nextPtsId[1]);
					mesh.addTriangle(addPtId, prevPtsId[0], nextPtsId[1]);
				}
				else // side < 0
				{
					nextPtsId[2] = mesh.addPoint(pts[i] - normals[i] * w);
					Mesh::PointID addPtId = mesh.addPoint(pts[i] - normals[i-1] * w);
					UV.push_back(Point(abscissa[i], 0));
					UV.push_back(Point(abscissa[i], 0));

					mesh.addTriangle(nextPtsId[2], nextPtsId[1], addPtId);
					mesh.addTriangle(addPtId, nextPtsId[1], prevPtsId[2]);
				}
				break;
			}
		} // end switch

		for(int j=0; j<3; ++j)
			prevPtsId[j] = nextPtsId[j];
	}

	// Last segment
	const PReal w = halfWidth.back();
	Mesh::PointID nextPtsId[3];
	Point dir = normals[nbPts-2] * w;
	nextPtsId[0] = mesh.addPoint(pts[nbPts-1] + dir);
	nextPtsId[1] = mesh.addPoint(pts[nbPts-1]);
	nextPtsId[2] = mesh.addPoint(pts[nbPts-1] - dir);
	UV.push_back(Point(abscissa[nbPts-1], 1));
	UV.push_back(Point(abscissa[nbPts-1], 0.5));
	UV.push_back(Point(abscissa[nbPts-1], 0));
	mesh.addTriangle(nextPtsId[0], prevPtsId[0], nextPtsId[1]);
	mesh.addTriangle(prevPtsId[0], prevPtsId[1], nextPtsId[1]);
	mesh.addTriangle(nextPtsId[1], prevPtsId[1], prevPtsId[2]);
	mesh.addTriangle(nextPtsId[1], prevPtsId[2], nextPtsId[2]);

	for(int j=0; j<3; ++j)
		prevPtsId[j] = nextPtsId[j];
}

void ExtrudeHelper::endCap()
{
	switch(cap)
	{
		default:
		case 0:	// Butt cap (nothing to do)
			break;
		case 1:	// Round cap
		{
			const PReal w = halfWidth.back();
			int nb = static_cast<int>(floor(w * M_PI));
			PReal angle = M_PI / nb; // We do a half turn
			PReal ca = cos(angle), sa = sin(angle);
			Point center = pts[nbPts-1];
			Point dir = normals[nbPts-2] * w;
			dir = -dir;
			Mesh::PointID ptId = prevPtsId[2];
			Point uvPt(0, 1), uvCenter(abscissa[nbPts-1], 0.5), uvMult(abscissa[0], -0.5);
			for(int i=0; i<nb-1; ++i)
			{
				dir = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
				uvPt = Point(uvPt.x*ca+uvPt.y*sa, uvPt.y*ca-uvPt.x*sa);
				Mesh::PointID newPtId = mesh.addPoint(center + dir);
				UV.push_back(uvCenter + uvPt.linearProduct(uvMult));
				mesh.addTriangle(ptId, newPtId, prevPtsId[1]);

				ptId = newPtId;
			}
			mesh.addTriangle(ptId, prevPtsId[0], prevPtsId[1]);
			break;
		}
		case 2: // Square cap
		{
			const PReal w = halfWidth.back();
			Point dir = normals[nbPts-2] * w;
			Point dir2 = Point(dir.y, -dir.x);
			Mesh::PointID addPtsId[3];
			addPtsId[0] = mesh.addPoint(pts[nbPts-1] + dir - dir2);
			addPtsId[1] = mesh.addPoint(pts[nbPts-1]	   - dir2);
			addPtsId[2] = mesh.addPoint(pts[nbPts-1] - dir - dir2);
			UV.push_back(Point(1, 1));
			UV.push_back(Point(1, 0.5));
			UV.push_back(Point(1, 0));

			mesh.addTriangle(addPtsId[0], prevPtsId[0], addPtsId[1]);
			mesh.addTriangle(prevPtsId[0], prevPtsId[1], addPtsId[1]);
			mesh.addTriangle(addPtsId[1], prevPtsId[1], prevPtsId[2]);
			mesh.addTriangle(addPtsId[1], prevPtsId[2], addPtsId[2]);
			break;
		}
	}
}

//****************************************************************************//

int PointListMath_ExtrudeClass = RegisterObject<PointListMath_Extrude>("Math/Path/Extrude")
		.setDescription("Add width to a line");

} // namespace Panda


