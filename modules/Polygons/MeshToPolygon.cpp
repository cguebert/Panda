#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Mesh.h>
#include <panda/types/Polygon.h>

#include <QMap>
#include <set>
#include <iostream>

namespace panda {

using types::Mesh;
using types::Path;
using types::Point;
using types::Polygon;

class Polygon_CreateFromMesh : public PandaObject
{
public:
	PANDA_CLASS(Polygon_CreateFromMesh, PandaObject)

	Polygon_CreateFromMesh(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input mesh"))
		, m_output(initData("output", "Output polygon" ))
	{
		addInput(m_input);
		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();

		auto output = m_output.getAccessor();
		output.clear();

		if(input.empty())
		{
			cleanDirty();
			return;
		}

		for(Mesh mesh : input) // Take a non-const copy
		{
			const auto& points = mesh.getPoints();
			std::set<int> unusedPts;
			QMap<int, QVector<int>> neighbours;
			auto borderEdges = mesh.getEdgesOnBorder();
			for(auto edgeId : borderEdges)
			{
				const auto& edge = mesh.getEdge(edgeId);
				unusedPts.insert(edge[0]);
				unusedPts.insert(edge[1]);
				neighbours[edge[0]].push_back(edge[1]);
				neighbours[edge[1]].push_back(edge[0]);
			}

			QVector<Path> tempPaths; // Before we can separate the contour from the holes
			while(!unusedPts.empty())
			{
				int start = *unusedPts.begin();
				unusedPts.erase(start);
				QVector<int> ptsId;
				ptsId.push_back(start);

				int prev = start, current = start;
				bool found = true;
				while(found)
				{
					found = false;
					for(auto ptId : neighbours[current])
					{
						if(ptId != prev/* && unusedPts.find(ptId) != unusedPts.end()*/)
						{
							ptsId.push_back(ptId);
							unusedPts.erase(ptId);
							prev = current;
							current = ptId;
							found = true;
							break;
						}
					}

					if(current == start)
						break;
				}

				if(ptsId.size() > 1)
				{
					Path path;
					for(auto p : ptsId)
						path.push_back(points[p]);
					tempPaths.push_back(path);
				}
			}

			Polygon poly;
			if(tempPaths.size() == 1)
			{
				poly.contour = std::move(tempPaths.front());
			}
			else
			{	// The path with the biggest area is the contour
				int contourId = 0;
				PReal maxArea = 0;
				for(int i=0, nb=tempPaths.size(); i<nb; ++i)
				{
					auto area = fabs(types::areaOfPolygon(tempPaths[i]));
					if(area > maxArea)
					{
						contourId = i;
						maxArea = area;
					}
				}

				for(int i=0, nb=tempPaths.size(); i<nb; ++i)
				{
					if(i == contourId)
						poly.contour = std::move(tempPaths[i]);
					else
						poly.holes.push_back(std::move(tempPaths[i]));
				}
			}

			output.push_back(poly);
		}

		cleanDirty();
	}

protected:
	Data< QVector<Mesh> > m_input;
	Data< QVector<Polygon> > m_output;
};

int Polygon_CreateFromMeshClass = RegisterObject<Polygon_CreateFromMesh>("Modifier/Mesh/Mesh to polygon")
		.setDescription("Compute the polygon corresponding to the input mesh");

} // namespace Panda
