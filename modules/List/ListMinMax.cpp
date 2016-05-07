#include <panda/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/GenericObject.h>
#include <panda/types/typesLists.h>

#include <algorithm>

namespace panda {

using types::DataTypeId;
using types::IntVector;

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
		defList.emplace_back(DataTypeId::getFullTypeOfVector(0),
							 GenericDataDefinition::Input,
							 "input",
							 "List in which to look for the minimum value");
		defList.emplace_back(DataTypeId::getFullTypeOfSingleValue(0),
							 GenericDataDefinition::Output,
							 "minimum",
							 "Minimum value");
		defList.emplace_back(DataTypeId::getFullTypeOfSingleValue(DataTypeId::getIdOf<int>()),
							 GenericDataDefinition::Output,
							 "index",
							 "Index corresponding to the minimum value");
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

int ListFindMinClass = RegisterObject<ListFindMin>("List/Find minimum").setDescription("Find the minimum value and its index in a list");

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
		defList.emplace_back(DataTypeId::getFullTypeOfVector(0),
							 GenericDataDefinition::Input,
							 "input",
							 "List in which to look for the minimum value");
		defList.emplace_back(DataTypeId::getFullTypeOfSingleValue(0),
							 GenericDataDefinition::Output,
							 "maximum",
							 "Maximum value");
		defList.emplace_back(DataTypeId::getFullTypeOfSingleValue(DataTypeId::getIdOf<int>()),
							 GenericDataDefinition::Output,
							 "index",
							 "Index corresponding to the maximum value");
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

int ListFindMaxClass = RegisterObject<ListFindMax>("List/Find maximum").setDescription("Find the maximum value and its index in a list");

//****************************************************************************//

class VectorListFindMin : public GenericObject
{
public:
	PANDA_CLASS(VectorListFindMin, GenericObject)

		VectorListFindMin(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the lists to get the items from"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		defList.emplace_back(DataTypeId::getFullTypeOfVector(0),
							 GenericDataDefinition::Input,
							 "input",
							 "Lists in which to look for the minimum value");
		defList.emplace_back(DataTypeId::getFullTypeOfSingleValue(0),
							 GenericDataDefinition::Output,
							 "minimum",
							 "Minimum values");
		defList.emplace_back(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<int>()),
							 GenericDataDefinition::Output,
							 "indices",
							 "Indices corresponding to the minimum values");
		setupGenericObject<allListsVectorTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		using ValType = typename T::value_type;
		using ValVec = std::vector<T>;
		using ValVecData = Data<ValVec>;
		using ValData = Data<T>;
		using VecInt = std::vector<int>;
		using VecIntData = Data<VecInt>;
		auto dataInput = dynamic_cast<ValVecData*>(list[0]);
		auto dataOutputValue = dynamic_cast<ValData*>(list[1]);
		auto dataOutputIndex = dynamic_cast<VecIntData*>(list[2]);

		assert(dataInput && dataOutputValue && dataOutputIndex);

		const auto& input = dataInput->getValue();
		auto outputValAcc = dataOutputValue->getAccessor();
		auto outputIndAcc = dataOutputIndex->getAccessor();
		auto& outputVal = outputValAcc.wref().values;
		auto& outputInd = outputIndAcc.wref();
		outputVal.clear();
		outputInd.clear();

		for (const auto& inputList : input)
		{
			const auto& values = inputList.values;
			if (values.empty())
			{
				outputVal.push_back(ValType());
				outputInd.push_back(-1);
				continue;
			}

			auto minVal = values.front();
			int minId = 0;
			int nb = values.size();
			for (int i = 1; i < nb; ++i)
			{
				const auto& val = values[i];
				if (val < minVal)
				{
					minVal = val;
					minId = i;
				}
			}

			outputVal.push_back(minVal);
			outputInd.push_back(minId);
		}
	}

protected:
	GenericVectorData generic;
};

int VectorListFindMinClass = RegisterObject<VectorListFindMin>("List/Vectors list/Minimum in lists")
	.setName("Minimum in vectors list")
	.setDescription("Find the minimum value and its index in each list");

//****************************************************************************//

class VectorListFindMax : public GenericObject
{
public:
	PANDA_CLASS(VectorListFindMax, GenericObject)

