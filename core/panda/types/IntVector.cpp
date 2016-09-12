#include <panda/data/DataFactory.h>

#include <panda/types/IntVector.h>

namespace panda
{

namespace types
{

	IntVector IntVector::operator+(const int& p) const
	{
		const int nb = values.size();
		std::vector<int> tmp(nb);
		for (int i = 0; i < nb; ++i)
			tmp[i] = values[i] + p;
		return tmp;
	}

	IntVector IntVector::operator-(const int& p) const
	{
		const int nb = values.size();
		std::vector<int> tmp(nb);
		for (int i = 0; i < nb; ++i)
			tmp[i] = values[i] - p;
		return tmp;
	}

	IntVector IntVector::operator*(int v) const
	{
		const int nb = values.size();
		std::vector<int> tmp(nb);
		for (int i = 0; i < nb; ++i)
			tmp[i] = values[i] * v;
		return tmp;
	}

	IntVector IntVector::operator/(int v) const
	{
		const int nb = values.size();
		std::vector<int> tmp(nb);
		for (int i = 0; i < nb; ++i)
			tmp[i] = values[i] / v;
		return tmp;
	}

	IntVector operator*(int v, const IntVector& p)
	{
		const int nb = p.values.size();
		std::vector<int> tmp(nb);
		for (int i = 0; i < nb; ++i)
			tmp[i] = p.values[i] * v;
		return tmp;
	}

	IntVector IntVector::reversed() const
	{
		auto tmp = values;
		std::reverse(tmp.begin(), tmp.end());
		return tmp;
	}

	void IntVector::reverse()
	{
		std::reverse(values.begin(), values.end());
	}

	//****************************************************************************//

	template<> PANDA_CORE_API std::string DataTrait<IntVector>::valueTypeName() { return "integers vector"; }
	template<> PANDA_CORE_API std::string DataTrait<IntVector>::typeName() { return "intvector"; }
	template<> PANDA_CORE_API std::string DataTrait<IntVector>::typeDescription() { return "vector of integers"; }
	template<> PANDA_CORE_API std::string DataTrait<std::vector<IntVector>>::typeDescription() { return "vector of vectors of integers"; }
	template<> PANDA_CORE_API unsigned int DataTrait<IntVector>::typeColor() { return 0x707020; }

	template<>
	PANDA_CORE_API void DataTrait<IntVector>::writeValue(XmlElement& elem, const IntVector& ints)
	{
		auto intTrait = DataTraitsList::getTraitOf<int>();
		for (const auto& pt : ints.values)
		{
			auto node = elem.addChild("Int");
			intTrait->writeValue(node, &pt);
		}
	}

	template<>
	PANDA_CORE_API void DataTrait<IntVector>::readValue(const XmlElement& elem, IntVector& ints)
	{
		auto& values = ints.values;
		values.clear();
		auto intTrait = DataTraitsList::getTraitOf<int>();

		for(auto node = elem.firstChild("Int"); node; node = node.nextSibling("Int"))
		{
			int v;
			intTrait->readValue(node, &v);
			values.push_back(v);
		}
	}

	template class PANDA_CORE_API Data< IntVector >;
	template class PANDA_CORE_API Data< std::vector<IntVector> >;

	int intsDataClass = RegisterData< IntVector >();
	int intsVectorDataClass = RegisterData< std::vector<IntVector> >();

} // namespace types

} // namespace panda

void convertType(const panda::types::IntVector& from, std::vector<int>& to)
{ to = from.values; }
void convertType(const std::vector<int>& from, panda::types::IntVector& to)
{ to.values = from; }

panda::types::RegisterTypeConverter<panda::types::IntVector, std::vector<int> > IntsToVectorConverter;
panda::types::RegisterTypeConverter<std::vector<int>, panda::types::IntVector> VectorToIntsConverter;