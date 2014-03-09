#include <panda/types/Path.h>

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

template<>
void DataTrait<Path>::writeValue(QDomDocument& doc, QDomElement& elem, const Path& path)
{
	for(const auto& pt : path)
	{
		QDomElement ptNode = doc.createElement("Point");
		ptNode.setAttribute("x", pt.x());
		ptNode.setAttribute("y", pt.y());
		elem.appendChild(ptNode);
	}
}

template<>
void DataTrait<Path>::readValue(QDomElement& elem, Path& path)
{
	path.clear();

	QDomElement ptNode = elem.firstChildElement("Point");
	while(!ptNode.isNull())
	{
		double x = ptNode.attribute("x").toDouble();
		double y = ptNode.attribute("y").toDouble();

		path.push_back(QPointF(x, y));
		ptNode = ptNode.nextSiblingElement("Point");
	}
}

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