	VectorListFindMax(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the lists to get the items from"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		defList.emplace_back(DataTypeId::getFullTypeOfVector(0),
							 GenericDataDefinition::Input,
							 "input",
							 "Lists in which to look for the maximum value");
		defList.emplace_back(DataTypeId::getFullTypeOfSingleValue(0),
							 GenericDataDefinition::Output,
							 "maximum",
							 "Maximum values");
		defList.emplace_back(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<int>()),
							 GenericDataDefinition::Output,
							 "indices",
							 "Indices corresponding to the maximum values");
		setupGenericObject<allListsVectorTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		using ValType = typename T::value_type;
		using ValVec = std::vector<T>;
		using ValVecData = Data<ValVec>;
		using ValData = Data<T>;
		using VecInt = std::vector<int>;
		using VecIntData = Data<VecInt>;
		auto dataInput = dynamic_cast<ValVecData*>(list[0]);
		auto dataOutputValue = dynamic_cast<ValData*>(list[1]);
		auto dataOutputIndex = dynamic_cast<VecIntData*>(list[2]);

		assert(dataInput && dataOutputValue && dataOutputIndex);

		const auto& input = dataInput->getValue();
		auto outputValAcc = dataOutputValue->getAccessor();
		auto outputIndAcc = dataOutputIndex->getAccessor();
		auto& outputVal = outputValAcc.wref().values;
		auto& outputInd = outputIndAcc.wref();
		outputVal.clear();
		outputInd.clear();

		for (const auto& inputList : input)
		{
			const auto& values = inputList.values;
			if (values.empty())
			{
				outputVal.push_back(ValType());
				outputInd.push_back(-1);
				continue;
			}

			auto maxVal = values.front();
			int maxId = 0;
			int nb = values.size();
			for (int i = 1; i < nb; ++i)
			{
				const auto& val = values[i];
				if (val > maxVal)
				{
					maxVal = val;
					maxId = i;
				}
			}

			outputVal.push_back(maxVal);
			outputInd.push_back(maxId);
		}
	}

protected:
	GenericVectorData generic;
};

int VectorListFindMaxClass = RegisterObject<VectorListFindMax>("List/Vectors list/Maximum in lists")
	.setName("Maximum in vectors list")
	.setDescription("Find the maximum value and its index in each list");

//****************************************************************************//

class VectorListIndicesMin : public GenericObject
{
public:
	PANDA_CLASS(VectorListIndicesMin, GenericObject)

	VectorListIndicesMin(PandaDocument *doc)
		: GenericObject(doc)
		, m_indices(initData("indices", "Indices to create the lists"))
		, generic(initData("input", "Connect here the lists to get the items from"))
	{
		addInput(m_indices);
		addInput(generic);

		GenericDataDefinitionList defList;
		defList.emplace_back(DataTypeId::getFullTypeOfVector(0),
							 GenericDataDefinition::Input,
							 "input",
							 "List in which to look for the minimum value");
		defList.emplace_back(DataTypeId::getFullTypeOfVector(0),
							 GenericDataDefinition::Output,
							 "minimum",
							 "Minimum values");
		defList.emplace_back(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<int>()),
							 GenericDataDefinition::Output,
							 "indices",
							 "Indices in the input of the minimum values");
		defList.emplace_back(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<int>()),
							 GenericDataDefinition::Output,
							 "position",
							 "Position of the minimum value in each list ");
		setupGenericObject<allSortableTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		using ValVec = std::vector<T>;
		using ValVecData = Data<ValVec>;
		using VecInt = std::vector<int>;
		using VecIntData = Data<VecInt>;
		auto dataInput = dynamic_cast<ValVecData*>(list[0]);
		auto dataOutputValue = dynamic_cast<ValVecData*>(list[1]);
		auto dataOutputIndex = dynamic_cast<VecIntData*>(list[2]);
		auto dataOutputPosition = dynamic_cast<VecIntData*>(list[3]);

		assert(dataInput && dataOutputValue && dataOutputIndex && dataOutputPosition);

		const auto& input = dataInput->getValue();
		auto outputValAcc = dataOutputValue->getAccessor();
		auto outputIndAcc = dataOutputIndex->getAccessor();
		auto outputPosAcc = dataOutputPosition->getAccessor();
		auto& outputVal = outputValAcc.wref();
		auto& outputInd = outputIndAcc.wref();
		auto& outputPos = outputPosAcc.wref();
		outputVal.clear();
		outputInd.clear();
		outputPos.clear();

		const int nbValues = input.size();
		const auto& indicesLists = m_indices.getValue();
		for (const auto& indicesList : indicesLists)
		{
			const auto& indices = indicesList.values;
			if (indices.empty())
			{
				outputVal.push_back(T());
				outputInd.push_back(-1);
				outputPos.push_back(-1);
				continue;
			}

			int id = helper::bound(0, indices.front(), nbValues - 1);
			auto minVal = input[id];
			int minPos = 0, minId = id;
			int nb = indices.size();
			for (int i = 1; i < nb; ++i)
			{
				id = helper::bound(0, indices[i], nbValues - 1);
				const auto& val = input[id];
				if (val < minVal)
				{
					minVal = val;
					minId = id;
					minPos = i;
				}
			}

			outputVal.push_back(minVal);
			outputInd.push_back(minId);
			outputPos.push_back(minPos);
		}
	}

