#include <panda/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/GenericObject.h>

#include <algorithm>

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

		std::sort(tmpList.begin(), tmpList.end());

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
