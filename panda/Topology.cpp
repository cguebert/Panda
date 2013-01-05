#include <panda/Topology.h>

#include <QMap>
#include <set>

namespace panda
{

Topology::Topology()
{
}

Topology::~Topology()
{
}

void Topology::addPoint(const QPointF& point)
{
	m_points.append(point);
}

void Topology::addPoints(const QVector<QPointF>& pts)
{
	m_points += pts;
}

void Topology::addEdge(int a, int b)
{
    Q_ASSERT(a != b);
	m_edges.append(std::make_pair(a, b));
}

void Topology::addEdges(const QVector<Edge>& e)
{
	m_edges += e;
}

void Topology::addPolygon(const Polygon& p)
{
	m_polygons.append(p);
}

void Topology::addPolygons(const QVector<Polygon>& p)
{
	m_polygons += p;
}

int Topology::getNumberOfPoints() const
{
	return m_points.size();
}

int Topology::getNumberOfEdges()
{
	if(!hasEdges() && getNumberOfPolygons() > 0)
		createEdgeList();

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

const QVector<typename Topology::Edge>& Topology::getEdges()
{
	if(!hasEdges())
		createEdgeList();

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

Topology::Edge Topology::getEdge(int index)
{
	if(!hasEdges())
		createEdgeList();

	return m_edges[index];
}

Topology::Polygon Topology::getPolygon(int index) const
{
	return m_polygons[index];
}

int Topology::getPointIndex(QPointF pt)
{
	return m_points.indexOf(pt);
}

int Topology::getEdgeIndex(int a, int b)
{
	Edge e1 = Edge(a, b), e2 = Edge(b, a);
	int id = m_edges.indexOf(e1);
	if(id < 0)
		return m_edges.indexOf(e2);
	return id;
}

int Topology::getEdgeIndex(const Edge& e)
{
	Edge e2 = Edge(e.second, e.first);
	int id = m_edges.indexOf(e);
	if(id < 0)
		return m_edges.indexOf(e2);
	return id;
}

int Topology::getPolygonIndex(const Polygon& p)
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

} // namespace panda
