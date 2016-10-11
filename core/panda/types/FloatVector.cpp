#include <panda/data/DataFactory.h>

#include <panda/types/FloatVector.h>
#include <panda/types/IntVector.h>

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

	template<> PANDA_CORE_API std::string DataTrait<FloatVector>::valueTypeName() { return "reals vector"; }
	template<> PANDA_CORE_API std::string DataTrait<FloatVector>::typeName() { return "realvector"; }
	template<> PANDA_CORE_API std::string DataTrait<FloatVector>::typeDescription() { return "vector of reals"; }
	template<> PANDA_CORE_API std::string DataTrait<std::vector<FloatVector>>::typeDescription() { return "vector of vectors of reals"; }
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
	PANDA_CORE_API void DataTrait<FloatVector>::readValue(const XmlElement& elem, FloatVector& floats)
	{
		auto& values = floats.values;
		values.clear();
		auto floatTrait = DataTraitsList::getTraitOf<float>();

		for(auto node = elem.firstChild("Float"); node; node = node.nextSibling("Float"))
		{
			float v;
			floatTrait->readValue(node, &v);
			values.push_back(v);
		}
	}

} // namespace types

template class PANDA_CORE_API Data<types::FloatVector>;
template class PANDA_CORE_API Data<std::vector<types::FloatVector>>;

int floatsDataClass = RegisterData<types::FloatVector>();
int floatsVectorDataClass = RegisterData<std::vector<types::FloatVector>>();

} // namespace panda

using panda::types::FloatVector;
using panda::types::IntVector;

void convertType(const FloatVector& from, std::vector<float>& to)
{ to = from.values; }
void convertType(const std::vector<float>& from, FloatVector& to)
{ to.values = from; }

void convertType(const IntVector& from, FloatVector& to)
{
	const auto& fromValues = from.values;
	auto& toValues = to.values;
	size_t nb = fromValues.size();
	toValues.resize(nb);
	for (size_t i = 0; i < nb; ++i)
		toValues[i] = static_cast<float>(fromValues[i]);
}

void convertType(const FloatVector& from, IntVector& to)
{
	const auto& fromValues = from.values;
	auto& toValues = to.values;
	size_t nb = fromValues.size();
	toValues.resize(nb);
	for (size_t i = 0; i < nb; ++i)
		toValues[i] = static_cast<int>(fromValues[i]);
}

panda::types::RegisterTypeConverter<FloatVector, std::vector<float> > FloatsToVectorConverter;
panda::types::RegisterTypeConverter<std::vector<float>, FloatVector> VectorToFloatsConverter;

panda::types::RegisterTypeConverter<IntVector, FloatVector> IntsToFloatsConverter;
panda::types::RegisterTypeConverter<FloatVector, IntVector> FloatsToIntsConverter;