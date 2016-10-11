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
// Overrides for typeColor

template<> PANDA_CORE_API unsigned int DataTrait<int>::typeColor() { return 0xB8C16B; }
template<> PANDA_CORE_API unsigned int DataTrait<float>::typeColor() { return 0x68D083; }
template<> PANDA_CORE_API unsigned int DataTrait<std::string>::typeColor() { return 0x8ED33E; }

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
PANDA_CORE_API void DataTrait<int>::readValue(const XmlElement& elem, int& v)
{ v = elem.attribute("int").toInt(); }

template<>
PANDA_CORE_API void DataTrait<float>::readValue(const XmlElement& elem, float& v)
{ v = elem.attribute("real").toFloat(); }

template<>
PANDA_CORE_API void DataTrait<std::string>::readValue(const XmlElement& elem, std::string& v)
{ v = elem.text(); }

//****************************************************************************//

} // namespace types

template class PANDA_CORE_API Data<int>;
template class PANDA_CORE_API Data<float>;
template class PANDA_CORE_API Data<std::string>;

template class PANDA_CORE_API Data< std::vector<int> >;
template class PANDA_CORE_API Data< std::vector<float> >;
template class PANDA_CORE_API Data< std::vector<std::string> >;

int intDataClass = RegisterData< int >();
int doubleDataClass = RegisterData< float >();
int stringDataClass = RegisterData< std::string >();

int intVectorDataClass = RegisterData< std::vector<int> >();
int doubleVectorDataClass = RegisterData< std::vector<float> >();
int stringVectorDataClass = RegisterData< std::vector<std::string> >();

template class PANDA_CORE_API types::Animation<float>;
template class PANDA_CORE_API Data<types::Animation<float>>;
int realAnimationDataClass = RegisterData<types::Animation<float>>();

} // namespace panda

void convertType(const int& from, float& to)
{ to = static_cast<float>(from); }
void convertType(const float& from, int& to)
{ to = static_cast<int>(from); }

panda::types::RegisterTypeConverter<int, float> intRealConverter;
panda::types::RegisterTypeConverter<float, int> realIntConverter;
