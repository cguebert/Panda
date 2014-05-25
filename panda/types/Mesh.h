#ifndef TYPES_MESH_H
#define TYPES_MESH_H

#include <QVector>
#include <panda/types/Point.h>

#include <array>

namespace panda
{

namespace types
{

class Mesh
{
public:
	typedef unsigned int index_type;
	enum { InvalidID = (unsigned)-1 };
	typedef index_type PointID;
	typedef index_type EdgeID;
	typedef index_type TriangleID;

	typedef QVector<PointID> PointsIndicesList;
	typedef QVector<EdgeID> EdgesIndicesList;
	typedef QVector<TriangleID> TrianglesIndicesList;

	typedef std::array<PointID, 2> Edge;
	typedef std::array<PointID, 3> Triangle;

	typedef QVector<Point> SeqPoints;
	typedef QVector<Edge> SeqEdges;
	typedef QVector<Triangle> SeqTriangles;

	typedef std::array<EdgeID, 3> EdgesInTriangle;

	Mesh();

	static Edge makeEdge(PointID p1, PointID p2);
	static Triangle makeTriangle(PointID p1, PointID p2, PointID p3);

	PointID addPoint(const Point& point);
	void addPoints(const SeqPoints& pts);

	EdgeID addEdge(const Edge& e);
	EdgeID addEdge(PointID a, PointID b);
	void addEdges(const SeqEdges& edges);

	TriangleID addTriangle(const Triangle& t);
	TriangleID addTriangle(PointID p1, PointID p2, PointID p3);
	void addTriangles(const SeqTriangles& triangles);

	int getNumberOfPoints() const;
	int getNumberOfEdges() const;
	int getNumberOfTriangles() const;

	const SeqPoints& getPoints() const;
	const SeqEdges& getEdges() const;
	const SeqTriangles& getTriangles() const;

	Point& getPoint(PointID index);
	Point getPoint(PointID index) const;
	Edge getEdge(EdgeID index) const;
	Triangle getTriangle(TriangleID index) const;

	PointID getPointIndex(const Point& pt) const;
	EdgeID getEdgeIndex(PointID a, PointID b) const;
	EdgeID getEdgeIndex(const Edge& e) const;
	TriangleID getTriangleIndex(const Triangle& p) const;

	const EdgesInTriangle& getEdgesInTriangle(TriangleID index);
	const QVector<EdgesInTriangle>& getEdgesInTriangleList() const;

	const EdgesIndicesList& getEdgesAroundPoint(PointID index);
	const QVector<EdgesIndicesList>& getEdgesAroundPointList() const;

	const TrianglesIndicesList& getTrianglesAroundPoint(PointID index);
	const QVector<TrianglesIndicesList>& getTrianglesAroundPointList() const;

	const TrianglesIndicesList& getTrianglesAroundEdge(EdgeID index);
	const QVector<TrianglesIndicesList>& getTrianglesAroundEdgeList() const;

	const QVector<PointID>& getPointsOnBorder();
	const QVector<EdgeID>& getEdgesOnBorder();
	const QVector<TriangleID>& getTrianglesOnBorder();

	TrianglesIndicesList getTrianglesAroundTriangle(TriangleID index);
	TrianglesIndicesList getTrianglesAroundTriangles(const TrianglesIndicesList& listID);
	TrianglesIndicesList getTrianglesConnectedToTriangle(TriangleID index);

	PointID getOtherPointInEdge(const Edge& edge, PointID point) const;
	PReal areaOfTriangle(const Triangle& poly) const;
	void reorientTriangle(Triangle& poly);
	Point centroidOfTriangle(const Triangle& poly) const;
	bool triangleContainsPoint(const Triangle& poly, Point pt) const;

	void removeUnusedPoints();

	bool hasPoints() const;
	bool hasEdges() const;
	bool hasTriangles() const;
	bool hasEdgesInTriangle() const;
	bool hasEdgesAroundPoint() const;
	bool hasTrianglesAroundPoint() const;
	bool hasTrianglesAroundEdge() const;
	bool hasBorderElementsLists() const;

	void createEdgeList();
	void createEdgesInTriangleList();
	void createEdgesAroundPointList();
	void createTrianglesAroundPointList();
	void createTrianglesAroundEdgeList();
	void createElementsOnBorder();
	void createTriangles();

	void clearPoints();
	void clearEdges();
	void clearTriangles();
	void clearEdgesInTriangle();
	void clearEdgesAroundPoint();
	void clearTrianglesAroundPoint();
	void clearTrianglesAroundEdge();
	void clearBorderElementLists();
	void clear();

	bool operator==(const Mesh& mesh) const;
	bool operator!=(const Mesh& mesh) const;

protected:
	SeqPoints m_points;
	SeqEdges m_edges;
	SeqTriangles m_triangles;

	QVector<EdgesInTriangle> m_edgesInTriangle;
	QVector<EdgesIndicesList> m_edgesAroundPoint;
	QVector<TrianglesIndicesList> m_trianglesAroundPoint, m_trianglesAroundEdge;

