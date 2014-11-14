#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Path.h>
#include <panda/types/Mesh.h>
#include <QVector>

#include "libs/clip2tri/clip2tri.h"

namespace panda {

using types::Mesh;
using types::Point;
using types::Path;

class Polygon_Triangulation : public PandaObject
{
public:
	PANDA_CLASS(Polygon_Triangulation, PandaObject)

	Polygon_Triangulation(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData(&m_input, "input", "Input polygon"))
		, m_output(initData(&m_output, "output", "Output mesh"))
	{
		addInput(&m_input);

		addOutput(&m_output);
	}

	Point convert(const c2t::Point& pt)
	{
		return Point(pt.x, pt.y);
	}

	void update()
	{
		const Path& input = m_input.getValue();
		auto output = m_output.getAccessor();
		output->clear();

		if(input.empty())
		{
			cleanDirty();
			return;
		}

		typedef std::vector<c2t::Point> c2tPoly;
		typedef std::vector<c2tPoly> c2tPolyList;
		c2tPolyList inputPolygons;
		c2tPoly boundingPolygon, triangulated;

		boundingPolygon.reserve(input.size());
		for(const Point& pt : input)
			boundingPolygon.emplace_back(c2t::Point(pt.x, pt.y));

		if(boundingPolygon.front() == boundingPolygon.back())
			boundingPolygon.pop_back();

		c2t::triangulate(inputPolygons, boundingPolygon, triangulated);

		size_t nbTri = triangulated.size() / 3;
		for(size_t i=0; i<nbTri; ++i)
		{
			int ptId1 = output->addPoint(convert(triangulated[i*3  ]));
			int ptId2 = output->addPoint(convert(triangulated[i*3+1]));
			int ptId3 = output->addPoint(convert(triangulated[i*3+2]));
			output->addTriangle(ptId1, ptId2, ptId3);
		}

		output->createEdgeList();
		cleanDirty();
	}

protected:
	Data< Path > m_input;
	Data< Mesh > m_output;
};

int Polygon_TriangulationClass = RegisterObject<Polygon_Triangulation>("Generator/Mesh/Triangulation").setDescription("Compute a triangulation of a polygon");

//****************************************************************************//



} // namespace Panda


