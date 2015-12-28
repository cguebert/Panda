#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Polygon.h>
#include <panda/types/Mesh.h>

#include "libs/clip2tri/clip2tri.h"

namespace panda {

using types::Mesh;
using types::Path;
using types::Point;
using types::Polygon;

std::vector<c2t::Point> convertPath(const Path& path)
{
	std::vector<c2t::Point> result;
	result.reserve(path.size());
	for(const Point& pt : path)
		result.emplace_back(c2t::Point(pt.x, pt.y));

	if(result.front() == result.back())
		result.pop_back();

	return result;
}

Point convertPoint(const c2t::Point& pt)
{
	return Point(pt.x, pt.y);
}

class Polygon_Triangulation : public PandaObject
{
public:
	PANDA_CLASS(Polygon_Triangulation, PandaObject)

	Polygon_Triangulation(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input polygon"))
		, m_output(initData("output", "Output mesh"))
	{
		addInput(m_input);

		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto output = m_output.getAccessor();
		output.clear();

		if(input.empty())
		{
			cleanDirty();
			return;
		}

		for(const auto& inputPoly : input)
		{
			typedef std::vector<c2t::Point> c2tPoly;
			typedef std::vector<c2tPoly> c2tPolyList;
			c2tPolyList holes;
			c2tPoly contour, triangulated;

			contour = convertPath(inputPoly.contour);
			for(const auto& hole : inputPoly.holes)
				holes.push_back(convertPath(hole));

			c2t::triangulate(holes, contour, triangulated);
			Mesh mesh;

			size_t nbTri = triangulated.size() / 3;
			for(size_t i=0; i<nbTri; ++i)
			{
				int ptId1 = mesh.addPoint(convertPoint(triangulated[i*3  ]));
				int ptId2 = mesh.addPoint(convertPoint(triangulated[i*3+1]));
				int ptId3 = mesh.addPoint(convertPoint(triangulated[i*3+2]));
				mesh.addTriangle(ptId1, ptId2, ptId3);
			}

			mesh.createEdgeList();
			output.push_back(mesh);
		}
		cleanDirty();
	}

protected:
	Data< std::vector<Polygon> > m_input;
	Data< std::vector<Mesh> > m_output;
};

int Polygon_TriangulationClass = RegisterObject<Polygon_Triangulation>("Generator/Mesh/Triangulation").setDescription("Compute a triangulation of a polygon");

//****************************************************************************//



} // namespace Panda