	PointsIndicesList m_pointsOnBorder;
	EdgesIndicesList m_edgesOnBorder;
	TrianglesIndicesList m_trianglesOnBorder;
};

//***************************************************************//

inline Mesh::Mesh() { }

inline Mesh::Edge Mesh::makeEdge(PointID p1, PointID p2)
{ Edge tmp = { { p1, p2 } }; return tmp; }

inline Mesh::Triangle Mesh::makeTriangle(PointID p1, PointID p2, PointID p3)
{ Triangle tmp = { { p1, p2, p3 } }; return tmp; }

inline Mesh::PointID Mesh::addPoint(const Point& point)
{ m_points.push_back(point); return m_points.size() - 1; }

inline void Mesh::addPoints(const SeqPoints& pts)
{ m_points += pts; }

inline Mesh::EdgeID Mesh::addEdge(const Edge& e)
{ m_edges.push_back(e); return m_edges.size() - 1; }

inline Mesh::EdgeID Mesh::addEdge(PointID a, PointID b)
{ return addEdge(makeEdge(a, b)); }

inline void Mesh::addEdges(const SeqEdges& edges)
{ m_edges += edges; }

inline Mesh::TriangleID Mesh::addTriangle(const Triangle& t)
{ m_triangles.push_back(t); return m_triangles.size() - 1; }

inline Mesh::TriangleID Mesh::addTriangle(PointID p1, PointID p2, PointID p3)
{ return addTriangle(makeTriangle(p1, p2, p3)); }

inline void Mesh::addTriangles(const SeqTriangles& triangles)
{ m_triangles += triangles; }

inline int Mesh::getNumberOfPoints() const
{ return m_points.size(); }

inline int Mesh::getNumberOfEdges() const
{ return m_edges.size(); }

inline int Mesh::getNumberOfTriangles() const
{ return m_triangles.size(); }

inline const Mesh::SeqPoints &Mesh::getPoints() const
{ return m_points; }

inline const Mesh::SeqEdges &Mesh::getEdges() const
{ return m_edges; }

inline const Mesh::SeqTriangles &Mesh::getTriangles() const
{ return m_triangles; }

inline Point& Mesh::getPoint(PointID index)
{ return m_points[index]; }

inline Point Mesh::getPoint(PointID index) const
{ return m_points[index]; }

inline Mesh::Edge Mesh::getEdge(EdgeID index) const
{ return m_edges[index]; }

inline Mesh::Triangle Mesh::getTriangle(TriangleID index) const
{ return m_triangles[index]; }

inline Mesh::PointID Mesh::getPointIndex(const Point &pt) const
{ return m_points.indexOf(pt); }

inline Mesh::EdgeID Mesh::getEdgeIndex(PointID a, PointID b) const
{ return getEdgeIndex(makeEdge(a, b)); }

inline Mesh::TriangleID Mesh::getTriangleIndex(const Triangle& p) const
{ return m_triangles.indexOf(p); }

inline const QVector<Mesh::EdgesInTriangle>& Mesh::getEdgesInTriangleList() const
{ return m_edgesInTriangle; }

inline const QVector<Mesh::EdgesIndicesList>& Mesh::getEdgesAroundPointList() const
{ return m_edgesAroundPoint; }

inline const QVector<Mesh::TrianglesIndicesList>& Mesh::getTrianglesAroundPointList() const
{ return m_trianglesAroundPoint; }

inline const QVector<Mesh::TrianglesIndicesList>& Mesh::getTrianglesAroundEdgeList() const
{ return m_trianglesAroundEdge; }

inline void Mesh::reorientTriangle(Triangle& t)
{ std::reverse(t.begin(), t.end()); }

inline bool Mesh::hasPoints() const
{ return !m_points.empty(); }

inline bool Mesh::hasEdges() const
{ return !m_edges.empty(); }

inline bool Mesh::hasTriangles() const
{ return !m_triangles.empty(); }

inline bool Mesh::hasEdgesInTriangle() const
{ return !m_edgesInTriangle.empty(); }

inline bool Mesh::hasEdgesAroundPoint() const
{ return !m_edgesAroundPoint.empty(); }

inline bool Mesh::hasTrianglesAroundPoint() const
{ return !m_trianglesAroundPoint.empty(); }

inline bool Mesh::hasTrianglesAroundEdge() const
{ return !m_trianglesAroundEdge.empty(); }

inline bool Mesh::hasBorderElementsLists() const
{ return !m_pointsOnBorder.empty() && !m_edgesOnBorder.empty() && !m_trianglesOnBorder.empty(); }

inline void Mesh::clearPoints()
{ m_points.clear(); }

inline void Mesh::clearEdges()
{ m_edges.clear(); }

inline void Mesh::clearTriangles()
{ m_triangles.clear(); }

inline void Mesh::clearEdgesInTriangle()
{ m_edgesInTriangle.clear(); }

inline void Mesh::clearEdgesAroundPoint()
{ m_edgesAroundPoint.clear(); }

inline void Mesh::clearTrianglesAroundPoint()
{ m_trianglesAroundPoint.clear(); }

inline void Mesh::clearTrianglesAroundEdge()
{ m_trianglesAroundEdge.clear(); }

inline bool Mesh::operator==(const Mesh& mesh) const
{ return m_points == mesh.m_points && m_edges == mesh.m_edges && m_triangles == mesh.m_triangles; }

inline bool Mesh::operator!=(const Mesh& mesh) const
{ return !(*this == mesh); }

} // namespace types

} // namespace panda

#endif // TYPES_MESH_H
