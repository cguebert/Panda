#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Mesh.h>
#include <panda/types/Animation.h>
#include <panda/types/Path.h>

#include <cmath>

namespace panda {

using types::Point;
using types::Mesh;
using types::Animation;
using types::Path;

class ExtrudeHelper
{
public:
	static void extrudePath(const Path& pts, Mesh& mesh, Path& UV,
							const Animation<float>& widthAnim,
							int capType, int joinType, bool close);

private:
	ExtrudeHelper(const Path& pts, Mesh& mesh, Path& UV, int capType, int joinType, bool close);
	void extrude();
	void prepare(const Animation<float>& widthAnim);
	void startCap();
	void lines();
	std::pair<bool, Point> computeInternalDir(int id);
	void segment(Mesh::PointID nextPtsId[3], int id, Point dir, float side, bool hasInternalPoint);
	void join(Mesh::PointID nextPtsId[3], int id, Point dir, float side);
	void lastSegment();
	void endCap();

	const Path& pts;
	Mesh& mesh;
	Path& UV;
	std::vector<Point> normals;
	std::vector<float> abscissa, lengths, halfWidths;
	int capType, joinType, nbPts;
	bool closePath;
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
		, m_closePath(initData(true, "close", "Close the path if possible"))
		, m_output(initData("output", "Mesh created from the extrusion"))
		, m_coordUV(initData("UV coords", "UV coords of the points in the mesh"))
	{
		addInput(m_input);
		addInput(m_width);
		addInput(m_capStyle);
		addInput(m_joinStyle);
		addInput(m_closePath);

		m_capStyle.setWidget("enum");
		m_capStyle.setWidgetData("Butt cap;Round cap;Square cap");
		m_joinStyle.setWidget("enum");
		m_joinStyle.setWidgetData("Miter join;Round join;Bevel join");
		m_closePath.setWidget("checkbox");

		addOutput(m_output);
		addOutput(m_coordUV);

		m_width.getAccessor()->add(0, 10.0);
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

		const Animation<float>& widthAnim = m_width.getValue();
		const auto cap = m_capStyle.getValue();
		const auto join = m_joinStyle.getValue();
		bool close = m_closePath.getValue() != 0;

		for(int i=0; i<nb; ++i)
		{
			int nbPts = input[i].size();
			if(!nbPts || !widthAnim.size())
				return;

			ExtrudeHelper::extrudePath(input[i], output[i], UV[i], widthAnim, cap, join, close);
		}
	}

protected:
	Data< std::vector<Path> > m_input;
	Data< Animation<float> > m_width;
	Data< int > m_capStyle, m_joinStyle, m_closePath;
	Data< std::vector<Mesh> > m_output;
	Data< std::vector<Path> > m_coordUV; // Hack: vector of vector of vec2
};

//****************************************************************************//

ExtrudeHelper::ExtrudeHelper(const Path& pts, Mesh& mesh, Path& UV, int cap, int join, bool close)
	: pts(pts)
	, mesh(mesh)
	, UV(UV)
	, capType(cap)
	, joinType(join)
	, closePath(close)
{ }

void ExtrudeHelper::extrudePath(const Path& pts, Mesh& mesh, Path& UV, const Animation<float>& widthAnim,int cap, int join, bool close)
{
	ExtrudeHelper helper{pts, mesh, UV, cap, join, close};
	helper.prepare(widthAnim);
	helper.extrude();
}

void ExtrudeHelper::extrude()
{
	if(abscissa.back() < 1e-3)
		return;

	startCap();
	lines();
	lastSegment();
	endCap();
}

void ExtrudeHelper::prepare(const Animation<float>& widthAnim)
{
	nbPts = pts.size();
	normals.resize(nbPts - 1);
	lengths.resize(nbPts - 1);
	abscissa.resize(nbPts);
	halfWidths.resize(nbPts);

	abscissa[0] = 0;
	if(capType) // add the width of the start cap to the first abscissa
		abscissa[0] = widthAnim.get(0) / 2;
	for(int i=0; i<nbPts-1; ++i)
	{
		const Point &pt1=pts[i], &pt2=pts[i+1];
		normals[i] = Point(pt2.y-pt1.y, pt1.x-pt2.x).normalized();
		lengths[i] = (pt2-pt1).norm();
		abscissa[i+1] = abscissa[i] + lengths[i];
	}
	float length = abscissa.back();
	const float firstAbs = abscissa[0];
	const float realLength = length - firstAbs;

	for(int i=0; i<nbPts; ++i)
		halfWidths[i] = widthAnim.get((abscissa[i] - firstAbs) / realLength) / 2;

	if(capType) // add the width of the end cap to the total length
		length += widthAnim.get(1) / 2;
	if(length < 1e-3)
		return;
	for(auto& a : abscissa)
		a /= length;

	if(closePath)
	{
		if(pts.front() != pts.back() || halfWidths.front() != halfWidths.back())
			closePath = false;
		if(closePath)
		{
			capType = 0;
			normals.push_back(normals.front());
		}
	}

	if (!closePath)
		normals.push_back(normals.back());
}

void ExtrudeHelper::startCap()
{
	Point dir = normals[0] * halfWidths[0];
	const Point currentPt = pts[0];
	const float currentAbs = abscissa[0];
	prevPtsId[0] = mesh.addPoint(currentPt + dir);
	prevPtsId[1] = mesh.addPoint(currentPt);
	prevPtsId[2] = mesh.addPoint(currentPt - dir);
	UV.push_back(Point(currentAbs, 1));
	UV.push_back(Point(currentAbs, 0.5));
	UV.push_back(Point(currentAbs, 0));
	switch(capType)
	{
		default:
		case 0:	// Butt cap (nothing to do)
			break;
		case 1:	// Round cap
		{
			int nb = static_cast<int>(floor(halfWidths[0] * M_PI));
			float angle = static_cast<float>(M_PI) / nb; // We do a half turn
			float ca = cos(angle), sa = sin(angle);
			Point uvPt(0, 1), uvCenter(currentAbs, 0.5), uvMult(-currentAbs, 0.5);
			Mesh::PointID ptId = prevPtsId[0];
			for(int i=0; i<nb-1; ++i)
			{
				dir = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
				uvPt = Point(uvPt.x*ca+uvPt.y*sa, uvPt.y*ca-uvPt.x*sa);
				Mesh::PointID newPtId = mesh.addPoint(currentPt + dir);
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
			addPtsId[0] = mesh.addPoint(currentPt + dir + dir2);
			addPtsId[1] = mesh.addPoint(currentPt       + dir2);
			addPtsId[2] = mesh.addPoint(currentPt - dir + dir2);
			UV.push_back(Point(0, 1));
			UV.push_back(Point(0, 0.5));
			UV.push_back(Point(0, 0));

			mesh.addTriangle(prevPtsId[0], addPtsId[0], prevPtsId[1]);
			mesh.addTriangle(prevPtsId[1], addPtsId[0], addPtsId[1]);
			mesh.addTriangle(addPtsId[1],  addPtsId[2], prevPtsId[1]);
			mesh.addTriangle(prevPtsId[1], addPtsId[2], prevPtsId[2]);
			break;
		}
	} // end switch
}

void ExtrudeHelper::lines()
{
	for(int id=1; id<nbPts-1; ++id)
	{
		const Point curNor = normals[id], prevNor = normals[id-1];
		float side = prevNor.cross(curNor);
		if(fabs(side) < 1e-3 && prevNor * curNor > 0)	// Don't create a join (nor points) if these 2 segments are aligned
			continue;

		Mesh::PointID nextPtsId[3];
		auto internal = computeInternalDir(id);
		segment(nextPtsId, id, internal.second, side, internal.first);
		join(nextPtsId, id, internal.second, side);

		for(int j=0; j<3; ++j)
			prevPtsId[j] = nextPtsId[j];
	}
}

std::pair<bool, Point> ExtrudeHelper::computeInternalDir(int id)
{
	const Point curNor = normals[id], prevNor = normals[id-1];
	if(prevNor.dot(curNor) < -0.999)
		return std::make_pair(false, Point());

	const float maxLen = std::min(lengths[id % (nbPts-1)], lengths[id-1]);
	Point dir = prevNor + curNor;
	dir = prevNor / dir.dot(prevNor) + curNor / dir.dot(curNor);
	dir *= halfWidths[id];
	if(prevNor.dot(curNor) < -0.9 && dir.norm2() > maxLen*maxLen)
		return std::make_pair(false, dir);

	return std::make_pair(true, dir);
}

void ExtrudeHelper::segment(Mesh::PointID nextPtsId[3], int id, Point dir, float side, bool hasInternalPoint)
{
	const float halfWidth = halfWidths[id], curAbs = abscissa[id];
	const Point curPt = pts[id], curNor = normals[id], prevNor = normals[id-1];

	// Main extrusion (without the exterior of the curvature)
	nextPtsId[1] = mesh.addPoint(curPt);
	UV.push_back(Point(curAbs, 0.5));
	mesh.addTriangle(nextPtsId[1], prevPtsId[0], prevPtsId[1]);
	mesh.addTriangle(nextPtsId[1], prevPtsId[1], prevPtsId[2]);

	if(side > 0)
	{
		if(hasInternalPoint)
		{
			nextPtsId[2] = mesh.addPoint(curPt - dir);
			UV.push_back(Point(curAbs, 0));
			mesh.addTriangle(nextPtsId[1], prevPtsId[2], nextPtsId[2]);
		}
		else
		{
			Mesh::PointID addPtId = mesh.addPoint(curPt - prevNor * halfWidth);
			UV.push_back(Point(curAbs, 0));
			mesh.addTriangle(nextPtsId[1], prevPtsId[2], addPtId);
			nextPtsId[2] = mesh.addPoint(curPt - curNor * halfWidth);
			UV.push_back(Point(curAbs, 0));
		}

	}
	else // side < 0
	{
		if(hasInternalPoint)
		{
			nextPtsId[0] = mesh.addPoint(curPt + dir);
			UV.push_back(Point(curAbs, 1));
			mesh.addTriangle(nextPtsId[0], prevPtsId[0], nextPtsId[1]);
		}
		else
		{
			Mesh::PointID addPtId = mesh.addPoint(curPt + prevNor * halfWidth);
			UV.push_back(Point(curAbs, 1));
			mesh.addTriangle(addPtId, prevPtsId[0], nextPtsId[1]);
			nextPtsId[0] = mesh.addPoint(curPt + curNor * halfWidth);
			UV.push_back(Point(curAbs, 1));
		}
	}
}

void ExtrudeHelper::join(Mesh::PointID nextPtsId[3], int id, Point dir, float side)
{
	const float halfWidth = halfWidths[id], curAbs = abscissa[id];
	const Point curPt = pts[id], curNor = normals[id], prevNor = normals[id-1];

	// Do a Bevel join instead of a miter join if the angle is too small
	int usedJoinType = joinType;
	if(prevNor.dot(curNor) < -0.9 && joinType == 0)
		usedJoinType = 2;

	// Join
	switch(usedJoinType)
	{
		default:
		case 0: // Miter join
		{
			if(side > 0)
			{
				nextPtsId[0] = mesh.addPoint(curPt + dir);
				UV.push_back(Point(curAbs, 1));
				mesh.addTriangle(nextPtsId[0], prevPtsId[0], nextPtsId[1]);
			}
			else // side < 0
			{
				nextPtsId[2] = mesh.addPoint(curPt - dir);
				UV.push_back(Point(curAbs, 0));
				mesh.addTriangle(nextPtsId[2], nextPtsId[1], prevPtsId[2]);
			}

			break;
		}
		case 1: // Round join
		{
			if(side > 0)
			{
				float angle = acos(prevNor.dot(curNor));
				Mesh::PointID addPtId = mesh.addPoint(curPt + prevNor * halfWidth);
				nextPtsId[0] = mesh.addPoint(curPt + curNor * halfWidth);
				UV.push_back(Point(curAbs, 1));
				UV.push_back(Point(curAbs, 1));

				mesh.addTriangle(addPtId, prevPtsId[0], nextPtsId[1]);

				int nb = static_cast<int>(floor(halfWidth * angle));
				angle /= nb;
				float ca = cos(angle), sa = sin(angle);
				Point center = curPt;
				Point r = curNor * halfWidth;
				Mesh::PointID ptId = nextPtsId[0];
				for(int j=0; j<nb-1; ++j)
				{
					Point nr = Point(r.x*ca+r.y*sa, r.y*ca-r.x*sa);
					Mesh::PointID newPtId = mesh.addPoint(center + nr);
					UV.push_back(Point(curAbs, 1));
					mesh.addTriangle(ptId, newPtId, nextPtsId[1]);

					r = nr;
					ptId = newPtId;
				}
				mesh.addTriangle(ptId, addPtId, nextPtsId[1]);
			}
			else // side < 0
			{
				float angle = acos(prevNor.dot(curNor));
				nextPtsId[2] = mesh.addPoint(curPt - curNor * halfWidth);
				Mesh::PointID addPtId = mesh.addPoint(curPt - prevNor * halfWidth);
				UV.push_back(Point(curAbs, 0));
				UV.push_back(Point(curAbs, 0));

				mesh.addTriangle(addPtId, nextPtsId[1], prevPtsId[2]);

				int nb = static_cast<int>(floor(halfWidth * angle));
				angle /= nb;
				float ca = cos(angle), sa = sin(angle);
				Point center = curPt;
				Point r = prevNor * halfWidth;
				Mesh::PointID ptId = addPtId;
				for(int j=0; j<nb-1; ++j)
				{
					Point nr = Point(r.x*ca+r.y*sa, r.y*ca-r.x*sa);
					Mesh::PointID newPtId = mesh.addPoint(center - nr);
					UV.push_back(Point(curAbs, 0));
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
				Mesh::PointID addPtId = mesh.addPoint(curPt + prevNor * halfWidth);
				nextPtsId[0] = mesh.addPoint(curPt + curNor * halfWidth);
				UV.push_back(Point(curAbs, 1));
				UV.push_back(Point(curAbs, 1));

				mesh.addTriangle(nextPtsId[0], addPtId, nextPtsId[1]);
				mesh.addTriangle(addPtId, prevPtsId[0], nextPtsId[1]);
			}
			else // side < 0
			{
				nextPtsId[2] = mesh.addPoint(curPt - curNor * halfWidth);
				Mesh::PointID addPtId = mesh.addPoint(curPt - prevNor * halfWidth);
				UV.push_back(Point(curAbs, 0));
				UV.push_back(Point(curAbs, 0));

				mesh.addTriangle(nextPtsId[2], nextPtsId[1], addPtId);
				mesh.addTriangle(addPtId, nextPtsId[1], prevPtsId[2]);
			}
			break;
		}
	} // end switch
}

void ExtrudeHelper::lastSegment()
{
	const int id = nbPts - 1;
	const float halfWidth = halfWidths[id], curAbs = abscissa[id];
	const Point curPt = pts[id], curNor = normals[id], prevNor = normals[id - 1];
	Mesh::PointID nextPtsId[3];

	if(closePath)
	{
		float side = prevNor.cross(curNor);
		auto internal = computeInternalDir(id);
		segment(nextPtsId, id, internal.second, side, internal.first);
		join(nextPtsId, id, internal.second, side);

		for(int i=0; i<3; ++i)
			mesh.getPoint(i) = mesh.getPoint(nextPtsId[i]);
	}
	else
	{
		Point dir = prevNor * halfWidth;

		nextPtsId[0] = mesh.addPoint(curPt + dir);
		nextPtsId[1] = mesh.addPoint(curPt);
		nextPtsId[2] = mesh.addPoint(curPt - dir);
		UV.push_back(Point(curAbs, 1));
		UV.push_back(Point(curAbs, 0.5));
		UV.push_back(Point(curAbs, 0));
		mesh.addTriangle(nextPtsId[0], prevPtsId[0], nextPtsId[1]);
		mesh.addTriangle(prevPtsId[0], prevPtsId[1], nextPtsId[1]);
		mesh.addTriangle(nextPtsId[1], prevPtsId[1], prevPtsId[2]);
		mesh.addTriangle(nextPtsId[1], prevPtsId[2], nextPtsId[2]);
	}

	for(int j=0; j<3; ++j)
		prevPtsId[j] = nextPtsId[j];
}

void ExtrudeHelper::endCap()
{
	switch(capType)
	{
		default:
		case 0:	// Butt cap (nothing to do)
			break;
		case 1:	// Round cap
		{
			const float halfWidth = halfWidths.back();
			int nb = static_cast<int>(floor(halfWidth * M_PI));
			float angle = static_cast<float>(M_PI) / nb; // We do a half turn
			float ca = cos(angle), sa = sin(angle);
			Point center = pts[nbPts-1];
			Point dir = normals[nbPts-2] * halfWidth;
			dir = -dir;
			Mesh::PointID ptId = prevPtsId[2];
			const float lastAbs = abscissa[nbPts-1];
			Point uvPt(0, 1), uvCenter(lastAbs, 0.5), uvMult(1-lastAbs, -0.5);
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
			const float w = halfWidths.back();
			Point dir = normals[nbPts-2] * w;
			Point dir2 = Point(dir.y, -dir.x);
			Mesh::PointID addPtsId[3];
			const Point currentPt = pts[nbPts-1];
			addPtsId[0] = mesh.addPoint(currentPt + dir - dir2);
			addPtsId[1] = mesh.addPoint(currentPt       - dir2);
			addPtsId[2] = mesh.addPoint(currentPt - dir - dir2);
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


