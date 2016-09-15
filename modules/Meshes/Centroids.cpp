#include <panda/document/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

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
		, m_mesh(initData("mesh", "Mesh to analyse"))
		, m_centroids(initData("centroids", "Centroids of the triangles"))
	{
		addInput(m_mesh);
		addOutput(m_centroids);
	}

	void update()
	{
		const Mesh& inMesh = m_mesh.getValue();
		auto pts = m_centroids.getAccessor();
		pts.clear();
		pts.resize(inMesh.nbTriangles());

		int i=0;
		for(auto triangle : inMesh.getTriangles())
			pts[i++] = inMesh.centroidOfTriangle(triangle);
	}

protected:
	Data<Mesh> m_mesh;
	Data< std::vector<Point> > m_centroids;
};

int MeshMath_CentroidsClass = RegisterObject<MeshMath_Centroids>("Math/Mesh/Centroids of triangles")
		.setDescription("Compute the centroid of each triangle");

//****************************************************************************//

class MeshMath_Centroid : public PandaObject
{
public:
	PANDA_CLASS(MeshMath_Centroid, PandaObject)

	MeshMath_Centroid(PandaDocument *doc)
		: PandaObject(doc)
		, m_mesh(initData("mesh", "Mesh to analyse"))
		, m_centroids(initData("centroids", "Centroids of the triangles"))
	{
		addInput(m_mesh);
		addOutput(m_centroids);
	}

	void update()
	{
		const auto& meshes = m_mesh.getValue();
		auto centroids = m_centroids.getAccessor();
		centroids.clear();
		int nb = meshes.size();
		centroids.resize(nb);

		for(int i=0; i<nb; ++i)
		{
			Point avg;
			float totArea = 0;
			const auto& mesh = meshes[i];
			for(int j=0, nbP = mesh.nbTriangles(); j<nbP; ++j)
			{
				const auto tri = mesh.getTriangle(j);
				float area = mesh.areaOfTriangle(tri);
				avg += mesh.centroidOfTriangle(tri) * area;
				totArea += area;
			}

			centroids[i] = avg / totArea;
		}
	}

protected:
	Data< std::vector<Mesh> > m_mesh;
	Data< std::vector<Point> > m_centroids;
};

int MeshMath_CentroidClass = RegisterObject<MeshMath_Centroid>("Math/Mesh/Centroid")
		.setName("Centroid of mesh")
		.setDescription("Compute the centroid of a mesh");

} // namespace Panda
