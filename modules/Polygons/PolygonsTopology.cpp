#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/IntVector.h>
#include <panda/types/Path.h>

#include <algorithm>

namespace panda {

using types::IntVector;
using types::Path;
using types::Point;

class Polygon_FacesAroundPoint : public PandaObject
{
public:
	PANDA_CLASS(Polygon_FacesAroundPoint, PandaObject)

	Polygon_FacesAroundPoint(PandaDocument *doc)
		: PandaObject(doc)
		, m_indices(initData("indices", "Indices of points forming the polygons"))
		, m_neighbors(initData("neighbors", "For each point, neighboring polygons"))
	{
		addInput(m_indices);
		addOutput(m_neighbors);
	}

	void update()
	{
		const auto& indices = m_indices.getValue();
		auto neighborsAcc = m_neighbors.getAccessor();
		auto& neighbors = neighborsAcc.wref();
		neighbors.clear();

		// First pass, get the number of points
		int nbPoints = -1;
		for (const auto& list : indices)
		{
			for (const auto& id : list.values)
			{
				if (id > nbPoints)
					nbPoints = id;
			}
		}

		if (nbPoints < 0)
			return;

		// Second pass, add the index of each polygon to every of its points
		neighbors.resize(nbPoints + 1);
		int nbPoly = indices.size();
		for (int i = 0; i < nbPoly; ++i)
		{
			const auto& poly = indices[i].values;
			int nb = poly.size();
			for (int j = 0; j < nb - 1; ++j)
			{
				const auto id = poly[j];
				neighbors[id].values.push_back(i);
			}

			// The last point is often a copy of the first one
			if (nb && poly.back() != poly.front())
				neighbors[poly.back()].values.push_back(i);
		}
	}

protected:
	Data< std::vector<IntVector> > m_indices, m_neighbors;	
};

int Polygon_FacesAroundPointClass = RegisterObject<Polygon_FacesAroundPoint>("Generator/Polygon/Polygons around point")
	.setDescription("Compute the neighboring polygon of each point");

//****************************************************************************//


} // namespace Panda