protected:
	Data<std::vector<IntVector>> m_indices;
	GenericVectorData generic;
};

int VectorListIndicesMainClass = RegisterObject<VectorListIndicesMin>("List/Vectors list/Minimum using indices")
	.setName("Minimum in lists with indices")
	.setDescription("Find the minimum value and its index in each list created using given indices");

//****************************************************************************//

class VectorListIndicesMax : public GenericObject
{
public:
	PANDA_CLASS(VectorListIndicesMax, GenericObject)

	VectorListIndicesMax(PandaDocument *doc)
		: GenericObject(doc)
		, m_indices(initData("indices", "Indices to create the vectors lists"))
		, generic(initData("input", "Connect here the lists to get the items from"))
	{
		addInput(m_indices);
		addInput(generic);

		GenericDataDefinitionList defList;
		defList.emplace_back(DataTypeId::getFullTypeOfVector(0),
							 GenericDataDefinition::Input,
							 "input",
							 "List in which to look for the maximum value");
		defList.emplace_back(DataTypeId::getFullTypeOfVector(0),
							 GenericDataDefinition::Output,
							 "maximum",
							 "Maximum values");
		defList.emplace_back(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<int>()),
							 GenericDataDefinition::Output,
							 "indices",
							 "Indices in the input of the maximum values");
		defList.emplace_back(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<int>()),
							 GenericDataDefinition::Output,
							 "position",
							 "Position of the maximum value in each list");
		setupGenericObject<allSortableTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		using ValVec = std::vector<T>;
		using ValVecData = Data<ValVec>;
		using VecInt = std::vector<int>;
		using VecIntData = Data<VecInt>;
		auto dataInput = dynamic_cast<ValVecData*>(list[0]);
		auto dataOutputValue = dynamic_cast<ValVecData*>(list[1]);
		auto dataOutputIndex = dynamic_cast<VecIntData*>(list[2]);
		auto dataOutputPosition = dynamic_cast<VecIntData*>(list[3]);

		assert(dataInput && dataOutputValue && dataOutputIndex && dataOutputPosition);

		const auto& input = dataInput->getValue();
		auto outputValAcc = dataOutputValue->getAccessor();
		auto outputIndAcc = dataOutputIndex->getAccessor();
		auto outputPosAcc = dataOutputPosition->getAccessor();
		auto& outputVal = outputValAcc.wref();
		auto& outputInd = outputIndAcc.wref();
		auto& outputPos = outputPosAcc.wref();
		outputVal.clear();
		outputInd.clear();
		outputPos.clear();

		const int nbValues = input.size();
		const auto& indicesLists = m_indices.getValue();
		for (const auto& indicesList : indicesLists)
		{
			const auto& indices = indicesList.values;
			if (indices.empty())
			{
				outputVal.push_back(T());
				outputInd.push_back(-1);
				outputPos.push_back(-1);
				continue;
			}

			int id = helper::bound(0, indices.front(), nbValues - 1);
			auto maxVal = input[id];
			int maxPos = 0, maxId = id;
			int nb = indices.size();
			for (int i = 1; i < nb; ++i)
			{
				id = helper::bound(0, indices[i], nbValues - 1);
				const auto& val = input[id];
				if (maxVal < val)
				{
					maxVal = val;
					maxId = id;
					maxPos = i;
				}
			}

			outputVal.push_back(maxVal);
			outputInd.push_back(maxId);
			outputPos.push_back(maxPos);
		}
	}

protected:
	Data<std::vector<IntVector>> m_indices;
	GenericVectorData generic;
};

int VectorListIndicesMaxClass = RegisterObject<VectorListIndicesMax>("List/Vectors list/Maximum using indices")
	.setName("Maximum in lists with indices")
	.setDescription("Find the maximum value and its index in each list");

} // namespace Panda
