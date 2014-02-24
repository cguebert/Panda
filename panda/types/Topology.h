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
	typedef unsigned int index_type;
	enum { InvalidID = (unsigned)-1 };
	typedef index_type PointID;
	typedef index_type EdgeID;
	typedef index_type PolygonID;

	typedef QVector<PointID> PointsIndicesList;
	typedef QVector<EdgeID> EdgesIndicesList;
	typedef QVector<PolygonID> PolygonsIndicesList;

	typedef std::pair<PointID, PointID> Edge;
	typedef QVector<PointID> Polygon;

	typedef QVector<QPointF> SeqPoints;
	typedef QVector<Edge> SeqEdges;
	typedef QVector<Polygon> SeqPolygons;

	Topology();
	virtual ~Topology();

	PointID addPoint(const QPointF& point);
	void addPoints(const SeqPoints& pts);

	EdgeID addEdge(PointID a, PointID b);
	EdgeID addEdge(Edge e);
	void addEdges(const SeqEdges& e);

	PolygonID addPolygon(const Polygon& p);
	void addPolygons(const SeqPolygons& p);

	int getNumberOfPoints() const;
	int getNumberOfEdges() const;
	int getNumberOfPolygons() const;

	const SeqPoints& getPoints() const;
	const SeqEdges& getEdges() const;
	const SeqPolygons& getPolygons() const;

	QPointF& getPoint(PointID index);
	QPointF getPoint(PointID index) const;
	Edge getEdge(EdgeID index) const;
	Polygon getPolygon(PolygonID index) const;

	PointID getPointIndex(const QPointF& pt) const;
	EdgeID getEdgeIndex(PointID a, PointID b) const;
	EdgeID getEdgeIndex(const Edge& e) const;
	PolygonID getPolygonIndex(const Polygon& p) const;

	const EdgesIndicesList& getEdgesInPolygon(PolygonID index);
	const QVector<EdgesIndicesList>& getEdgesInPolygonList();

	const EdgesIndicesList& getEdgesAroundPoint(PointID index);
	const QVector<EdgesIndicesList>& getEdgesAroundPointList();

	const PolygonsIndicesList& getPolygonsAroundPoint(PointID index);
	const QVector<PolygonsIndicesList>& getPolygonsAroundPointList();

	const PolygonsIndicesList& getPolygonsAroundEdge(EdgeID index);
	const QVector<PolygonsIndicesList>& getPolygonsAroundEdgeList();

	const QVector<PointID>& getPointsOnBorder();
	const QVector<EdgeID>& getEdgesOnBorder();
	const QVector<PolygonID>& getPolygonsOnBorder();

	PolygonsIndicesList getPolygonsAroundPolygon(PolygonID index);
	PolygonsIndicesList getPolygonsAroundPolygons(const PolygonsIndicesList& listID);
	PolygonsIndicesList getPolygonsConnectedToPolygon(PolygonID index);

	PointID getOtherPointInEdge(const Edge& edge, PointID point) const;
	double areaOfPolygon(const Polygon& poly) const;
	void reorientPolygon(Polygon& poly);
	static bool comparePolygon(Polygon p1, Polygon p2);
	bool polygonContainsPoint(const Polygon& poly, QPointF pt) const;

	void removeUnusedPoints();

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
	SeqPoints m_points;
	SeqEdges m_edges;
	SeqPolygons m_polygons;

	QVector<EdgesIndicesList> m_edgesInPolygon, m_edgesAroundPoint;
	QVector<PolygonsIndicesList> m_polygonsAroundPoint, m_polygonsAroundEdge;

	PointsIndicesList m_pointsOnBorder;
	EdgesIndicesList m_edgesOnBorder;
	PolygonsIndicesList m_polygonsOnBorder;
};

//***************************************************************//


} // namespace types

} // namespace panda

#endif // ANIMATION_H
