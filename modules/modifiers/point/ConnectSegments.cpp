#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QMap>
#include <set>

// To be able to use QPointF in QMaps (actual definition in helper/Point.cpp)
template<> bool qMapLessThanKey<QPointF>(const QPointF& p1, const QPointF& p2);

namespace panda {

class ModifierPoints_ConnectSegments : public PandaObject
{
public:
	PANDA_CLASS(ModifierPoints_ConnectSegments, PandaObject)

	ModifierPoints_ConnectSegments(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "List of segments (pair of points)"))
		, output(initData(&output, "output", "List of connected points" ))
		, required(initData(&required, "required", "If possible, the line must go though these points"))
	{
		addInput(&input);
		addInput(&required);

		addOutput(&output);
	}

	void update()
	{
		const auto& inList = input.getValue();
		const auto& reqList = required.getValue();

		auto outList = output.getAccessor();
		outList.clear();

		if(inList.empty())
		{
			cleanDirty();
			return;
		}

		QMap<QPointF, int> pointsMap;
		QVector<QPointF> points;
		QMap<int, QVector<int> > neighbours;
		for(int i=0, nb=inList.size()/2; i<nb; ++i)
		{
			const QPointF &pt1 = inList[i*2], &pt2 = inList[i*2+1];
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

		QVector<int> reqIndices;
		for(auto pt : reqList)
			reqIndices.push_back(pointsMap.value(reqList[0], 0));

		int start = 0;
		if(!reqIndices.empty())
			start = reqIndices.front();

		std::set<int> includedIndices;
		includedIndices.insert(start);

		QList<int> resIndices;
		resIndices.push_back(start);

		// First direction
		int prev = start, current = start;
		bool found = true;
		while(found)
		{
			found = false;
			for(auto p : neighbours[current])
			{
				if(p != prev)
				{
					resIndices.push_back(p);
					if(includedIndices.find(p) != includedIndices.end())	// We alreaded included this point, we found a loop
						break;
					includedIndices.insert(p);
					prev = current;
					current = p;
					found = true;
					break;
				}
			}
		}

		for(auto p : resIndices)
			outList.push_back(points[p]);

		cleanDirty();
	}

protected:
	Data< QVector<QPointF> > input, output, required;
};

int ModifierPoints_ConnectSegmentsClass = RegisterObject<ModifierPoints_ConnectSegments>("Modifier/Point/Connect segments")
		.setDescription("Create a continous line from a list of segments, including the specified index");

} // namespace Panda
