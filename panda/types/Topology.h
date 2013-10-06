#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include <QVector>
#include <QPointF>

namespace panda
{

namespace types
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


} // namespace types

} // namespace panda

#endif // ANIMATION_H
