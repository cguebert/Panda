#include <panda/data/DataFactory.h>

#include <panda/types/StandardTypes.h>
#include <panda/types/Animation.inl>
#include <panda/types/AnimationTraits.h>

#include <string>
#include <vector>

namespace panda
{

namespace types
{

template<> PANDA_CORE_API std::string DataTrait<int>::valueTypeName() { return "integer"; }
template<> PANDA_CORE_API std::string DataTrait<float>::valueTypeName() { return "real"; }
template<> PANDA_CORE_API std::string DataTrait<std::string>::valueTypeName() { return "text"; }

//****************************************************************************//
// Overrides for writeValue xml

template<>
PANDA_CORE_API void DataTrait<int>::writeValue(XmlElement& elem, const int& v)
{ elem.setAttribute("int", v); }

template<>
PANDA_CORE_API void DataTrait<float>::writeValue(XmlElement& elem, const float& v)
{ elem.setAttribute("real", v); }

template<>
PANDA_CORE_API void DataTrait<std::string>::writeValue(XmlElement& elem, const std::string& v)
{ elem.setText(v); }

//****************************************************************************//
// Overrides for readValue xml

template<>
PANDA_CORE_API void DataTrait<int>::readValue(XmlElement& elem, int& v)
{ v = elem.attribute("int").toInt(); }

template<>
PANDA_CORE_API void DataTrait<float>::readValue(XmlElement& elem, float& v)
{ v = elem.attribute("real").toFloat(); }

template<>
PANDA_CORE_API void DataTrait<std::string>::readValue(XmlElement& elem, std::string& v)
{ v = elem.text(); }

//****************************************************************************//

template class Data<int>;
template class Data<float>;
template class Data<std::string>;

template class Data< std::vector<int> >;
template class Data< std::vector<float> >;
template class Data< std::vector<std::string> >;

int intDataClass = RegisterData< int >();
int doubleDataClass = RegisterData< float >();
int stringDataClass = RegisterData< std::string >();

int intVectorDataClass = RegisterData< std::vector<int> >();
int doubleVectorDataClass = RegisterData< std::vector<float> >();
int stringVectorDataClass = RegisterData< std::vector<std::string> >();

template class Animation<float>;
template class Data< Animation<float> >;
int realAnimationDataClass = RegisterData< Animation<float> >();

} // namespace types

} // namespace panda

void convertType(const int& from, float& to)
{ to = static_cast<float>(from); }
void convertType(const float& from, int& to)
{ to = static_cast<int>(from); }

panda::types::RegisterTypeConverter<int, float> intRealConverter;
panda::types::RegisterTypeConverter<float, int> realIntConverter;
