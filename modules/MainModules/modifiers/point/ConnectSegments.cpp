#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Point.h>
#include <QMap>
#include <set>

template<> static bool qMapLessThanKey<panda::types::Point>(const panda::types::Point& p1, const panda::types::Point& p2)
{
	return p1.x < p2.x || (p1.x == p2.x && p1.y < p2.y);
}

namespace panda {

using types::Point;

class ModifierPoints_ConnectSegments : public PandaObject
{
public:
	PANDA_CLASS(ModifierPoints_ConnectSegments, PandaObject)

	ModifierPoints_ConnectSegments(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "List of segments (pair of points)"))
		, m_output(initData("output", "List of connected points" ))
	{
		addInput(m_input);
		addOutput(m_output);
	}

	std::pair<int, int> make_edge(int a, int b)
	{
		if(a < b)
			return std::make_pair(a, b);
		else
			return std::make_pair(b, a);
	}

	void update()
	{
		const auto& inList = m_input.getValue();

		auto outList = m_output.getAccessor();
		outList.clear();

		if(inList.empty())
		{
			cleanDirty();
			return;
		}

		QMap<Point, int> pointsMap;
		QVector<Point> points;
		QMap<int, QVector<int> > neighbours;
		for(int i=0, nb=inList.size()/2; i<nb; ++i)
		{
			const Point &pt1 = inList[i*2], &pt2 = inList[i*2+1];
			if(!pointsMap.contains(pt1))
			{
				pointsMap[pt1] = points.size();
				points.push_back(pt1);
			}
			if(!pointsMap.contains(pt2))
			{
				pointsMap[pt2] = points.size();
				points.push_back(pt2);
			}

			int i1 = pointsMap[pt1], i2 = pointsMap[pt2];
			neighbours[i1].push_back(i2);
			neighbours[i2].push_back(i1);
		}

		int start = 0;
		std::set<std::pair<int, int>> usedEdges;

		QList<int> resIndices;
		resIndices.push_back(start);

		PReal pi2 = 2 * (PReal)M_PI;
		int prev = start, current = start;
		bool found = true;
		while(found)
		{
			found = false;
			int best = -1;
			PReal minAngle = 7; // > 2 * pi
			Point BA = points[start] - points[current];
			PReal prevAngle = (start != current ? atan2(BA.y, BA.x) : 0);
			for(auto p : neighbours[current])
			{
				if(p != prev && usedEdges.find(make_edge(current, p)) == usedEdges.end())
				{
					// Compute the angle from the current segment to this one
					Point BC = points[p] - points[current];
					PReal angle = atan2(BC.y, BC.x);
					PReal delta = angle;
				/*	PReal delta = fabs(angle - prevAngle);
					if(delta > pi2)
						delta -= pi2;
				*/	if(delta < minAngle)
					{
						minAngle = delta;
						best = p;
					}
				}
			}

			if(best != -1)
			{
				resIndices.push_back(best);
				usedEdges.insert(make_edge(current, best));
				prev = current;
				current = best;
				found = true;
			}
			else // Can we close the loop ?
			{
				if(neighbours[current].contains(start))
					resIndices.push_back(start);
				// Else: go back to a point with neighbours, and continue in another direction
			}
		}

		for(auto p : resIndices)
			outList.push_back(points[p]);

		cleanDirty();
	}

protected:
	Data< QVector<Point> > m_input, m_output;
};

int ModifierPoints_ConnectSegmentsClass = RegisterObject<ModifierPoints_ConnectSegments>("Modifier/Point/Connect segments")
		.setDescription("Create a continous line from a list of segments");

} // namespace Panda
