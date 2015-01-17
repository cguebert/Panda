#include <panda/types/Point.h>
#include <panda/types/Path.h>

#include <panda/DataFactory.h>
#include <panda/Data.h>

namespace panda
{

namespace types
{

Path& Path::operator=(const QVector<Point> &v)
{
	QVector<Point>::operator=(v);
	return *this;
}

//****************************************************************************//

template<> PANDA_CORE_API QString DataTrait<Path>::valueTypeName() { return "path"; }
template<> PANDA_CORE_API int DataTrait<Path>::size(const Path& v) { return v.size(); }
template<> PANDA_CORE_API void DataTrait<Path>::clear(Path& v, int size, bool init)
{
	if(init)
		v.clear();
	v.resize(size);
}

template<>
PANDA_CORE_API void DataTrait<Path>::writeValue(QDomDocument& doc, QDomElement& elem, const Path& path)
{
	auto pointTrait = DataTraitsList::getTraitOf<Point>();
	for(const auto& pt : path)
	{
		QDomElement ptNode = doc.createElement("Point");
		pointTrait->writeValue(doc, ptNode, &pt);
		elem.appendChild(ptNode);
	}
}

template<>
PANDA_CORE_API void DataTrait<Path>::readValue(QDomElement& elem, Path& path)
{
	path.clear();
	auto pointTrait = DataTraitsList::getTraitOf<Point>();

	QDomElement ptNode = elem.firstChildElement("Point");
	while(!ptNode.isNull())
	{
		Point pt;
		pointTrait->readValue(ptNode, &pt);
		path.push_back(pt);
		ptNode = ptNode.nextSiblingElement("Point");
	}
}

template class PANDA_CORE_API Data< Path >;
template class PANDA_CORE_API Data< QVector<Path> >;

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

