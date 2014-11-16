#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Mesh.h>
#include <panda/helper/PointsGrid.h>

#include <QMap>

// To be able to use Point in QMaps
template<> static bool qMapLessThanKey<panda::types::Point>(const panda::types::Point& p1, const panda::types::Point& p2)
{
	return p1.x < p2.x || (p1.x == p2.x && p1.y < p2.y);
}

namespace panda {

using types::Point;
using types::Rect;
using types::Mesh;

class GeneratorMesh_MergeMeshes : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_MergeMeshes, PandaObject)

	GeneratorMesh_MergeMeshes(PandaDocument *doc)
		: PandaObject(doc)
		, inputs(initData(&inputs, "inputs", "Meshes to merge"))
		, threshold(initData(&threshold, (PReal)0.1, "threshold", "Distance under which 2 points are considered the same"))
		, output(initData(&output, "output", "Merged mesh"))
	{
		addInput(&inputs);
		addInput(&threshold);

		addOutput(&output);
	}

	void update()
	{
		helper::PointsGrid grid;
		QSize size = m_parentDocument->getRenderSize();
		Rect area(0, 0, size.width(), size.height());
		grid.initGrid(area, 10);

		PReal thres = threshold.getValue();

		Mesh::SeqPoints newPoints;
		Mesh::SeqTriangles newTriangles;

		QMap<Point, Mesh::PointID> pointsMap;

		const QVector<Mesh>& meshesList = inputs.getValue();
		for(const Mesh& mesh : meshesList)
		{
			if(newPoints.empty())
			{
				newPoints = mesh.getPoints();
				newTriangles = mesh.getTriangles();
				grid.addPoints(newPoints);
				for(int i=0, nb=newPoints.size(); i<nb; ++i)
					pointsMap[newPoints[i]] = i;
			}
			else
			{
				QMap<Mesh::PointID, Mesh::PointID> tmpIDMap;
				for(int i=0, nb=mesh.getNumberOfPoints(); i<nb; ++i)
				{
					const Point& pt = mesh.getPoint(i);
					Point res;
					if(grid.getNearest(pt, thres, res))
					{
						tmpIDMap[i] = pointsMap[res];
					}
					else
					{
						int id = newPoints.size();
						pointsMap[pt] = id;
						tmpIDMap[i] = id;
						newPoints.push_back(pt);
						grid.addPoint(pt);
					}
				}

				for(auto triangle : mesh.getTriangles())
				{
					for(auto& id : triangle)
						id = tmpIDMap[id];

					newTriangles.push_back(triangle);
				}
			}
		}

		auto outMesh = output.getAccessor();
		outMesh->clear();
		outMesh->addPoints(newPoints);
		outMesh->addTriangles(newTriangles);

		cleanDirty();
	}

protected:
	Data< QVector<Mesh> > inputs;
	Data< PReal > threshold;
	Data< Mesh > output;
};

int GeneratorMesh_MergeMeshesClass = RegisterObject<GeneratorMesh_MergeMeshes>("Generator/Mesh/Merge meshes").setDescription("Merge multiple meshes into one");

} // namespace Panda
