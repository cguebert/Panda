#include <panda/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/GenericObject.h>

#include <algorithm>

using panda::types::Color;
using panda::types::Point;
using panda::types::Rect;

bool operator<(const Color& lhs, const Color& rhs)
{
	return lhs.toHex() < rhs.toHex();
}

bool operator<(const Point& p1, const Point& p2)
{
	return p1.x < p2.x || (p1.x == p2.x && p1.y < p2.y);
}

bool operator<(const Rect& lhs, const Rect& rhs)
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

namespace
{

template <class T>
struct Comparator
{
	using pair_type = std::pair<T, int>;
	bool operator()(const pair_type& lhs, const pair_type& rhs)
	{
		return lhs.first < rhs.first;
	}
};

}

namespace panda {

using types::DataTypeId;

class SortList : public GenericObject
{
public:
	PANDA_CLASS(SortList, GenericObject)

	SortList(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the lists to get the items from"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		int listType = DataTypeId::getFullTypeOfVector(0);
		defList.push_back(GenericDataDefinition(listType,
											 true, false,
											 "input",
											 "List to sort"));
		defList.push_back(GenericDataDefinition(listType,
											 false, true,
											 "output",
											 "Sorted list"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<int>()),
											 false, true,
											 "indices",
											 "Indices corresponding to the sorted list"));
		setupGenericObject<allSearchableTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > VecData;
		typedef Data< std::vector<int> > VecIntData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[1]);
		VecIntData* dataIndices = dynamic_cast<VecIntData*>(list[2]);

		assert(dataInput && dataOutput && dataIndices);

		const std::vector<T> &valIn = dataInput->getValue();

		std::vector<std::pair<T, int>> tmpList;
		int nb = valIn.size();

		for(int i=0; i<nb; ++i)
			tmpList.emplace_back(valIn[i], i);

		std::sort(tmpList.begin(), tmpList.end(), Comparator<T>());

		auto outValAcc = dataOutput->getAccessor();
		auto outIndAcc = dataIndices->getAccessor();
		auto& outVal = outValAcc.wref();
		auto& outInd = outIndAcc.wref();
		outVal.clear();
		outInd.clear();
		outVal.reserve(nb);
		outInd.reserve(nb);
		for (const auto& v : tmpList)
		{
			outVal.push_back(v.first);
			outInd.push_back(v.second);
		}
	}

protected:
	GenericVectorData generic;
};

int SortListClass = RegisterObject<SortList>("List/Sort list").setDescription("Sort a list");

} // namespace Panda
