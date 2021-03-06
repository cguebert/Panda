#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/typesLists.h>

#include <set>

using panda::types::Color;
using panda::types::Point;
using panda::types::Rect;

namespace std {

template<>
struct less<Color> : public binary_function<Color, Color, bool>
{
	bool operator()(const Color& lhs, const Color& rhs) const
	{
		return lhs.toHex() < rhs.toHex();
	}
};

template<>
struct less<Point> : public binary_function<Point, Point, bool>
{
	bool operator()(const Point& lhs, const Point& rhs) const
	{
		return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y);
	}
};

template<>
struct less<Rect> : public binary_function<Rect, Rect, bool>
{
	bool operator()(const Rect& lhs, const Rect& rhs) const
	{
		if(lhs.left() < rhs.left()) return true;
		if(rhs.left() < lhs.left()) return false;

		if(lhs.top() < rhs.top()) return true;
		if(rhs.top() < lhs.top()) return false;

		if(lhs.right() < rhs.right()) return true;
		if(rhs.right() < lhs.right()) return false;

		if(lhs.bottom() < rhs.bottom()) return true;
		if(rhs.bottom() < lhs.bottom()) return false;

		return false;
	}
};

}

namespace panda {

using types::DataTypeId;

class RemoveDuplicates : public GenericObject
{
public:
	PANDA_CLASS(RemoveDuplicates, GenericObject)

	RemoveDuplicates(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the lists in which to remove duplicates"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.emplace_back(listType,
							 GenericDataDefinition::Input,
							 "input",
							 "Input list");
		defList.emplace_back(listType,
							 GenericDataDefinition::Output,
							 "output",
							 "Output list");
		setupGenericObject<allSortableTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > ListData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[1]);

		assert(dataInput && dataOutput);

		const std::vector<T> &valIn = dataInput->getValue();
		auto valOut = dataOutput->getAccessor();

		std::set<T> tmpSet;

		valOut.clear();
		for(auto v : valIn)
		{
			if(tmpSet.find(v) == tmpSet.end())
			{
				tmpSet.insert(v);
				valOut.push_back(v);
			}
		}
	}

protected:
	GenericVectorData generic;
};

int RemoveDuplicatesClass = RegisterObject<RemoveDuplicates>("List/Remove duplicates").setDescription("Remove values that appear more than one time");

} // namespace Panda
