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

PReal areaOfPolygon(const QVector<types::Point>& poly)
{
	int nbPts = poly.size();
	PReal area = 0;
	for(int i=0; i<nbPts; ++i)
	{
		Point p1 = poly[i], p2 = poly[(i+1)%nbPts];
		area += p1.cross(p2);
	}

	return area / 2;
}

types::Point centroidOfPolygon(const QVector<types::Point>& poly)
{
	int nbPts = poly.size();
	Point pt;
	for(int i=0; i<nbPts; ++i)
	{
		Point p1 = poly[i], p2 = poly[(i+1)%nbPts];
		pt += (p1 + p2) * p1.cross(p2);
	}

	return pt / (6 * areaOfPolygon(poly));
}

bool polygonContainsPoint(const QVector<types::Point> &poly, types::Point pt)
{
	int nb = poly.size();
	for(int i1=0, i0=nb-1; i1<nb; i0=i1++)
	{
		const Point &p0 = poly[i0], &p1 = poly[i1];
		Point n = Point(p1.y - p0.y, p0.x - p1.x);
		Point d = pt - p0;

		if(n.dot(d) > 0)
			return false;
	}
	return true;
}

void reorientPolygon(QVector<types::Point>& poly)
{
	std::reverse(poly.begin(), poly.end());
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
