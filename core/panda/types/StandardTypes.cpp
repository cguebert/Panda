#include <panda/DataFactory.h>

#include <panda/types/StandardTypes.h>
#include <panda/types/Animation.inl>
#include <panda/types/AnimationTraits.h>

#include <QString>
#include <QVector>

namespace panda
{

namespace types
{

template<> PANDA_CORE_API QString DataTrait<int>::valueTypeName() { return "integer"; }
template<> PANDA_CORE_API QString DataTrait<PReal>::valueTypeName() { return "real"; }
template<> PANDA_CORE_API QString DataTrait<QString>::valueTypeName() { return "text"; }

//****************************************************************************//
// Overrides for writeValue xml

template<>
PANDA_CORE_API void DataTrait<int>::writeValue(QDomDocument&, QDomElement& elem, const int& v)
{ elem.setAttribute("int", v); }

template<>
PANDA_CORE_API void DataTrait<PReal>::writeValue(QDomDocument&, QDomElement& elem, const PReal& v)
{ elem.setAttribute("real", v); }

template<>
PANDA_CORE_API void DataTrait<QString>::writeValue(QDomDocument& doc, QDomElement& elem, const QString& v)
{
	QDomText node = doc.createTextNode(v);
	elem.appendChild(node);
}

//****************************************************************************//
// Overrides for readValue xml

template<>
PANDA_CORE_API void DataTrait<int>::readValue(QDomElement& elem, int& v)
{ v = elem.attribute("int").toInt(); }

template<>
PANDA_CORE_API void DataTrait<PReal>::readValue(QDomElement& elem, PReal& v)
#ifdef PANDA_DOUBLE
{ v = elem.attribute("real").toDouble(); }
#else
{ v = elem.attribute("real").toFloat(); }
#endif

template<>
PANDA_CORE_API void DataTrait<QString>::readValue(QDomElement& elem, QString& v)
{ v = elem.text(); }

//****************************************************************************//

template class Data<int>;
template class Data<PReal>;
template class Data<QString>;

template class Data< QVector<int> >;
template class Data< QVector<PReal> >;
template class Data< QVector<QString> >;

int intDataClass = RegisterData< int >();
int doubleDataClass = RegisterData< PReal >();
int stringDataClass = RegisterData< QString >();

int intVectorDataClass = RegisterData< QVector<int> >();
int doubleVectorDataClass = RegisterData< QVector<PReal> >();
int stringVectorDataClass = RegisterData< QVector<QString> >();

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
