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
		, mesh(initData("mesh", "Mesh to analyse"))
		, centroids(initData("centroids", "Centroids of the triangles"))
	{
		addInput(mesh);

		addOutput(centroids);
	}

	void update()
	{
		const Mesh& inMesh = mesh.getValue();
		auto pts = centroids.getAccessor();
		pts.clear();
		pts.resize(inMesh.nbTriangles());

		int i=0;
		for(auto triangle : inMesh.getTriangles())
			pts[i++] = inMesh.centroidOfTriangle(triangle);

		cleanDirty();
	}

protected:
	Data<Mesh> mesh;
	Data< QVector<Point> > centroids;
};

int MeshMath_CentroidsClass = RegisterObject<MeshMath_Centroids>("Math/Mesh/Centroids")
		.setDescription("Compute the centroid of each triangle");

} // namespace Panda
