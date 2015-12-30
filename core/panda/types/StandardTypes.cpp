#include <panda/DataFactory.h>

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
template<> PANDA_CORE_API std::string DataTrait<PReal>::valueTypeName() { return "real"; }
template<> PANDA_CORE_API std::string DataTrait<std::string>::valueTypeName() { return "text"; }

//****************************************************************************//
// Overrides for writeValue xml

template<>
PANDA_CORE_API void DataTrait<int>::writeValue(XmlElement& elem, const int& v)
{ elem.setAttribute("int", v); }

template<>
PANDA_CORE_API void DataTrait<PReal>::writeValue(XmlElement& elem, const PReal& v)
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
PANDA_CORE_API void DataTrait<PReal>::readValue(XmlElement& elem, PReal& v)
#ifdef PANDA_DOUBLE
{ v = elem.attribute("real").toDouble(); }
#else
{ v = elem.attribute("real").toFloat(); }
#endif

template<>
PANDA_CORE_API void DataTrait<std::string>::readValue(XmlElement& elem, std::string& v)
{ v = elem.text(); }

//****************************************************************************//

template class Data<int>;
template class Data<PReal>;
template class Data<std::string>;

template class Data< std::vector<int> >;
template class Data< std::vector<PReal> >;
template class Data< std::vector<std::string> >;

int intDataClass = RegisterData< int >();
int doubleDataClass = RegisterData< PReal >();
int stringDataClass = RegisterData< std::string >();

int intVectorDataClass = RegisterData< std::vector<int> >();
int doubleVectorDataClass = RegisterData< std::vector<PReal> >();
int stringVectorDataClass = RegisterData< std::vector<std::string> >();

template class Animation<PReal>;
template class Data< Animation<PReal> >;
int realAnimationDataClass = RegisterData< Animation<PReal> >();

} // namespace types

} // namespace panda

void convertType(const int& from, PReal& to)
{ to = static_cast<PReal>(from); }
void convertType(const PReal& from, int& to)
{ to = static_cast<int>(from); }

panda::types::RegisterTypeConverter<int, PReal> intRealConverter;
panda::types::RegisterTypeConverter<PReal, int> realIntConverter;
