#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Mesh.h>

namespace panda {

using types::Mesh;
using types::Point;

class MeshMath_Centroids : public PandaObject
{
public:
	PANDA_CLASS(MeshMath_Centroids, PandaObject)

	MeshMath_Centroids(PandaDocument *doc)
		: PandaObject(doc)
		, mesh(initData(&mesh, "mesh", "Mesh to analyse"))
		, centroids(initData(&centroids, "centroids", "Centroids of the polygons"))
	{
		addInput(&mesh);

		addOutput(&centroids);
	}

	void update()
	{
		const Mesh& topo = mesh.getValue();
		auto pts = centroids.getAccessor();
		pts.clear();
		pts.resize(topo.getNumberOfPolygons());

		int i=0;
		for(auto poly : topo.getPolygons())
			pts[i++] = topo.centroidOfPolygon(poly);

		cleanDirty();
	}

protected:
	Data<Mesh> mesh;
	Data< QVector<Point> > centroids;
};

int MeshMath_CentroidsClass = RegisterObject<MeshMath_Centroids>("Math/Mesh/Centroids")
		.setDescription("Compute the centroid of each polygon");

} // namespace Panda
