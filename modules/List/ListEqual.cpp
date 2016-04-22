#include <panda/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/GenericObject.h>
#include <panda/types/typesLists.h>

#include <algorithm>

namespace panda {

using types::DataTypeId;
using types::IntVector;

class ListEqual : public GenericObject
{
public:
	PANDA_CLASS(ListEqual, GenericObject)

	ListEqual(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the lists to compare"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(0),
											 true, false,
											 "input A",
											 "First value"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(0),
											 true, false,
											 "input B",
											 "Second value"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfSingleValue(DataTypeId::getIdOf<int>()),
											 false, true,
											 "equal",
											 "1 if the two values are equal, 0 otherwise"));
		setupGenericObject<allDataTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		using ValVecData = Data<std::vector<T>>;
		using IntData = Data<int>;
		auto dataInputA = dynamic_cast<ValVecData*>(list[0]);
		auto dataInputB = dynamic_cast<ValVecData*>(list[1]);
		auto dataOutput = dynamic_cast<IntData*>(list[2]);

		assert(dataInputA && dataInputB && dataOutput);

		const auto& inputA = dataInputA->getValue();
		const auto& inputB = dataInputB->getValue();

		dataOutput->setValue((inputA == inputB) ? 1 : 0);
	}

protected:
	GenericVectorData generic;
};

int ListEqualClass = RegisterObject<ListEqual>("List/Lists equal").setDescription("Test if two lists are equal");

//****************************************************************************//

class ListItemsEqual : public GenericObject
{
public:
	PANDA_CLASS(ListItemsEqual, GenericObject)

	ListItemsEqual(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the lists to compare"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(0),
											 true, false,
											 "input A",
											 "First value"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(0),
											 true, false,
											 "input B",
											 "Second value"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<int>()),
											 false, true,
											 "equal",
											 "For each item: 1 if the two values are equal, 0 otherwise"));
		setupGenericObject<allDataTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		using ValVecData = Data<std::vector<T>>;
		using IntVecData = Data<std::vector<int>>;
		auto dataInputA = dynamic_cast<ValVecData*>(list[0]);
		auto dataInputB = dynamic_cast<ValVecData*>(list[1]);
		auto dataOutput = dynamic_cast<IntVecData*>(list[2]);

		assert(dataInputA && dataInputB && dataOutput);

		const auto& inputA = dataInputA->getValue();
		const auto& inputB = dataInputB->getValue();
		auto outputAcc = dataOutput->getAccessor();
		auto& output = outputAcc.wref();
		output.clear();

		int nbA = inputA.size(), nbB = inputB.size();
		if (!nbA || !nbB)
			return;

		int nb = std::max(nbA, nbB);
		if (nbA < nb) nbA = 1;
		if (nbB < nb) nbB = 1;

		output.reserve(nb);
		for (int i = 0; i < nb; ++i)
		{
			const auto& valA = inputA[i % nbA];
			const auto& valB = inputB[i % nbB];
			output.push_back((valA == valB) ? 1 : 0);
		}
	}

protected:
	GenericVectorData generic;
};

int ListItemsEqualClass = RegisterObject<ListItemsEqual>("List/List items equal").setDescription("Test if each item of two lists are equal");

//****************************************************************************//

class VectorListItemsEqual : public GenericObject
{
public:
	PANDA_CLASS(VectorListItemsEqual, GenericObject)

