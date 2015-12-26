#include <panda/types/Point.h>
#include <panda/types/Path.h>

#include <panda/DataFactory.h>
#include <panda/Data.h>

namespace panda
{

namespace types
{

Path& Path::operator=(const std::vector<Point> &v)
{
	std::vector<Point>::operator=(v);
	return *this;
}

Path Path::operator+(const Point& p) const
{
	int nb = size();
	Path tmp;
	tmp.resize(nb);
	for(int i=0; i<nb; ++i)
		tmp[i] = (*this)[i] + p;
	return tmp;
}

Path Path::operator-(const Point& p) const
{
	int nb = size();
	Path tmp;
	tmp.resize(nb);
	for(int i=0; i<nb; ++i)
		tmp[i] = (*this)[i] - p;
	return tmp;
}

Path& Path::operator+=(const Point& p)
{
	for(auto& pt : *this)
		pt += p;
	return *this;
}

Path& Path::operator-=(const Point& p)
{
	for(auto& pt : *this)
		pt -= p;
	return *this;
}

Path Path::operator*(PReal v) const
{
	int nb = size();
	Path tmp;
	tmp.resize(nb);
	for(int i=0; i<nb; ++i)
		tmp[i] = (*this)[i] * v;
	return tmp;
}

Path Path::operator/(PReal v) const
{
	int nb = size();
	Path tmp;
	tmp.resize(nb);
	for(int i=0; i<nb; ++i)
		tmp[i] = (*this)[i] / v;
	return tmp;
}

Path& Path::operator*=(PReal v)
{
	for(auto& pt : *this)
		pt *= v;
	return *this;
}

Path& Path::operator/=(PReal v)
{
	for(auto& pt : *this)
		pt /= v;
	return *this;
}

Path operator*(PReal v, const Path& p)
{
	int nb = p.size();
	Path tmp;
	tmp.resize(nb);
	for(int i=0; i<nb; ++i)
		tmp[i] = p[i] * v;
	return tmp;
}

Path operator/(PReal v, const Path& p)
{
	int nb = p.size();
	Path tmp;
	tmp.resize(nb);
	for(int i=0; i<nb; ++i)
		tmp[i] = p[i] / v;
	return tmp;
}

Path Path::linearProduct(const Point& p) const
{
	int nb = size();
	Path tmp;
	tmp.resize(nb);
	for(int i=0; i<nb; ++i)
		tmp[i] = (*this)[i].linearProduct(p);
	return tmp;
}

Path Path::linearDivision(const Point& p) const
{
	int nb = size();
	Path tmp;
	tmp.resize(nb);
	for(int i=0; i<nb; ++i)
		tmp[i] = (*this)[i].linearDivision(p);
	return tmp;
}

Path Path::reversed() const
{
	int nb = size();
	Path tmp;
	tmp.resize(nb);
	for(int i=0; i<nb; ++i)
		tmp[i] = (*this)[nb-1-i];
	return tmp;
}

void Path::reverse()
{
	reversed().swap(*this);
}

void rotate(Path& path, const Point& center, PReal angle)
{
	int nb = path.size();
	PReal ca = cos(angle), sa = sin(angle);
	for(int i=0; i<nb; ++i)
	{
		Point pt = path[i] - center;
		path[i] = center + Point(pt.x*ca-pt.y*sa, pt.x*sa+pt.y*ca);
	}
}

Path rotated(const Path& path, const Point& center, PReal angle)
{
	int nb = path.size();
	Path tmp;
	tmp.resize(nb);
	PReal ca = cos(angle), sa = sin(angle);
	for(int i=0; i<nb; ++i)
	{
		Point pt = path[i] - center;
		tmp[i] = center + Point(pt.x*ca-pt.y*sa, pt.x*sa+pt.y*ca);
	}
	return tmp;
}

//****************************************************************************//

PReal areaOfPolygon(const Path& poly)
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

types::Point centroidOfPolygon(const Path& poly)
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

bool polygonContainsPoint(const Path &poly, types::Point pt)
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

void reorientPolygon(Path& poly)
{
	std::reverse(poly.begin(), poly.end());
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
template class PANDA_CORE_API Data< std::vector<Path> >;

int pathDataClass = RegisterData< Path >();
int pathVectorDataClass = RegisterData< std::vector<Path> >();

} // namespace types

} // namespace panda

void convertType(const panda::types::Path& from, std::vector<panda::types::Point>& to)
{ to = static_cast< std::vector<panda::types::Point> >(from); }
void convertType(const std::vector<panda::types::Point>& from, panda::types::Path& to)
{ to = from; }

panda::types::RegisterTypeConverter<panda::types::Path, std::vector<panda::types::Point> > PathPointsConverter;
panda::types::RegisterTypeConverter<std::vector<panda::types::Point>, panda::types::Path> PointsPathConverter;

