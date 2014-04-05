#include <panda/types/Point.h>
#include <panda/types/Path.h>

#include <panda/types/Animation.inl>
#include <panda/types/AnimationTraits.h>

#include <panda/DataFactory.h>
#include <panda/Data.inl>

namespace panda
{

namespace types
{

Path& Path::operator=(const QVector<Point> &v)
{
	QVector<Point>::operator=(v);
	return *this;
}

//*************************************************************************//

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
		ptNode.setAttribute("x", pt.x);
		ptNode.setAttribute("y", pt.y);
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
#ifdef PANDA_DOUBLE
		PReal x = ptNode.attribute("x").toDouble();
		PReal y = ptNode.attribute("y").toDouble();
#else
		PReal x = ptNode.attribute("x").toFloat();
		PReal y = ptNode.attribute("y").toFloat();
#endif

		path.push_back(Point(x, y));
		ptNode = ptNode.nextSiblingElement("Point");
	}
}

template class Data< Path >;
template class Data< QVector<Path> >;

int pathDataClass = RegisterData< Path >();
int pathVectorDataClass = RegisterData< QVector<Path> >();

} // namespace types

} // namespace panda

void convertType(const panda::types::Path& from, QVector<panda::types::Point>& to)
{ to = static_cast< QVector<panda::types::Point> >(from); }
void convertType(const QVector<panda::types::Point>& from, panda::types::Path& to)
{ to = from; }

panda::types::RegisterTypeConverter<panda::types::Path, QVector<panda::types::Point> > PathPointsConverter;
panda::types::RegisterTypeConverter<QVector<panda::types::Point>, panda::types::Path> PointsPathConverter;

