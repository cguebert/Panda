#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Path.h>
#include <panda/types/Rect.h>

#include <cmath>
#include <algorithm>

namespace panda {

using types::Point;
using types::Path;
using types::Rect;

class PolygonCreation_Rectangle : public PandaObject
{
public:
	PANDA_CLASS(PolygonCreation_Rectangle, PandaObject)

	PolygonCreation_Rectangle(PandaDocument *doc)
		: PandaObject(doc)
		, m_rectangles(initData("rectangle", "Input rectangle"))
		, m_paths(initData("path", "Output path"))
	{
		addInput(m_rectangles);
		addOutput(m_paths);
	}

	void update()
	{
		const std::vector<Rect>& rects = m_rectangles.getValue();
		int nb = rects.size();

		auto paths = m_paths.getAccessor();
		paths.clear();
		paths.resize(nb);
		for(int i=0; i<nb; ++i)
		{
			const Rect& r = rects[i];
			Path& path = paths[i];
			path.push_back(r.topLeft());
			path.push_back(r.bottomLeft());
			path.push_back(r.bottomRight());
			path.push_back(r.topRight());
			path.push_back(r.topLeft());
		}
	}

protected:
	Data< std::vector<Rect> > m_rectangles;
	Data< std::vector<Path> > m_paths;
};

int PolygonCreation_RectangleClass = RegisterObject<PolygonCreation_Rectangle>("Generator/Polygon/Rectangle path").setDescription("Create a rectangular path");

//****************************************************************************//

class PolygonCreation_Circle : public PandaObject
{
public:
	PANDA_CLASS(PolygonCreation_Circle, PandaObject)

	PolygonCreation_Circle(PandaDocument *doc)
		: PandaObject(doc)
		, m_centers(initData("center", "Center of the circle"))
		, m_radiuses(initData("radius", "Radius of the circle"))
		, m_precision(initData((float)1.0, "precision", "Maximum length of a segment"))
		, m_paths(initData("path", "Output path"))
	{
		addInput(m_centers);
		addInput(m_radiuses);
		addInput(m_precision);
		addOutput(m_paths);
	}

	void update()
	{
		const std::vector<Point>& centers = m_centers.getValue();
		const std::vector<float>& radiuses = m_radiuses.getValue();
		int nbC = centers.size();
		int nbR = radiuses.size();
		auto paths = m_paths.getAccessor();

		if(!nbC || !nbR)
		{
			paths.clear();
			return;
		}

		int nb = std::max(nbC, nbR);
		float precision = std::max((float)1e-3, m_precision.getValue());
		float PI2 = static_cast<float>(M_PI) * 2;

		paths.clear();
		paths.reserve(nb);
		for(int i=0; i<nb; ++i)
		{
			const Point& center = centers[i%nbC];
			const float radius = radiuses[i%nbR];

			int nbSeg = static_cast<int>(floor(radius * PI2 / precision));
			if(nbSeg < 3) continue;

			Path path;
			path.resize(nbSeg + 1);

			float angle = PI2 / nbSeg;
			float ca = cos(angle), sa = sin(angle);
			Point dir = Point(radius, 0);

			for(int i=0; i<=nbSeg; ++i)
			{
				Point pt = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
				path[i] = center + pt;
				dir = pt;
			}

			paths.push_back(path);
		}
	}

protected:
	Data< std::vector<Point> > m_centers;
	Data< std::vector<float> > m_radiuses;
	Data< float > m_precision;
	Data< std::vector<Path> > m_paths;
};

int PolygonCreation_CircleClass = RegisterObject<PolygonCreation_Circle>("Generator/Polygon/Circle path").setDescription("Create a circular path");

//****************************************************************************//

} // namespace Panda


