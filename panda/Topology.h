#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include <QVector>
#include <QPointF>
#include <utility>

#include <QMetaType>
#include <QTextStream>

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

    void addPoint(const QPointF& point);
    void addPoints(const QVector<QPointF>& pts);

    void addEdge(int a, int b);
    void addEdges(const QVector<Edge>& e);

    void addPolygon(const Polygon& p);
    void addPolygons(const QVector<Polygon>& p);

    int getNumberOfPoints() const;
	int getNumberOfEdges();
    int getNumberOfPolygons() const;

	const QVector<QPointF>& getPoints() const;
	const QVector<Edge>& getEdges();
	const QVector<Polygon>& getPolygons() const;

    QPointF& getPoint(int index);
	QPointF getPoint(int index) const;
	Edge getEdge(int index);
	Polygon getPolygon(int index) const;

	int getPointIndex(QPointF pt);
	int getEdgeIndex(int a, int b);
	int getEdgeIndex(const Edge& e);
	int getPolygonIndex(const Polygon& p);

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

Q_DECLARE_METATYPE(Topology)
const int topologyMetaTypeId = qMetaTypeId<Topology>();

} // namespace panda

#endif // ANIMATION_H
