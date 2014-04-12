#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Mesh.h>

namespace panda {

using types::Mesh;
using types::Point;

class ModifierMesh_FindPolygon : public PandaObject
{
public:
	PANDA_CLASS(ModifierMesh_FindPolygon, PandaObject)

	ModifierMesh_FindPolygon(PandaDocument *doc)
		: PandaObject(doc)
		, mesh(initData(&mesh, "mesh", "Mesh in which to search"))
		, points(initData(&points, "points", "List of points to test"))
		, indices(initData(&indices, "indices", "Indices of the polygons are the corresponding points"))
	{
		addInput(&mesh);
		addInput(&points);

		addOutput(&indices);
	}

	void update()
	{
		const Mesh& topo = mesh.getValue();

		const QVector<Point>& pts = points.getValue();
		auto output = indices.getAccessor();
		int nbPts = pts.size();
		output.wref().fill(Mesh::InvalidID, nbPts);

		int nbPoly = topo.getNumberOfPolygons();

		for(int i=0; i<nbPts; ++i)
		{
			Point pt = pts[i];
			for(int j=0; j<nbPoly; ++j)
			{
				const Mesh::Polygon& poly = topo.getPolygon(j);
				if(topo.polygonContainsPoint(poly, pt))
				{
					output[i] = j;
					break;
				}
			}
		}

		cleanDirty();
	}

protected:
	Data< Mesh > mesh;
	Data< QVector<Point> > points;
	Data< QVector<int> > indices;
};

int ModifierMesh_FindPolygonClass = RegisterObject<ModifierMesh_FindPolygon>("Modifier/Mesh/Find polygon").setDescription("Find polygons are specific positions");

} // namespace Panda
