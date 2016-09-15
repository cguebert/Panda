#include <panda/document/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Mesh.h>

namespace panda {

using types::Mesh;
using types::Point;

class ModifierMesh_FindTriangle : public PandaObject
{
public:
	PANDA_CLASS(ModifierMesh_FindTriangle, PandaObject)

	ModifierMesh_FindTriangle(PandaDocument *doc)
		: PandaObject(doc)
		, mesh(initData("mesh", "Mesh in which to search"))
		, points(initData("points", "List of points to test"))
		, indices(initData("indices", "Indices of the triangles at the corresponding points"))
	{
		addInput(mesh);
		addInput(points);

		addOutput(indices);
	}

	void update()
	{
		const Mesh& inMesh = mesh.getValue();

		const std::vector<Point>& pts = points.getValue();
		auto output = indices.getAccessor();
		int nbPts = pts.size();
		output.wref().resize(nbPts, Mesh::InvalidID);

		int nbTri = inMesh.nbTriangles();

		for(int i=0; i<nbPts; ++i)
		{
			Point pt = pts[i];
			for(int j=0; j<nbTri; ++j)
			{
				const Mesh::Triangle& triangle = inMesh.getTriangle(j);
				if(inMesh.triangleContainsPoint(triangle, pt))
				{
					output[i] = j;
					break;
				}
			}
		}
	}

protected:
	Data< Mesh > mesh;
	Data< std::vector<Point> > points;
	Data< std::vector<int> > indices;
};

int ModifierMesh_FindTriangleClass = RegisterObject<ModifierMesh_FindTriangle>("Modifier/Mesh/Find triangle").setDescription("Find triangles at specific positions");

} // namespace Panda
