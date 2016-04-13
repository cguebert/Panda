#include <panda/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/GenericObject.h>
#include <panda/types/typesLists.h>

#include <algorithm>

namespace panda {

using types::DataTypeId;
using types::IntVector;

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
		setupGenericObject<allSortableTypes>(this, generic, defList);
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

//****************************************************************************//

class SortVectorsList : public GenericObject
{
public:
	PANDA_CLASS(SortVectorsList, GenericObject)

		SortVectorsList(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the vectors of lists"))
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
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<IntVector>()),
												false, true,
												"indices",
												"Indices corresponding to the sorted list"));
		setupGenericObject<allListsVectorTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		using ValVec = std::vector<T>;
		using IndVec = std::vector<IntVector>;
		using ValData = Data<ValVec>;
		using IndData =  Data<IndVec>;
		ValData* dataInput = dynamic_cast<ValData*>(list[0]);
		ValData* dataOutput = dynamic_cast<ValData*>(list[1]);
		IndData* dataIndices = dynamic_cast<IndData*>(list[2]);

		assert(dataInput && dataOutput && dataIndices);

		const auto& input = dataInput->getValue();
		auto outValAcc = dataOutput->getAccessor();
		auto outIndAcc = dataIndices->getAccessor();
		auto& outVal = outValAcc.wref();
		auto& outInd = outIndAcc.wref();

		int nbLists = input.size();
		outVal.clear();
		outInd.clear();
		outVal.reserve(nbLists);
		outInd.reserve(nbLists);

		for (const auto& inputList : input)
		{
			std::vector<std::pair<T::value_type, int>> tmpList;
			int nb = inputList.values.size();

			for (int i = 0; i < nb; ++i)
				tmpList.emplace_back(inputList.values[i], i);

			std::sort(tmpList.begin(), tmpList.end());

			T valVec;
			IntVector indVec;
			valVec.values.reserve(nb);
			indVec.values.reserve(nb);
			for (const auto& v : tmpList)
			{
				valVec.values.push_back(v.first);
				indVec.values.push_back(v.second);
			}

			outVal.push_back(valVec);
			outInd.push_back(indVec);
		}
	}

protected:
	GenericVectorData generic;
};

int SortVectorsListClass = RegisterObject<SortVectorsList>("List/Vectors list/Sort lists")
	.setName("Sort vectors list")
	.setDescription("Sort each list in a vector of lists");

} // namespace Panda
