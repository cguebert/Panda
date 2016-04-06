#include <panda/data/DataFactory.h>

#include <panda/types/FloatVector.h>

namespace panda
{

namespace types
{

	FloatVector FloatVector::operator+(const float& p) const
	{
		const int nb = values.size();
		std::vector<float> tmp(nb);
		for (int i = 0; i < nb; ++i)
			tmp[i] = values[i] + p;
		return tmp;
	}

	FloatVector FloatVector::operator-(const float& p) const
	{
		const int nb = values.size();
		std::vector<float> tmp(nb);
		for (int i = 0; i < nb; ++i)
			tmp[i] = values[i] - p;
		return tmp;
	}

	FloatVector FloatVector::operator*(float v) const
	{
		const int nb = values.size();
		std::vector<float> tmp(nb);
		for (int i = 0; i < nb; ++i)
			tmp[i] = values[i] * v;
		return tmp;
	}

	FloatVector FloatVector::operator/(float v) const
	{
		const int nb = values.size();
		std::vector<float> tmp(nb);
		for (int i = 0; i < nb; ++i)
			tmp[i] = values[i] / v;
		return tmp;
	}

	FloatVector operator*(float v, const FloatVector& p)
	{
		const int nb = p.values.size();
		std::vector<float> tmp(nb);
		for (int i = 0; i < nb; ++i)
			tmp[i] = p.values[i] * v;
		return tmp;
	}

	FloatVector FloatVector::reversed() const
	{
		auto tmp = values;
		std::reverse(tmp.begin(), tmp.end());
		return tmp;
	}

	void FloatVector::reverse()
	{
		std::reverse(values.begin(), values.end());
	}

	//****************************************************************************//

	template<> PANDA_CORE_API std::string DataTrait<FloatVector>::valueTypeName() { return "vector of floats"; }
	template<> PANDA_CORE_API std::string DataTrait<FloatVector>::valueTypeNamePlural() { return "vectors of floats"; }
	template<> PANDA_CORE_API unsigned int DataTrait<FloatVector>::typeColor() { return 0x719B7A; }

	template<>
	PANDA_CORE_API void DataTrait<FloatVector>::writeValue(XmlElement& elem, const FloatVector& floats)
	{
		auto floatTrait = DataTraitsList::getTraitOf<float>();
		for (const auto& pt : floats.values)
		{
			auto node = elem.addChild("Float");
			floatTrait->writeValue(node, &pt);
		}
	}

	template<>
	PANDA_CORE_API void DataTrait<FloatVector>::readValue(XmlElement& elem, FloatVector& floats)
	{
		auto& values = floats.values;
		values.clear();
		auto floatTrait = DataTraitsList::getTraitOf<float>();

		auto node = elem.firstChild("Float");
		while (node)
		{
			float v;
			floatTrait->readValue(node, &v);
			values.push_back(v);
			node = node.nextSibling("Float");
		}
	}

	template class PANDA_CORE_API Data< FloatVector >;
	template class PANDA_CORE_API Data< std::vector<FloatVector> >;

	int floatsDataClass = RegisterData< FloatVector >();
	int floatsVectorDataClass = RegisterData< std::vector<FloatVector> >();

} // namespace types

} // namespace panda

void convertType(const panda::types::FloatVector& from, std::vector<float>& to)
{ to = from.values; }
void convertType(const std::vector<float>& from, panda::types::FloatVector& to)
{ to.values = from; }

panda::types::RegisterTypeConverter<panda::types::FloatVector, std::vector<float> > FloatsToVectorConverter;
panda::types::RegisterTypeConverter<std::vector<float>, panda::types::FloatVector> VectorToFloatsConverter;