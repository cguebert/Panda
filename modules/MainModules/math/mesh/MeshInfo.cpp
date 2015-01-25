#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Mesh.h>

namespace panda {

using types::Mesh;

class MeshMath_NumberOfPrimitives : public PandaObject
{
public:
	PANDA_CLASS(MeshMath_NumberOfPrimitives, PandaObject)

	MeshMath_NumberOfPrimitives(PandaDocument *doc)
		: PandaObject(doc)
		, mesh(initData("mesh", "Mesh to analyse"))
		, nbPoints(initData("nb points", "Number of points in the mesh"))
		, nbEdges(initData("nb edges", "Number of edges in the mesh"))
		, nbTriangles(initData("nb triangles", "Number of triangles in the mesh"))
	{
		addInput(mesh);

		addOutput(nbPoints);
		addOutput(nbEdges);
		addOutput(nbTriangles);
	}

	void update()
	{
		const Mesh& inMesh = mesh.getValue();

		nbPoints.setValue(inMesh.nbPoints());
		nbEdges.setValue(inMesh.nbEdges());
		nbTriangles.setValue(inMesh.nbTriangles());

		cleanDirty();
	}

protected:
	Data<Mesh> mesh;
	Data<int> nbPoints, nbEdges, nbTriangles;
};

int MeshMath_NumberOfPrimitivesClass = RegisterObject<MeshMath_NumberOfPrimitives>("Math/Mesh/Number of primitives")
		.setDescription("Compute the number of points, edges and triangles in the mesh");

//****************************************************************************//

} // namespace Panda
