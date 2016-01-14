#include <panda/types/Polygon.h>

#include <panda/data/DataFactory.h>

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

template<> PANDA_CORE_API std::string DataTrait<Polygon>::valueTypeName() { return "polygon"; }

template<>
PANDA_CORE_API void DataTrait<Polygon>::writeValue(XmlElement& elem, const Polygon& poly)
{
	auto pathTrait = DataTraitsList::getTraitOf<Path>();
	auto ctNode = elem.addChild("Contour");
	pathTrait->writeValue(ctNode, &poly.contour);

	for(const auto& hole : poly.holes)
	{
		auto holeNode = elem.addChild("Hole");
		pathTrait->writeValue(holeNode, &hole);
	}
}

template<>
PANDA_CORE_API void DataTrait<Polygon>::readValue(XmlElement& elem, Polygon& poly)
{
	poly.clear();
	auto pathTrait = DataTraitsList::getTraitOf<Path>();
	auto ctNode = elem.firstChild("Contour");
	if(ctNode)
		pathTrait->readValue(ctNode, &poly.contour);

	auto holeNode = elem.firstChild("Hole");
	while(holeNode)
	{
		Path path;
		pathTrait->readValue(holeNode, &path);
		poly.holes.push_back(path);
		holeNode = holeNode.nextSibling("Hole");
	}
}

template class PANDA_CORE_API Data< Polygon >;
template class PANDA_CORE_API Data< std::vector<Polygon> >;

int polygonDataClass = RegisterData< Polygon >();
int polygonVectorDataClass = RegisterData< std::vector<Polygon> >();

} // namespace types

} // namespace panda

void convertType(const std::vector<panda::types::Point>& from, panda::types::Polygon& to)
{
	to.contour = from;
	to.holes.clear();
}

void convertType(const panda::types::Path& from, panda::types::Polygon& to)
{
	to.contour = from;
	to.holes.clear();
}

panda::types::RegisterTypeConverter<std::vector<panda::types::Point>, panda::types::Polygon > PointsVectorPolygonConverter;
panda::types::RegisterTypeConverter<panda::types::Path, panda::types::Polygon > PathPolygonConverter;
