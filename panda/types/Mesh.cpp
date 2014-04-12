#include <panda/types/Mesh.h>

#include <panda/DataFactory.h>
#include <panda/Data.inl>

#include <QMap>
#include <set>

namespace panda
{

namespace types
{

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

Mesh::PointID Mesh::addPoint(const Point& point)
{
	m_points.push_back(point);
	return m_points.size() - 1;
}

void Mesh::addPoints(const SeqPoints& pts)
{
	m_points += pts;
}

Mesh::EdgeID Mesh::addEdge(PointID a, PointID b)
{
	Q_ASSERT(a != b);
	m_edges.push_back(std::make_pair(a, b));
	return m_edges.size() - 1;
}

Mesh::EdgeID Mesh::addEdge(Edge e)
{
	Q_ASSERT(e.first != e.second);
	m_edges.push_back(e);
	return m_edges.size() - 1;
}

void Mesh::addEdges(const SeqEdges& e)
{
	m_edges += e;
}

Mesh::PolygonID Mesh::addPolygon(const Polygon& p)
{
	m_polygons.push_back(p);
	return m_polygons.size() - 1;
}

Mesh::PolygonID Mesh::addPolygon(PolygonID p1, PolygonID p2, PolygonID p3)
{
	Polygon p;
	p.push_back(p1);
	p.push_back(p2);
	p.push_back(p3);
	return addPolygon(p);
}

void Mesh::addPolygons(const SeqPolygons& p)
{
	m_polygons += p;
}

int Mesh::getNumberOfPoints() const
{
	return m_points.size();
}

int Mesh::getNumberOfEdges() const
{
	if(!hasEdges() && getNumberOfPolygons() > 0)
		const_cast<Mesh*>(this)->createEdgeList();

	return m_edges.size();
}

int Mesh::getNumberOfPolygons() const
{
	return m_polygons.size();
}

const Mesh::SeqPoints &Mesh::getPoints() const
{
	return m_points;
}

const Mesh::SeqEdges &Mesh::getEdges() const
{
	if(!hasEdges())
		const_cast<Mesh*>(this)->createEdgeList();

	return m_edges;
}

const Mesh::SeqPolygons &Mesh::getPolygons() const
{
	return m_polygons;
}

Point& Mesh::getPoint(PointID index)
{
	return m_points[index];
}

Point Mesh::getPoint(PointID index) const
{
	return m_points[index];
}

Mesh::Edge Mesh::getEdge(EdgeID index) const
{
	if(!hasEdges())
		const_cast<Mesh*>(this)->createEdgeList();

	return m_edges[index];
}

Mesh::Polygon Mesh::getPolygon(PolygonID index) const
{
	return m_polygons[index];
}

Mesh::PointID Mesh::getPointIndex(const Point &pt) const
{
	return m_points.indexOf(pt);
}

Mesh::EdgeID Mesh::getEdgeIndex(PointID a, PointID b) const
{
	Edge e1 = Edge(a, b), e2 = Edge(b, a);
	int id = m_edges.indexOf(e1);
	if(id < 0)
		return m_edges.indexOf(e2);
	return id;
}

Mesh::EdgeID Mesh::getEdgeIndex(const Edge& e) const
{
	Edge e2 = Edge(e.second, e.first);
	int id = m_edges.indexOf(e);
	if(id < 0)
		return m_edges.indexOf(e2);
	return id;
}

Mesh::PolygonID Mesh::getPolygonIndex(const Polygon& p) const
{
	return m_polygons.indexOf(p);
}

const Mesh::EdgesIndicesList &Mesh::getEdgesInPolygon(PolygonID index)
{
	if(hasEdgesInPolygon())
		createEdgesInPolygonList();

	if((int)index < getNumberOfPolygons() && (int)index >= m_edgesInPolygon.size())
		createEdgesInPolygonList();

	return m_edgesInPolygon[index];
}

const QVector<Mesh::EdgesIndicesList>& Mesh::getEdgesInPolygonList()
{
	return m_edgesInPolygon;
}

const Mesh::EdgesIndicesList& Mesh::getEdgesAroundPoint(PointID index)
{
	if(!hasEdgesAroundPoint())
		createEdgesAroundPointList();

	if((int)index < getNumberOfPoints() && (int)index >= m_edgesAroundPoint.size())
		createEdgesAroundPointList();

	return m_edgesAroundPoint[index];
}

const QVector<Mesh::EdgesIndicesList>& Mesh::getEdgesAroundPointList()
{
	return m_edgesAroundPoint;
}

const Mesh::PolygonsIndicesList &Mesh::getPolygonsAroundPoint(PointID index)
{
	if(!hasPolygonsAroundPoint())
		createPolygonsAroundPointList();

	if((int)index < getNumberOfPoints() && (int)index >= m_polygonsAroundPoint.size())
		createPolygonsAroundPointList();

	return m_polygonsAroundPoint[index];
}

const QVector<Mesh::PolygonsIndicesList>& Mesh::getPolygonsAroundPointList()
{
	return m_polygonsAroundPoint;
}

const Mesh::PolygonsIndicesList& Mesh::getPolygonsAroundEdge(EdgeID index)
{
	if(!hasPolygonsAroundEdge())
		createPolygonsAroundEdgeList();

	if((int)index < getNumberOfEdges() && (int)index >= m_polygonsAroundEdge.size())
		createPolygonsAroundEdgeList();

	return m_polygonsAroundEdge[index];
}

const QVector<Mesh::PolygonsIndicesList>& Mesh::getPolygonsAroundEdgeList()
{
	return m_polygonsAroundEdge;
}

const QVector<Mesh::PointID> &Mesh::getPointsOnBorder()
{
	if(!hasBorderElementsLists())
		createElementsOnBorder();

	return m_pointsOnBorder;
}

const QVector<Mesh::EdgeID> &Mesh::getEdgesOnBorder()
{
	if(!hasBorderElementsLists())
		createElementsOnBorder();

	return m_edgesOnBorder;
}

const QVector<Mesh::PolygonID> &Mesh::getPolygonsOnBorder()
{
	if(!hasBorderElementsLists())
		createElementsOnBorder();

	return m_polygonsOnBorder;
}

Mesh::PolygonsIndicesList Mesh::getPolygonsAroundPolygon(PolygonID index)
{
	if(!hasPolygonsAroundPoint())
		createPolygonsAroundPointList();

	std::set<int> polySet;
	PolygonsIndicesList polyList;

	const Polygon& poly = m_polygons[index];
	for(int pt : poly)
	{
		const PolygonsIndicesList& polyAP = m_polygonsAroundPoint[pt];

		for(int p : polyAP)
		{
			if(p != index && polySet.find(p) == polySet.end())
			{
				polySet.insert(p);
				polyList.push_back(p);
			}
		}
	}

	return polyList;
}

Mesh::PolygonsIndicesList Mesh::getPolygonsAroundPolygons(const PolygonsIndicesList& listID)
{
	std::set<PolygonID> polySet;
	for(auto index : listID)
	{
		const PolygonsIndicesList list = getPolygonsAroundPolygon(index);
		polySet.insert(list.begin(), list.end());
	}

	for(auto index : listID)
		polySet.erase(index);

	PolygonsIndicesList polyList;
	for(auto polyId : polySet)
		polyList.push_back(polyId);

	return polyList;
}

Mesh::PolygonsIndicesList Mesh::getPolygonsConnectedToPolygon(PolygonID index)
{
	if(!hasPolygonsAroundPoint())
		createPolygonsAroundPointList();

	PolygonsIndicesList polyAll, polyOnFront, polyPrev, polyNext;
	bool end = false;
	int cpt = 1, nb = m_polygons.size();

	polyAll.push_back(index);
	polyOnFront.push_back(index);

	while(!end && cpt < nb)
	{
		polyNext = getPolygonsAroundPolygons(polyOnFront);

		for(int p : polyNext)
		{
			if(!polyAll.contains(p))
			{
				polyAll.push_back(p);
				polyPrev.push_back(p);
			}
		}

		cpt += polyPrev.size();

		if(polyPrev.empty())
			end = true;

		polyOnFront = polyPrev;
		polyPrev.clear();
	}

	return polyAll;
}

Mesh::PointID Mesh::getOtherPointInEdge(const Edge &edge, PointID point) const
{
	if(edge.first == point)
		return edge.second;
	else if(edge.second == point)
		return edge.first;
	else
		return -1;
}

PReal Mesh::areaOfPolygon(const Polygon& poly) const
{
	int nbPts = poly.size();
	PReal area = 0;
	for(int i=0; i<nbPts; ++i)
	{
		Point p1 = getPoint(poly[i]), p2 = getPoint(poly[(i+1)%nbPts]);
		area += p1.cross(p2);
	}

	return area / 2;
}

void Mesh::reorientPolygon(Polygon& poly)
{
	Polygon copy = poly;
	poly.clear();
	poly.reserve(copy.size());

	for(int pt : copy)
		poly.push_front(pt);
}

Point Mesh::centroidOfPolygon(const Polygon& poly) const
{
	int nbPts = poly.size();
	Point pt;
	for(int i=0; i<nbPts; ++i)
	{
		Point p1 = getPoint(poly[i]), p2 = getPoint(poly[(i+1)%nbPts]);
		pt += (p1 + p2) * p1.cross(p2);
	}

	return pt / (6 * areaOfPolygon(poly));
}

bool Mesh::comparePolygon(Polygon p1, Polygon p2)
{
	if(p1.size() != p2.size())
		return false;

	while(!p1.empty())
	{
		int pt1 = p1.back();
		p1.pop_back();

		bool found = false;

		for(int i=0; i<p2.size(); ++i)
		{
			if(p2[i] == pt1)
			{
				p2.remove(i);
				found = true;
				break;
			}
		}

		if(!found)
			return false;
	}

	return true;
}

bool Mesh::polygonContainsPoint(const Polygon &poly, Point pt) const
{
	int nb = poly.size();
	for(int i1=0, i0=nb-1; i1<nb; i0=i1++)
	{
		const Point &p0 = getPoint(poly[i0]), &p1 = getPoint(poly[i1]);
		Point n = Point(p1.y - p0.y, p0.x - p1.x);
		Point d = pt - p0;

		if(n.dot(d) > 0)
			return false;
	}
	return true;
}

void Mesh::removeUnusedPoints()
{
	clearEdges();
	clearEdgesAroundPoint();
	clearEdgesInPolygon();
	clearPolygonsAroundPoint();
	clearPolygonsAroundEdge();
	clearBorderElementLists();

	QMap<PointID, PointID> pointsMap;
	SeqPoints newPoints;
	int nbPoints = 0;
	for(Polygon& p : m_polygons)
	{
		for(auto& id : p)
		{
			if(!pointsMap.contains(id))
			{
				pointsMap[id] = nbPoints++;
				newPoints.push_back(m_points[id]);
			}

			id = pointsMap[id];
		}
	}

	m_points.swap(newPoints);
}

bool Mesh::hasPoints() const
{
	return !m_points.empty();
}

bool Mesh::hasEdges() const
{
	return !m_edges.empty();
}

bool Mesh::hasPolygons() const
{
	return !m_polygons.empty();
}

bool Mesh::hasEdgesInPolygon() const
{
	return !m_edgesInPolygon.empty();
}

bool Mesh::hasEdgesAroundPoint() const
{
	return !m_edgesAroundPoint.empty();
}

bool Mesh::hasPolygonsAroundPoint() const
{
	return !m_polygonsAroundPoint.empty();
}

bool Mesh::hasPolygonsAroundEdge() const
{
	return !m_polygonsAroundEdge.empty();
}

bool Mesh::hasBorderElementsLists() const
{
	return !m_pointsOnBorder.empty() && !m_edgesOnBorder.empty() && !m_polygonsOnBorder.empty();
}

void Mesh::createEdgeList()
{
	if(!hasPolygons())
		return;

	if(hasEdges())
	{
		clearEdges();
		clearEdgesAroundPoint();
		clearEdgesInPolygon();
		clearPolygonsAroundEdge();
	}

	QMap<Edge, int> edgeMap;
	for(Polygon p : m_polygons)
	{
		int nbPts = p.size();
		for(int i=0; i<nbPts; ++i)
		{
			const int p1 = p[i];
			const int p2 = p[(i+1)%nbPts];

			const Edge e = ((p1<p2) ? Edge(p1, p2) : Edge(p2, p1));

			if(!edgeMap.contains(e))
			{
				const int edgeId = edgeMap.size();
				edgeMap[e] = edgeId;
				m_edges.push_back(Edge(p1, p2));
			}
		}
	}
}

void Mesh::createEdgesInPolygonList()
{
	if(hasEdgesInPolygon())
		clearEdgesInPolygon();

	int nbPolys = m_polygons.size();
	m_edgesInPolygon.resize(nbPolys);

	if(!hasEdges())
	{
		QMap<Edge, int> edgeMap;

		for(int i=0; i<nbPolys; ++i)
		{
			const Polygon& p = m_polygons[i];
			int nbPts = p.size();
			for(int j=0; j<nbPts; ++j)
			{
				const int p1 = p[j];
				const int p2 = p[(j+1)%nbPts];

				const Edge e = ((p1<p2) ? Edge(p1, p2) : Edge(p2, p1));

				if(!edgeMap.contains(e))
				{
					const int edgeId = edgeMap.size();
					edgeMap[e] = edgeId;
					m_edges.push_back(Edge(p1, p2));
				}

				m_edgesInPolygon[i].push_back(edgeMap[e]);
			}
		}
	}
	else
	{
		if(!hasEdgesAroundPoint())
			createEdgesAroundPointList();

		const QVector<EdgesIndicesList>& eapl = getEdgesAroundPointList();

		for(int i=0; i<nbPolys; ++i)
		{
			EdgesIndicesList& eip = m_edgesInPolygon[i];
			const Polygon& p = m_polygons[i];
			const int nbP = p.size();
			for(int j=0; j<nbP; ++j)
			{
				bool found = false;
				const int p1 = p[j], p2 = p[(j+1)%nbP];
				const EdgesIndicesList& eap = eapl[p1];
				const int nbE = eap.size();
				for(int k=0; k<nbE; ++k)
				{
					const int eid = eap[k];
					const Edge& e = m_edges[eid];
					if(e.first == p1 && e.second == p2 || e.first == p2 && e.second == p1)
					{
						found = true;
						eip.push_back(eid);
						break;
					}
				}

				Q_ASSERT(found);
			}
		}
	}
}

void Mesh::createEdgesAroundPointList()
{
	if(!hasEdges())
		createEdgeList();

	if(hasEdgesAroundPoint())
		clearEdgesAroundPoint();

	m_edgesAroundPoint.resize(getNumberOfPoints());
	int nbEdges = getNumberOfEdges();
	for(int i=0; i<nbEdges; ++i)
	{
		m_edgesAroundPoint[m_edges[i].first].push_back(i);
		m_edgesAroundPoint[m_edges[i].second].push_back(i);
	}
}

void Mesh::createPolygonsAroundPointList()
{
	if(!hasPolygons())
		return;

	if(hasPolygonsAroundPoint())
		clearPolygonsAroundPoint();

	m_polygonsAroundPoint.resize(getNumberOfPoints());
	int nbPolys = getNumberOfPolygons();
	for(int i=0; i<nbPolys; ++i)
	{
		const Polygon& p = m_polygons[i];
		int nb = p.size();
		for(int j=0; j<nb; ++j)
			m_polygonsAroundPoint[p[j]].push_back(i);
	}
}

void Mesh::createPolygonsAroundEdgeList()
{
	if(!hasPolygons())
		return;

	if(!hasEdges())
		createEdgeList();

	if(!hasEdgesInPolygon())
		createEdgesInPolygonList();

	const int nbPolys = getNumberOfPolygons();
	const int nbEdges = getNumberOfEdges();

	m_polygonsAroundEdge.resize(nbEdges);
	for(int i=0; i<nbPolys; ++i)
	{
		const EdgesIndicesList& eip = m_edgesInPolygon[i];
		const int nb = eip.size();
		for(int j=0; j<nb; ++j)
			m_polygonsAroundEdge[eip[j]].push_back(i);
	}
}

void Mesh::createElementsOnBorder()
{
	if(!hasPolygonsAroundEdge())
		createPolygonsAroundEdgeList();

	if(!hasPolygonsAroundPoint())
		createPolygonsAroundPointList();

	m_pointsOnBorder.clear();
	m_edgesOnBorder.clear();
	m_polygonsOnBorder.clear();

	for(int i=0, nb=getNumberOfEdges(); i<nb; ++i)
	{
		if(m_polygonsAroundEdge[i].size() == 1) // On a border
		{
			m_edgesOnBorder.push_back(i);

			const int pt1Id = m_edges[i].first;
			if(!m_pointsOnBorder.contains(pt1Id))
				m_pointsOnBorder.push_back(pt1Id);

			const int pt2Id = m_edges[i].second;
			if(!m_pointsOnBorder.contains(pt2Id))
				m_pointsOnBorder.push_back(pt2Id);
		}
	}

	for(auto pt : m_pointsOnBorder)
	{
		for(auto poly : m_polygonsAroundPoint[pt])
		{
			if(!m_polygonsOnBorder.contains(poly))
				m_polygonsOnBorder.push_back(poly);
		}
	}
}

void Mesh::createTriangles()
{
	if(!hasEdges())
		return;

	if(!hasEdgesAroundPoint())
		createEdgesAroundPointList();

	if(hasPolygons())
		clearPolygons();

	const int nbEdges = getNumberOfEdges();
	QList<int> tmpEdgesId;
	for(int i=0; i<nbEdges; ++i)
		tmpEdgesId.push_back(i);

	clearPolygonsAroundEdge();
	m_polygonsAroundEdge.resize(nbEdges);

	while(!tmpEdgesId.empty())
	{
		int eid = tmpEdgesId.front();
		Edge e = getEdge(eid);
		tmpEdgesId.pop_front();

		const EdgesIndicesList& neighbors = getEdgesAroundPoint(e.first);
		int nbNgh = neighbors.size();
		for(int i=0; i<nbNgh; ++i)
		{
			int e2id = neighbors[i];
			Edge e2 = getEdge(e2id);
			int p2id = getOtherPointInEdge(e2, e.first);
			int e3id = getEdgeIndex(p2id, e.second);

			if(e3id != -1)
			{
				Polygon poly;
				poly.push_back(e.first);
				poly.push_back(p2id);
				poly.push_back(e.second);

				if(!m_polygonsAroundEdge[eid].empty())
				{
					int poly2 = m_polygonsAroundEdge[eid][0];
					if(comparePolygon(poly, getPolygon(poly2)))
						continue;
				}

				if(areaOfPolygon(poly) < 0)
					reorientPolygon(poly);

				int polyid = addPolygon(poly);

				m_polygonsAroundEdge[eid].push_back(polyid);
				if(m_polygonsAroundEdge[eid].size() == 2)
					tmpEdgesId.removeOne(eid);

				m_polygonsAroundEdge[e2id].push_back(polyid);
				if(m_polygonsAroundEdge[e2id].size() == 2)
					tmpEdgesId.removeOne(e2id);

				m_polygonsAroundEdge[e3id].push_back(polyid);
				if(m_polygonsAroundEdge[e3id].size() == 2)
					tmpEdgesId.removeOne(e3id);
			}
		}
	}
}

void Mesh::clearPoints()
{
	m_points.clear();
}

void Mesh::clearEdges()
{
	m_edges.clear();
}

void Mesh::clearPolygons()
{
	m_polygons.clear();
}

void Mesh::clearEdgesInPolygon()
{
	m_edgesInPolygon.clear();
}

void Mesh::clearEdgesAroundPoint()
{
	m_edgesAroundPoint.clear();
}

void Mesh::clearPolygonsAroundPoint()
{
	m_polygonsAroundPoint.clear();
}

void Mesh::clearPolygonsAroundEdge()
{
	m_polygonsAroundEdge.clear();
}

void Mesh::clearBorderElementLists()
{
	m_pointsOnBorder.clear();
	m_edgesOnBorder.clear();
	m_polygonsOnBorder.clear();
}

void Mesh::clear()
{
	clearPoints();
	clearEdges();
	clearPolygons();
	clearEdgesInPolygon();
	clearEdgesAroundPoint();
	clearPolygonsAroundPoint();
	clearPolygonsAroundEdge();
	clearBorderElementLists();
}

//***************************************************************//

template<> QString DataTrait<Mesh>::valueTypeName() { return "mesh"; }
template<> QString DataTrait<Mesh>::valueTypeNamePlural() { return "meshes"; }
template<> bool DataTrait<Mesh>::isDisplayed() { return false; }

template<>
void DataTrait<Mesh>::writeValue(QDomDocument& doc, QDomElement& elem, const Mesh& v)
{
	for(const auto& p : v.getPoints())
	{
		QDomElement ptNode = doc.createElement("Point");
		elem.appendChild(ptNode);
		ptNode.setAttribute("x", p.x);
		ptNode.setAttribute("y", p.y);
	}

	for(const auto& e : v.getEdges())
	{
		QDomElement edgeNode = doc.createElement("Edge");
		elem.appendChild(edgeNode);
		edgeNode.setAttribute("p1", e.first);
		edgeNode.setAttribute("p2", e.second);
	}

	for(const auto& poly : v.getPolygons())
	{
		QDomElement polyNode = doc.createElement("Poly");
		elem.appendChild(polyNode);
		for(const auto& p : poly)
		{
			QDomElement indexNode = doc.createElement("Point");
			polyNode.appendChild(indexNode);
			indexNode.setAttribute("index", p);
		}
	}
}

template<>
void DataTrait<Mesh>::readValue(QDomElement& elem, Mesh& v)
{
	Mesh tmp;

	QDomElement ptNode = elem.firstChildElement("Point");
	while(!ptNode.isNull())
	{
		Point pt;
#ifdef PANDA_DOUBLE
		pt.x = ptNode.attribute("x").toDouble();
		pt.y = ptNode.attribute("y").toDouble();
#else
		pt.x = ptNode.attribute("x").toFloat();
		pt.y = ptNode.attribute("y").toFloat();
#endif
		tmp.addPoint(pt);
		ptNode = ptNode.nextSiblingElement("Point");
	}

	QDomElement edgeNode = elem.firstChildElement("Edge");
	while(!edgeNode.isNull())
	{
		Mesh::Edge edge;
		edge.first = edgeNode.attribute("p1").toInt();
		edge.second = edgeNode.attribute("p2").toInt();
		tmp.addEdge(edge);
		edgeNode = edgeNode.nextSiblingElement("Edge");
	}

	QDomElement polyNode = elem.firstChildElement("Poly");
	while(!polyNode.isNull())
	{
		Mesh::Polygon poly;
		QDomElement indexNode = elem.firstChildElement("Point");
		while(!indexNode.isNull())
		{
			poly.push_back(indexNode.attribute("index").toInt());
			indexNode = indexNode.nextSiblingElement("Point");
		}
		tmp.addPolygon(poly);
		polyNode = polyNode.nextSiblingElement("Poly");
	}

	v = std::move(tmp);
}

template class Data< Mesh >;
template class Data< QVector<Mesh> >;

int meshDataClass = RegisterData< Mesh >();
int meshVectorDataClass = RegisterData< QVector<Mesh> >();

} // namespace types

} // namespace panda
