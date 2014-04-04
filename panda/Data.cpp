#include <panda/helper/system/Config.h>
#include <panda/DataFactory.h>
#include <panda/Data.inl>
#include <panda/types/TypeConverter.h>

#include <QColor>
#include <QString>

#include <QVector>

namespace panda
{

namespace types
{

template<> QString DataTrait<int>::valueTypeName() { return "integer"; }
template<> QString DataTrait<PReal>::valueTypeName() { return "real"; }
template<> QString DataTrait<QColor>::valueTypeName() { return "color"; }
template<> QString DataTrait<QString>::valueTypeName() { return "text"; }

//***************************************************************//
// Overrides for writeValue xml

template<>
void DataTrait<int>::writeValue(QDomDocument&, QDomElement& elem, const int& v)
{ elem.setAttribute("int", v); }

template<>
void DataTrait<PReal>::writeValue(QDomDocument&, QDomElement& elem, const PReal& v)
{ elem.setAttribute("real", v); }

template<>
void DataTrait<QColor>::writeValue(QDomDocument&, QDomElement& elem, const QColor& v)
{	elem.setAttribute("r", v.red());
	elem.setAttribute("g", v.green());
	elem.setAttribute("b", v.blue());
	elem.setAttribute("a", v.alpha()); }

template<>
void DataTrait<QString>::writeValue(QDomDocument& doc, QDomElement& elem, const QString& v)
{
	QDomText node = doc.createTextNode(v);
	elem.appendChild(node);
}

//***************************************************************//
// Overrides for readValue xml

template<>
void DataTrait<int>::readValue(QDomElement& elem, int& v)
{ v = elem.attribute("int").toInt(); }

template<>
void DataTrait<PReal>::readValue(QDomElement& elem, PReal& v)
#ifdef PANDA_DOUBLE
{ v = elem.attribute("real").toDouble(); }
#else
{ v = elem.attribute("real").toFloat(); }
#endif

template<>
void DataTrait<QColor>::readValue(QDomElement& elem, QColor& v)
{	v.setRed(  elem.attribute("r").toInt());
	v.setGreen(elem.attribute("g").toInt());
	v.setBlue( elem.attribute("b").toInt());
	v.setAlpha(elem.attribute("a").toInt()); }

template<>
void DataTrait<QString>::readValue(QDomElement& elem, QString& v)
{
	v = elem.text();
}

//***************************************************************//

template class Data<int>;
template class Data<PReal>;
template class Data<QColor>;
template class Data<QString>;

template class Data< QVector<int> >;
template class Data< QVector<PReal> >;
template class Data< QVector<QColor> >;
template class Data< QVector<QString> >;

int intDataClass = RegisterData< int >();
int doubleDataClass = RegisterData< PReal >();
int colorDataClass = RegisterData< QColor >();
int stringDataClass = RegisterData< QString >();

int intVectorDataClass = RegisterData< QVector<int> >();
int doubleVectorDataClass = RegisterData< QVector<PReal> >();
int colorVectorDataClass = RegisterData< QVector<QColor> >();
int stringVectorDataClass = RegisterData< QVector<QString> >();

} // namespace types

} // namespace panda

void convertType(const int& from, PReal& to)
{ to = static_cast<PReal>(from); }
void convertType(const PReal& from, int& to)
{ to = static_cast<int>(from); }

panda::types::RegisterTypeConverter<int, PReal> intRealConverter;
panda::types::RegisterTypeConverter<PReal, int> realIntConverter;
