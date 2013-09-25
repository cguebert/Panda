#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include <QVector>
#include <QPointF>
#include <utility>

#include <QMetaType>
#include <QTextStream>
#include <QDomDocument>

#include <panda/DataTraits.h>

namespace panda
{

class Topology
{
public:
    typedef std::pair<int, int> Edge;
    typedef QVector<int> IndicesList;
	typedef IndicesList Polygon;

    Topology();
    virtual ~Topology();

	int addPoint(const QPointF& point);
    void addPoints(const QVector<QPointF>& pts);

	int addEdge(int a, int b);
	int addEdge(Edge e);
    void addEdges(const QVector<Edge>& e);

	int addPolygon(const Polygon& p);
    void addPolygons(const QVector<Polygon>& p);

    int getNumberOfPoints() const;
	int getNumberOfEdges() const;
    int getNumberOfPolygons() const;

	const QVector<QPointF>& getPoints() const;
	const QVector<Edge>& getEdges() const;
	const QVector<Polygon>& getPolygons() const;

    QPointF& getPoint(int index);
	QPointF getPoint(int index) const;
	Edge getEdge(int index) const;
	Polygon getPolygon(int index) const;

	int getPointIndex(QPointF pt) const;
	int getEdgeIndex(int a, int b) const;
	int getEdgeIndex(const Edge& e) const;
	int getPolygonIndex(const Polygon& p) const;

	const IndicesList& getEdgesInPolygon(int index);
	const QVector<IndicesList>& getEdgesInPolygonList();

	const IndicesList& getEdgesAroundPoint(int index);
	const QVector<IndicesList>& getEdgesAroundPointList();

	const IndicesList& getPolygonsAroundPoint(int index);
	const QVector<IndicesList>& getPolygonsAroundPointList();

	const IndicesList& getPolygonsAroundEdge(int index);
	const QVector<IndicesList>& getPolygonsAroundEdgeList();

	const IndicesList& getPointsOnBorder();
	const IndicesList& getEdgesOnBorder();
	const IndicesList& getPolygonsOnBorder();

	IndicesList getPolygonsAroundPolygon(int index);
	IndicesList getPolygonsAroundPolygons(const IndicesList& listID);
	IndicesList getPolygonsConnectedToPolygon(int index);

	int getOtherPointInEdge(int edge, int point) const;
	double areaOfPolygon(int polyId) const;
	void reorientPolygon(int polyId);
	static bool comparePolygon(Polygon p1, Polygon p2);

	bool hasPoints() const;
	bool hasEdges() const;
	bool hasPolygons() const;
	bool hasEdgesInPolygon() const;
	bool hasEdgesAroundPoint() const;
	bool hasPolygonsAroundPoint() const;
	bool hasPolygonsAroundEdge() const;
	bool hasBorderElementsLists() const;

	void createEdgeList();
	void createEdgesInPolygonList();
	void createEdgesAroundPointList();
	void createPolygonsAroundPointList();
	void createPolygonsAroundEdgeList();
	void createElementsOnBorder();
	void createTriangles();

	void clearPoints();
	void clearEdges();
	void clearPolygons();
	void clearEdgesInPolygon();
	void clearEdgesAroundPoint();
	void clearPolygonsAroundPoint();
	void clearPolygonsAroundEdge();
	void clearBorderElementLists();
	void clear();

	friend QTextStream& operator<<(QTextStream& stream, const Topology& topo);
	friend QTextStream& operator>>(QTextStream& stream, Topology& topo);

protected:
    QVector<QPointF> m_points;
	QVector<Edge> m_edges;
    QVector<Polygon> m_polygons;

    QVector<IndicesList> m_edgesInPolygon,
        m_edgesAroundPoint,
        m_polygonsAroundPoint,
        m_polygonsAroundEdge;

    IndicesList m_pointsOnBorder,
        m_edgesOnBorder,
        m_polygonsOnBorder;
};

//***************************************************************//

Q_DECLARE_METATYPE(Topology)
const int topologyMetaTypeId = qMetaTypeId<Topology>();

template<> QString data_trait<Topology>::valueTypeName() { return "topology"; }
template<> bool data_trait<Topology>::isDisplayed() { return false; }

template<>
void data_trait<Topology>::writeValue(QDomDocument& doc, QDomElement& elem, const Topology& v)
{
	for(const auto& p : v.getPoints())
	{
		QDomElement ptNode = doc.createElement("Point");
		elem.appendChild(ptNode);
		ptNode.setAttribute("x", p.x());
		ptNode.setAttribute("y", p.y());
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
void data_trait<Topology>::readValue(QDomElement& elem, Topology& v)
{
	Topology tmp;

	QDomElement ptNode = elem.firstChildElement("Point");
	while(!ptNode.isNull())
	{
		QPointF pt;
		pt.setX(ptNode.attribute("x").toDouble());
		pt.setY(ptNode.attribute("y").toDouble());
		tmp.addPoint(pt);
		ptNode = ptNode.nextSiblingElement("Point");
	}

	QDomElement edgeNode = elem.firstChildElement("Edge");
	while(!edgeNode.isNull())
	{
		Topology::Edge edge;
		edge.first = edgeNode.attribute("p1").toInt();
		edge.second = edgeNode.attribute("p2").toInt();
		tmp.addEdge(edge);
		edgeNode = edgeNode.nextSiblingElement("Edge");
	}

	QDomElement polyNode = elem.firstChildElement("Poly");
	while(!polyNode.isNull())
	{
		Topology::Polygon poly;
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

} // namespace panda

#endif // ANIMATION_H
