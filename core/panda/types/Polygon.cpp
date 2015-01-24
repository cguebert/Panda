#include <panda/types/Polygon.h>

#include <panda/DataFactory.h>
#include <panda/Data.h>

namespace panda
{

namespace types
{

void Polygon::clear()
{
	contour.clear();
	holes.clear();
}

//****************************************************************************//

void translate(Polygon& poly, const Point& delta)
{
	poly.contour += delta;
	for(auto& hole : poly.holes)
		hole += delta;
}

Polygon translated(const Polygon& poly, const Point& delta)
{
	Polygon tmp;
	tmp.contour = poly.contour + delta;
	for(const auto& hole : poly.holes)
		tmp.holes.push_back(hole + delta);
	return tmp;
}

void scale(Polygon& poly, PReal scale)
{
	poly.contour *= scale;
	for(auto& hole : poly.holes)
		hole *= scale;
}

Polygon scaled(const Polygon& poly, PReal scale)
{
	Polygon tmp;
	tmp.contour = poly.contour * scale;
	for(const auto& hole : poly.holes)
		tmp.holes.push_back(hole * scale);
	return tmp;
}

void rotate(Polygon& poly, const Point& center, PReal angle)
{
	rotate(poly.contour, center, angle);
	for(auto& hole : poly.holes)
		rotate(hole, center, angle);
}

Polygon rotated(const Polygon& poly, const Point& center, PReal angle)
{
	Polygon tmp;
	tmp.contour = rotated(poly.contour, center, angle);
	for(const auto& hole : poly.holes)
		tmp.holes.push_back(rotated(hole, center, angle));
	return tmp;
}

//****************************************************************************//

template<> PANDA_CORE_API QString DataTrait<Polygon>::valueTypeName() { return "polygon"; }

template<>
PANDA_CORE_API void DataTrait<Polygon>::writeValue(QDomDocument& doc, QDomElement& elem, const Polygon& poly)
{
	auto pathTrait = DataTraitsList::getTraitOf<Path>();
	QDomElement ctNode = doc.createElement("Contour");
	pathTrait->writeValue(doc, ctNode, &poly.contour);
	elem.appendChild(ctNode);
	for(const auto& hole : poly.holes)
	{
		QDomElement holeNode = doc.createElement("Hole");
		pathTrait->writeValue(doc, holeNode, &hole);
		elem.appendChild(holeNode);
	}
}

template<>
PANDA_CORE_API void DataTrait<Polygon>::readValue(QDomElement& elem, Polygon& poly)
{
	poly.clear();
	auto pathTrait = DataTraitsList::getTraitOf<Path>();
	QDomElement ctNode = elem.firstChildElement("Contour");
	if(!ctNode.isNull())
		pathTrait->readValue(ctNode, &poly.contour);

	QDomElement holeNode = elem.firstChildElement("Hole");
	while(!holeNode.isNull())
	{
		Path path;
		pathTrait->readValue(holeNode, &path);
		poly.holes.push_back(path);
		holeNode = holeNode.nextSiblingElement("Hole");
	}
}

template class PANDA_CORE_API Data< Polygon >;
template class PANDA_CORE_API Data< QVector<Polygon> >;

int polygonDataClass = RegisterData< Polygon >();
int polygonVectorDataClass = RegisterData< QVector<Polygon> >();

} // namespace types

} // namespace panda

void convertType(const QVector<panda::types::Point>& from, panda::types::Polygon& to)
{
	to.contour = from;
	to.holes.clear();
}

void convertType(const panda::types::Path& from, panda::types::Polygon& to)
{
	to.contour = from;
	to.holes.clear();
}

panda::types::RegisterTypeConverter<QVector<panda::types::Point>, panda::types::Polygon > PointsVectorPolygonConverter;
panda::types::RegisterTypeConverter<panda::types::Path, panda::types::Polygon > PathPolygonConverter;
