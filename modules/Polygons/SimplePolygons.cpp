#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Path.h>
#include <panda/types/Rect.h>
#include <QVector>

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
		, m_rectangles(initData(&m_rectangles, "rectangle", "Input rectangle"))
		, m_paths(initData(&m_paths, "path", "Output path"))
	{
		addInput(m_rectangles);
		addOutput(m_paths);
	}

	void update()
	{
		const QVector<Rect>& rects = m_rectangles.getValue();
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

		cleanDirty();
	}

protected:
	Data< QVector<Rect> > m_rectangles;
	Data< QVector<Path> > m_paths;
};

int PolygonCreation_RectangleClass = RegisterObject<PolygonCreation_Rectangle>("Generator/Polygon/Rectangle path").setDescription("Create a rectangular path");

//****************************************************************************//

class PolygonCreation_Circle : public PandaObject
{
public:
	PANDA_CLASS(PolygonCreation_Circle, PandaObject)

	PolygonCreation_Circle(PandaDocument *doc)
		: PandaObject(doc)
		, m_centers(initData(&m_centers, "center", "Center of the circle"))
		, m_radiuses(initData(&m_radiuses, "radius", "Radius of the circle"))
		, m_precision(initData(&m_precision, (PReal)1.0, "precision", "Maximum length of a segment"))
		, m_paths(initData(&m_paths, "path", "Output path"))
	{
		addInput(m_centers);
		addInput(m_radiuses);
		addInput(m_precision);
		addOutput(m_paths);
	}

	void update()
	{
		const QVector<Point>& centers = m_centers.getValue();
		const QVector<PReal>& radiuses = m_radiuses.getValue();
		int nbC = centers.size();
		int nbR = radiuses.size();
		auto paths = m_paths.getAccessor();

		if(!nbC || !nbR)
		{
			paths.clear();
			cleanDirty();
			return;
		}

		int nb = std::max(nbC, nbR);
		PReal precision = std::max((PReal)1e-3, m_precision.getValue());
		PReal PI2 = static_cast<PReal>(M_PI) * 2;

		paths.clear();
		paths.reserve(nb);
		for(int i=0; i<nb; ++i)
		{
			const Point& center = centers[i%nbC];
			const PReal radius = radiuses[i%nbR];

			int nbSeg = static_cast<int>(floor(radius * PI2 / precision));
			if(nbSeg < 3) continue;

			Path path;
			path.resize(nbSeg + 1);

			PReal angle = PI2 / nbSeg;
			PReal ca = cos(angle), sa = sin(angle);
			Point dir = Point(radius, 0);

			for(int i=0; i<=nbSeg; ++i)
			{
				Point pt = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
				path[i] = center + pt;
				dir = pt;
			}

			paths.push_back(path);
		}

		cleanDirty();
	}

protected:
	Data< QVector<Point> > m_centers;
	Data< QVector<PReal> > m_radiuses;
	Data< PReal > m_precision;
	Data< QVector<Path> > m_paths;
};

int PolygonCreation_CircleClass = RegisterObject<PolygonCreation_Circle>("Generator/Polygon/Circle path").setDescription("Create a circular path");

//****************************************************************************//

} // namespace Panda