	VectorListItemsEqual(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the lists to compare"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(0),
												true, false,
												"input A",
												"First value"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(0),
												true, false,
												"input B",
												"Second value"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<IntVector>()),
												false, true,
												"equal",
												"For each item: 1 if the two values are equal, 0 otherwise"));
		setupGenericObject<allListsVectorTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		using ValVec = std::vector<T>;
		using ValVecData = Data<ValVec>;
		using VecIntVec = std::vector<IntVector>;
		using VecIntVecData = Data<VecIntVec>;
		auto dataInputA = dynamic_cast<ValVecData*>(list[0]);
		auto dataInputB = dynamic_cast<ValVecData*>(list[1]);
		auto dataOutput = dynamic_cast<VecIntVecData*>(list[2]);

		assert(dataInputA && dataInputB && dataOutput);

		const auto& inputA = dataInputA->getValue();
		const auto& inputB = dataInputB->getValue();
		auto outputAcc = dataOutput->getAccessor();
		auto& output = outputAcc.wref();
		output.clear();

		int nbA = inputA.size(), nbB = inputB.size();
		if (!nbA || !nbB)
			return;

		int nb = std::max(nbA, nbB);
		if (nbA < nb) nbA = 1;
		if (nbB < nb) nbB = 1;

		output.resize(nb);
		for (int i = 0; i < nb; ++i)
		{
			const auto& vecA = inputA[i % nbA].values;
			const auto& vecB = inputB[i % nbB].values;

			int nbVecA = vecA.size(), nbVecB = vecB.size();
			int nbVec = std::max(nbVecA, nbVecB);
			if (nbVecA < nbVec) nbVecA = 1;
			if (nbVecB < nbVec) nbVecB = 1;
			for (int j = 0; j < nbVec; ++j)
			{
				const auto& valA = vecA[j % nbA];
				const auto& valB = vecB[j % nbB];
				output[i].values.push_back((valA == valB) ? 1 : 0);
			}
		}
	}

protected:
	GenericVectorData generic;
};

int VectorListItemsEqualClass = RegisterObject<VectorListItemsEqual>("List/Vectors list/Vector lists items equal")
	.setDescription("Test if each item of two vectors lists are equal");
	
//****************************************************************************//

class VectorListIndicesEqual : public GenericObject
{
public:
	PANDA_CLASS(VectorListIndicesEqual, GenericObject)

	VectorListIndicesEqual(PandaDocument *doc)
		: GenericObject(doc)
		, m_indices(initData("indices", "Indices to create the lists"))
		, generic(initData("input", "Connect here the lists to compare"))
	{
		addInput(m_indices);
		addInput(generic);

		GenericDataDefinitionList defList;
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(0),
												true, false,
												"input A",
												"First value"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(0),
												true, false,
												"input B",
												"Second value"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<int>()),
												false, true,
												"equal",
												"For each pair of lists: 1 if the two lists are equal, 0 otherwise"));
		setupGenericObject<allDataTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		using ValVec = std::vector<T>;
		using ValVecData = Data<ValVec>;
		using VecInt = std::vector<int>;
		using VecIntData = Data<VecInt>;
		auto dataInputA = dynamic_cast<ValVecData*>(list[0]);
		auto dataInputB = dynamic_cast<ValVecData*>(list[1]);
		auto dataOutput = dynamic_cast<VecIntData*>(list[2]);

		assert(dataInputA && dataInputB && dataOutput);

		const auto& indicesList = m_indices.getValue();
		const auto& inputA = dataInputA->getValue();
		const auto& inputB = dataInputB->getValue();
		auto outputAcc = dataOutput->getAccessor();
		auto& output = outputAcc.wref();
		output.clear();

		int nbA = inputA.size(), nbB = inputB.size();
		int nbIL = indicesList.size();
		output.resize(nbIL);

		for (int i = 0; i < nbIL; ++i)
		{
			const auto& indices = indicesList[i].values;
			bool equal = true;
			int nbI = indices.size();
			for (int j = 0; j < nbI; ++j)
			{
				auto id = indices[j];
				if (id >= nbA || id >= nbB ||	// Index outside one of the lists
					inputA[id] != inputB[id])	// Values inequal
				{
					equal = false;
					break;
				}
			}

			output[i] = equal;
		}
	}

protected:
	Data<std::vector<IntVector>> m_indices;
	GenericVectorData generic;
};

int VectorListIndicesEqualClass = RegisterObject<VectorListIndicesEqual>("List/Vectors list/Lists equal using indices")
	.setDescription("Test if two lists created using indices are equal");

} // namespace Panda
