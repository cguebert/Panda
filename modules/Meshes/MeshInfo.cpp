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

//****************************************************************************//

class MeshMath_Area : public PandaObject
{
public:
	PANDA_CLASS(MeshMath_Area, PandaObject)

	MeshMath_Area(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("mesh", "Mesh to analyse"))
		, m_output(initData("area", "Area of the mesh"))
	{
		addInput(m_input);
		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto output = m_output.getAccessor();

		int nb = input.size();
		output.resize(nb);
		for(int i=0; i<nb; ++i)
		{
			const auto& mesh = input[i];
			PReal area = 0;
			for(int j=0, nbT=mesh.nbTriangles(); j<nbT; ++j)
				area += mesh.areaOfTriangle(mesh.getTriangle(j));
			output[i] = fabs(area);
		}

		cleanDirty();
	}

protected:
	Data< QVector<Mesh> > m_input;
	Data< QVector<PReal> > m_output;
};

int MeshMath_AreaClass = RegisterObject<MeshMath_Area>("Math/Mesh/Area")
						 .setName("Area of mesh")
						 .setDescription("Compute the area of a mesh");


} // namespace Panda
