#include <panda/Topology.h>
#include <panda/DataFactory.h>

#include <QMap>
#include <QStack>
#include <set>

namespace panda
{

Topology::Topology()
{
}

Topology::~Topology()
{
}

int Topology::addPoint(const QPointF& point)
{
	m_points.push_back(point);
	return m_points.size() - 1;
}

void Topology::addPoints(const QVector<QPointF>& pts)
{
	m_points += pts;
}

int Topology::addEdge(int a, int b)
{
    Q_ASSERT(a != b);
	m_edges.push_back(std::make_pair(a, b));
	return m_edges.size() - 1;
}

int Topology::addEdge(Edge e)
{
	Q_ASSERT(e.first != e.second);
	m_edges.push_back(e);
	return m_edges.size() - 1;
}

void Topology::addEdges(const QVector<Edge>& e)
{
	m_edges += e;
}

int Topology::addPolygon(const Polygon& p)
{
	m_polygons.push_back(p);
	return m_polygons.size() - 1;
}

void Topology::addPolygons(const QVector<Polygon>& p)
{
	m_polygons += p;
}

int Topology::getNumberOfPoints() const
{
	return m_points.size();
}

int Topology::getNumberOfEdges() const
{
	if(!hasEdges() && getNumberOfPolygons() > 0)
		const_cast<Topology*>(this)->createEdgeList();

	return m_edges.size();
}

int Topology::getNumberOfPolygons() const
{
	return m_polygons.size();
}

const QVector<QPointF>& Topology::getPoints() const
{
	return m_points;
}

const QVector<typename Topology::Edge>& Topology::getEdges() const
{
	if(!hasEdges())
		const_cast<Topology*>(this)->createEdgeList();

	return m_edges;
}

const QVector<typename Topology::Polygon>& Topology::getPolygons() const
{
	return m_polygons;
}

QPointF& Topology::getPoint(int index)
{
	return m_points[index];
}

QPointF Topology::getPoint(int index) const
{
	return m_points[index];
}

Topology::Edge Topology::getEdge(int index) const
{
	if(!hasEdges())
		const_cast<Topology*>(this)->createEdgeList();

	return m_edges[index];
}

Topology::Polygon Topology::getPolygon(int index) const
{
	return m_polygons[index];
}

int Topology::getPointIndex(QPointF pt) const
{
	return m_points.indexOf(pt);
}

int Topology::getEdgeIndex(int a, int b) const
{
	Edge e1 = Edge(a, b), e2 = Edge(b, a);
	int id = m_edges.indexOf(e1);
	if(id < 0)
		return m_edges.indexOf(e2);
	return id;
}

int Topology::getEdgeIndex(const Edge& e) const
{
	Edge e2 = Edge(e.second, e.first);
	int id = m_edges.indexOf(e);
	if(id < 0)
		return m_edges.indexOf(e2);
	return id;
}

int Topology::getPolygonIndex(const Polygon& p) const
{
	return m_polygons.indexOf(p);
}

const Topology::IndicesList& Topology::getEdgesInPolygon(int index)
{
	if(hasEdgesInPolygon())
		createEdgesInPolygonList();

	if(index >= m_edgesInPolygon.size())
		createEdgesInPolygonList();

	return m_edgesInPolygon[index];
}

const QVector<Topology::IndicesList>& Topology::getEdgesInPolygonList()
{
	return m_edgesInPolygon;
}

const Topology::IndicesList& Topology::getEdgesAroundPoint(int index)
{
	if(!hasEdgesAroundPoint())
		createEdgesAroundPointList();

	if(index >= m_edgesAroundPoint.size())
		createEdgesAroundPointList();

	return m_edgesAroundPoint[index];
}

const QVector<Topology::IndicesList>& Topology::getEdgesAroundPointList()
{
	return m_edgesAroundPoint;
}

const Topology::IndicesList& Topology::getPolygonsAroundPoint(int index)
{
	if(!hasPolygonsAroundPoint())
		createPolygonsAroundPointList();

	if(index >= m_polygonsAroundPoint.size())
		createPolygonsAroundPointList();

	return m_polygonsAroundPoint[index];
}

const QVector<Topology::IndicesList>& Topology::getPolygonsAroundPointList()
{
	return m_polygonsAroundPoint;
}

const Topology::IndicesList& Topology::getPolygonsAroundEdge(int index)
{
	if(!hasPolygonsAroundEdge())
		createPolygonsAroundEdgeList();

	if(index >= m_polygonsAroundEdge.size())
		createPolygonsAroundEdgeList();

	return m_polygonsAroundEdge[index];
}

const QVector<Topology::IndicesList>& Topology::getPolygonsAroundEdgeList()
{
	return m_polygonsAroundEdge;
}

const Topology::IndicesList& Topology::getPointsOnBorder()
{
	if(!hasBorderElementsLists())
		createElementsOnBorder();

	return m_pointsOnBorder;
}

const Topology::IndicesList& Topology::getEdgesOnBorder()
{
	if(!hasBorderElementsLists())
		createElementsOnBorder();

	return m_edgesOnBorder;
}

const Topology::IndicesList& Topology::getPolygonsOnBorder()
{
	if(!hasBorderElementsLists())
		createElementsOnBorder();

	return m_polygonsOnBorder;
}

Topology::IndicesList Topology::getPolygonsAroundPolygon(int index)
{
	if(!hasPolygonsAroundPoint())
		createPolygonsAroundPointList();

	std::set<int> polySet;
	IndicesList polyList;

	const Polygon& poly = m_polygons[index];
	foreach(int pt, poly)
	{
		const IndicesList& polyAP = m_polygonsAroundPoint[pt];

		foreach(int p, polyAP)
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

Topology::IndicesList Topology::getPolygonsAroundPolygons(const IndicesList& listID)
{
	std::set<int> polySet;
	foreach(int index, listID)
	{
		const IndicesList list = getPolygonsAroundPolygon(index);
		polySet.insert(list.begin(), list.end());
	}

	foreach(int index, listID)
		polySet.erase(index);

	IndicesList polyList;
	for(std::set<int>::iterator iter=polySet.begin(); iter!=polySet.end(); ++iter)
		polyList.push_back(*iter);

	return polyList;
}

Topology::IndicesList Topology::getPolygonsConnectedToPolygon(int index)
{
	if(!hasPolygonsAroundPoint())
		createPolygonsAroundPointList();

	IndicesList polyAll, polyOnFront, polyPrev, polyNext;
	bool end = false;
	int cpt = 1, nb = m_polygons.size();

	polyAll.push_back(index);
	polyOnFront.push_back(index);

	while(!end && cpt < nb)
	{
		polyNext = getPolygonsAroundPolygons(polyOnFront);

		foreach(int p, polyNext)
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

int Topology::getOtherPointInEdge(int edge, int point) const
{
	Edge e = getEdge(edge);
	if(e.first == point)
		return e.second;
	else if(e.second == point)
		return e.first;
	else
		return -1;
}

double Topology::areaOfPolygon(int polyId) const
{
	const Polygon& p = getPolygon(polyId);

	int nbPts = p.size();
	double area = 0;
	for(int i=0; i<nbPts; ++i)
	{
		QPointF p1 = getPoint(p[i]), p2 = getPoint(p[(i+1)%nbPts]);
		area += p1.x()*p2.y() - p2.x()*p1.y();
	}

	return area / 2;
}

void Topology::reorientPolygon(int polyId)
{
	Polygon& p = getPolygon(polyId);
	Polygon copy = p;
	p.clear();
	p.reserve(copy.size());

	foreach(int pt, copy)
		p.push_front(pt);
}

bool Topology::comparePolygon(Polygon p1, Polygon p2)
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

bool Topology::hasPoints() const
{
	return !m_points.empty();
}

bool Topology::hasEdges() const
{
	return !m_edges.empty();
}

bool Topology::hasPolygons() const
{
	return !m_polygons.empty();
}

bool Topology::hasEdgesInPolygon() const
{
	return !m_edgesInPolygon.empty();
}

bool Topology::hasEdgesAroundPoint() const
{
	return !m_edgesAroundPoint.empty();
}

bool Topology::hasPolygonsAroundPoint() const
{
	return !m_polygonsAroundPoint.empty();
}

bool Topology::hasPolygonsAroundEdge() const
{
	return !m_polygonsAroundEdge.empty();
}

bool Topology::hasBorderElementsLists() const
{
	return !m_pointsOnBorder.empty() && !m_edgesOnBorder.empty() && !m_polygonsOnBorder.empty();
}

void Topology::createEdgeList()
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
	foreach(Polygon p, m_polygons)
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

void Topology::createEdgesInPolygonList()
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
		const QVector<IndicesList>& eapl = getEdgesAroundPointList();

		for(int i=0; i<nbPolys; ++i)
		{
			IndicesList& eip = m_edgesInPolygon[i];
			const Polygon& p = m_polygons[i];
			const int nbP = p.size();
			for(int j=0; j<nbP; ++j)
			{
				bool found = false;
				const int p1 = p[j], p2 = p[(j+1)%nbP];
				const IndicesList& eap = eapl[p1];
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

void Topology::createEdgesAroundPointList()
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

void Topology::createPolygonsAroundPointList()
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

void Topology::createPolygonsAroundEdgeList()
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
		const IndicesList& eip = m_edgesInPolygon[i];
		const int nb = eip.size();
		for(int j=0; j<nb; ++j)
			m_polygonsAroundEdge[eip[j]].push_back(i);
	}
}

void Topology::createElementsOnBorder()
{
	if(!hasPolygonsAroundEdge())
		createPolygonsAroundEdgeList();

	m_pointsOnBorder.clear();
	m_edgesOnBorder.clear();
	m_polygonsOnBorder.clear();

	const int nbEdges = getNumberOfEdges();
	for(int i=0; i<nbEdges; ++i)
	{
		if(m_polygonsAroundEdge[i].size() == 1) // On a border
		{
			m_edgesOnBorder.push_back(i);

			const int ptId = m_edges[i].first;
			if(!m_pointsOnBorder.contains(ptId))
				m_pointsOnBorder.push_back(ptId);

			const int plId = m_polygonsAroundEdge[i][0];
			if(m_polygonsOnBorder.contains(plId))
				m_polygonsOnBorder.push_back(plId);
		}
	}
}

void Topology::createTriangles()
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

		const IndicesList& neighbors = getEdgesAroundPoint(e.first);
		int nbNgh = neighbors.size();
		for(int i=0; i<nbNgh; ++i)
		{
			int e2id = neighbors[i];
			int p2id = getOtherPointInEdge(e2id, e.first);
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

				int polyid = addPolygon(poly);

				if(areaOfPolygon(polyid) < 0)
					reorientPolygon(polyid);

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

void Topology::clearPoints()
{
	m_points.clear();
}

void Topology::clearEdges()
{
	m_edges.clear();
}

void Topology::clearPolygons()
{
	m_polygons.clear();
}

void Topology::clearEdgesInPolygon()
{
	m_edgesInPolygon.clear();
}

void Topology::clearEdgesAroundPoint()
{
	m_edgesAroundPoint.clear();
}

void Topology::clearPolygonsAroundPoint()
{
	m_polygonsAroundPoint.clear();
}

void Topology::clearPolygonsAroundEdge()
{
	m_polygonsAroundEdge.clear();
}

void Topology::clearBorderElementLists()
{
	m_pointsOnBorder.clear();
	m_edgesOnBorder.clear();
	m_polygonsOnBorder.clear();
}

void Topology::clear()
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

QTextStream& operator<<(QTextStream& stream, const Topology& topo)
{
	const int nbPts = topo.m_points.size();
	stream << nbPts << " ";
	for(int i=0; i<nbPts; ++i)
	{
		const QPointF& pt = topo.m_points[i];
		stream << pt.x() << " " << pt.y() << " ";
	}

	const int nbEdges = topo.m_edges.size();
	stream << nbEdges << " ";
	for(int i=0; i<nbEdges; ++i)
	{
		const Topology::Edge& e = topo.m_edges[i];
		stream << e.first << " " << e.second << " ";
	}

	const int nbPolys = topo.m_polygons.size();
	stream << nbPolys << " ";
	for(int i=0; i<nbPolys; ++i)
	{
		const Topology::Polygon& p = topo.m_polygons[i];
		const int nbP = p.size();
		stream << nbP << " ";
		for(int j=0; j<nbP; ++j)
			stream << p[j] << " ";
	}

	return stream;
}

QTextStream& operator>>(QTextStream& stream, Topology& topo)
{
	topo.clear();

	int nbPts, nbEdges, nbPolys;
	stream >> nbPts;
	topo.m_points.resize(nbPts);
	for(int i=0; i<nbPts; ++i)
	{
		QPointF pt;
		stream >> pt.rx() >> pt.ry();
		topo.m_points[i] = pt;
	}

	stream >> nbEdges;
	topo.m_edges.resize(nbEdges);
	for(int i=0; i<nbEdges; ++i)
	{
		Topology::Edge e;
		stream >> e.first >> e.second;
		topo.m_edges[i] = e;
	}

	stream >> nbPolys;
	topo.m_polygons.resize(nbPolys);
	for(int i=0; i<nbPolys; ++i)
	{
		Topology::Polygon p;
		int nbP;
		stream >> nbP;
		p.resize(nbP);
		for(int j=0; j<nbP; ++j)
			stream >> p[j];
		topo.m_polygons[i] = p;
	}

	return stream;
}

//***************************************************************//

template class Data<Topology>;
template class Data< QVector<Topology> >;

int topologyDataClass = RegisterData< Data<Topology> >();
int topologyVectorDataClass = RegisterData< Data< QVector<Topology> > >();

} // namespace panda