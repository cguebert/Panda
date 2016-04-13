#include <panda/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/GenericObject.h>
#include <panda/types/typesLists.h>

#include <algorithm>

namespace panda {

using types::DataTypeId;

class ListFindMin : public GenericObject
{
public:
	PANDA_CLASS(ListFindMin, GenericObject)

	ListFindMin(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the lists to get the items from"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(0),
											 true, false,
											 "input",
											 "List in which to look for the minimum value"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfSingleValue(0),
											 false, true,
											 "minimum",
											 "Minimum value"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfSingleValue(DataTypeId::getIdOf<int>()),
											 false, true,
											 "indices",
											 "Index corresponding to the minimum value"));
		setupGenericObject<allSortableTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		using ValVec = std::vector<T>;
		using ValVecData = Data<ValVec>;
		using ValData = Data<T>;
		using IntData = Data<int>;
		auto dataInput = dynamic_cast<ValVecData*>(list[0]);
		auto dataOutputValue = dynamic_cast<ValData*>(list[1]);
		auto dataOutputIndex = dynamic_cast<IntData*>(list[2]);

		assert(dataInput && dataOutputValue && dataOutputIndex);

		const auto& input = dataInput->getValue();
		if (input.empty())
		{
			dataOutputValue->setValue(T());
			dataOutputIndex->setValue(-1);
			return;
		}

		T minVal = input.front();
		int minId = 0;
		int nb = input.size();
		for (int i = 1; i < nb; ++i)
		{
			const auto& val = input[i];
			if (val < minVal)
			{
				minVal = val;
				minId = i;
			}
		}

		dataOutputValue->setValue(minVal);
		dataOutputIndex->setValue(minId);
	}

protected:
	GenericVectorData generic;
};

int ListFindMinClass = RegisterObject<ListFindMin>("List/Find minimum").setDescription("Find the minimum value in a list and its index");

//****************************************************************************//

class ListFindMax : public GenericObject
{
public:
	PANDA_CLASS(ListFindMax, GenericObject)

	ListFindMax(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the lists to get the items from"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(0),
												true, false,
												"input",
												"List in which to look for the minimum value"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfSingleValue(0),
												false, true,
												"maximum",
												"Maximum value"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfSingleValue(DataTypeId::getIdOf<int>()),
												false, true,
												"indices",
												"Index corresponding to the maximum value"));
		setupGenericObject<allSortableTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		using ValVec = std::vector<T>;
		using ValVecData = Data<ValVec>;
		using ValData = Data<T>;
		using IntData = Data<int>;
		auto dataInput = dynamic_cast<ValVecData*>(list[0]);
		auto dataOutputValue = dynamic_cast<ValData*>(list[1]);
		auto dataOutputIndex = dynamic_cast<IntData*>(list[2]);

		assert(dataInput && dataOutputValue && dataOutputIndex);

		const auto& input = dataInput->getValue();
		if (input.empty())
		{
			dataOutputValue->setValue(T());
			dataOutputIndex->setValue(-1);
			return;
		}

		T maxVal = input.front();
		int maxId = 0;
		int nb = input.size();
		for (int i = 1; i < nb; ++i)
		{
			const auto& val = input[i];
			if (maxVal < val)
			{
				maxVal = val;
				maxId = i;
			}
		}

		dataOutputValue->setValue(maxVal);
		dataOutputIndex->setValue(maxId);
	}

protected:
	GenericVectorData generic;
};

int ListFindMaxClass = RegisterObject<ListFindMax>("List/Find maximum").setDescription("Find the maximum value in a list and its index");

} // namespace Panda
