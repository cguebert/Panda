#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/typesLists.h>

namespace panda {

using typesSupportingAddition = std::tuple<int, float, types::Color, types::Point>;

class ListSumFromIndices : public GenericObject
{
public:
	PANDA_CLASS(ListSumFromIndices, GenericObject)

	ListSumFromIndices(PandaDocument *doc)
		: GenericObject(doc)
		, m_indices(initData("indices", "Vectors of 0-based indices into the connected lists"))
		, m_generic(initData("input", "Connect here the lists to get the items from"))
	{
		addInput(m_indices);
		addInput(m_generic);

		int typeOfList = types::DataTypeId::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Input,
							 "input",
							 "List from which to extract values");
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Output,
							 "output",
							 "Sum of the values extracted from the list");

		setupGenericObject<typesSupportingAddition>(this, m_generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > VecData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[1]);
		assert(dataInput && dataOutput);

		const auto& indices = m_indices.getValue();
		const std::vector<T>& inVal = dataInput->getValue();
		auto outVal = dataOutput->getAccessor();

		int size = inVal.size();
		int nb = indices.size();
		outVal.resize(nb);
		if(size)
		{
			for (int i = 0; i < nb; ++i)
			{
				auto& val = outVal[i];
				val = T();
				for (auto id : indices[i].values)
				{
					if (id >= 0 && id < size)
						val += inVal[id];
				}
			}
		}
	}

protected:
	Data<std::vector<types::IntVector>> m_indices;
	GenericVectorData m_generic;
};

int ListSumFromIndicesClass = RegisterObject<ListSumFromIndices>("List/Sum using indices").setDescription("Compute a sum for each vector of indices");

//****************************************************************************//

class ListMeanFromIndices : public GenericObject
{
public:
	PANDA_CLASS(ListMeanFromIndices, GenericObject)

		ListMeanFromIndices(PandaDocument *doc)
		: GenericObject(doc)
		, m_indices(initData("indices", "Vectors of 0-based indices into the connected lists"))
		, m_generic(initData("input", "Connect here the lists to get the items from"))
	{
		addInput(m_indices);
		addInput(m_generic);

		int typeOfList = types::DataTypeId::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Input,
							 "input",
							 "List from which to extract values");
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Output,
							 "output",
							 "Mean of the values extracted from the list");

		setupGenericObject<typesSupportingAddition>(this, m_generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > VecData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[1]);
		assert(dataInput && dataOutput);

		const auto& indices = m_indices.getValue();
		const std::vector<T>& inVal = dataInput->getValue();
		auto outVal = dataOutput->getAccessor();

		int size = inVal.size();
		int nb = indices.size();
		outVal.resize(nb);
		if (size)
		{
			for (int i = 0; i < nb; ++i)
			{
				auto& val = outVal[i];
				val = T();
				for (auto id : indices[i].values)
				{
					if (id >= 0 && id < size)
						val += inVal[id];
				}

				int s = indices[i].values.size();
				if(s)
					val = static_cast<T>(val * (1.f / s)); // Cast here to remove a warning and because most of panda types use only floats in operators
			}
		}
	}

protected:
	Data<std::vector<types::IntVector>> m_indices;
	GenericVectorData m_generic;
};

int ListMeanFromIndicesClass = RegisterObject<ListMeanFromIndices>("List/Mean using indices").setDescription("Compute the mean for each vector of indices");

} // namespace Panda
