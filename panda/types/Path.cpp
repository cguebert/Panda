#include <panda/types/Path.h>
#include <panda/Data.h>

#include <panda/DataFactory.h>
#include <panda/Data.inl>

namespace panda
{

namespace types
{

Path& Path::operator=(const QVector<QPointF> &v)
{
	QVector<QPointF>::operator=(v);
	return *this;
}

template<> QString DataTrait<Path>::valueTypeName() { return "path"; }
template<> int DataTrait<Path>::size(const Path& v) { return v.size(); }
template<> void DataTrait<Path>::clear(Path& v, int size, bool init)
{
	if(init)
		v.clear();
	v.resize(size);
}
/*
template<>
void DataTrait<Path>::writeValue(QDomDocument& doc, QDomElement& elem, const Gradient& grad)
{
	for(const auto& s : grad.getStops())
	{
		QDomElement stopNode = doc.createElement("Stop");
		elem.appendChild(stopNode);
		stopNode.setAttribute("pos", s.first);
		DataTrait<QColor>::writeValue(doc, stopNode, s.second);
	}
}

template<>
void DataTrait<Path>::readValue(QDomElement& elem, Gradient& grad)
{
	grad.clear();
	grad.setExtend(elem.attribute("extend").toInt());

	QDomElement stopNode = elem.firstChildElement("Stop");
	while(!stopNode.isNull())
	{
		double pos = stopNode.attribute("pos").toDouble();
		QColor color;
		DataTrait<QColor>::readValue(stopNode, color);

		grad.add(pos, color);
		stopNode = stopNode.nextSiblingElement("Stop");
	}
}
*/
template class Data< Path >;
template class Data< QVector<Path> >;

int pathDataClass = RegisterData< Path >();
int pathVectorDataClass = RegisterData< QVector<Path> >();

} // namespace types

} // namespace panda

void convertType(const panda::types::Path& from, QVector<QPointF>& to)
{ to = static_cast< QVector<QPointF> >(from); }
void convertType(const QVector<QPointF>& from, panda::types::Path& to)
{ to = from; }

panda::types::RegisterTypeConverter<panda::types::Path, QVector<QPointF> > PathPointsConverter;
panda::types::RegisterTypeConverter<QVector<QPointF>, panda::types::Path> PointsPathConverter;
