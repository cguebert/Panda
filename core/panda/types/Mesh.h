#ifndef TYPES_MESH_H
#define TYPES_MESH_H

#include <panda/core.h>
#include <panda/types/Point.h>
#include <panda/helper/algorithm.h>

#include <array>
#include <vector>

namespace panda
{

namespace types
{

class PANDA_CORE_API Mesh
{
public:
	typedef unsigned int index_type;
	enum { InvalidID = (unsigned)-1 };
	typedef index_type PointID;
	typedef index_type EdgeID;
	typedef index_type TriangleID;

	typedef std::vector<PointID> PointsIndicesList;
	typedef std::vector<EdgeID> EdgesIndicesList;
	typedef std::vector<TriangleID> TrianglesIndicesList;

	typedef std::array<PointID, 2> Edge;
	typedef std::array<PointID, 3> Triangle;

	typedef std::vector<Point> SeqPoints;
	typedef std::vector<Edge> SeqEdges;
	typedef std::vector<Triangle> SeqTriangles;

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

	int nbPoints() const;
	int nbEdges() const;
	int nbTriangles() const;

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
	const std::vector<EdgesInTriangle>& getEdgesInTriangleList() const;

	const EdgesIndicesList& getEdgesAroundPoint(PointID index);
	const std::vector<EdgesIndicesList>& getEdgesAroundPointList() const;

	const TrianglesIndicesList& getTrianglesAroundPoint(PointID index);
	const std::vector<TrianglesIndicesList>& getTrianglesAroundPointList() const;

	const TrianglesIndicesList& getTrianglesAroundEdge(EdgeID index);
	const std::vector<TrianglesIndicesList>& getTrianglesAroundEdgeList() const;

	const std::vector<PointID>& getPointsOnBorder();
	const std::vector<EdgeID>& getEdgesOnBorder();
	const std::vector<TriangleID>& getTrianglesOnBorder();

	TrianglesIndicesList getTrianglesAroundTriangle(TriangleID index, bool shareEdge);
	TrianglesIndicesList getTrianglesAroundTriangles(const TrianglesIndicesList& listID, bool shareEdge);
	TrianglesIndicesList getTrianglesConnectedToTriangle(TriangleID index, bool shareEdge);

	PointID getOtherPointInEdge(const Edge& edge, PointID point) const;
	float areaOfTriangle(const Triangle& poly) const;
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

	std::vector<EdgesInTriangle> m_edgesInTriangle;
	std::vector<EdgesIndicesList> m_edgesAroundPoint;
	std::vector<TrianglesIndicesList> m_trianglesAroundPoint, m_trianglesAroundEdge;

	PointsIndicesList m_pointsOnBorder;
	EdgesIndicesList m_edgesOnBorder;
	TrianglesIndicesList m_trianglesOnBorder;
};

PANDA_CORE_API void translate(Mesh& mesh, const Point& delta);
PANDA_CORE_API Mesh translated(const Mesh& mesh, const Point& delta);

PANDA_CORE_API void scale(Mesh& mesh, float scale);
PANDA_CORE_API Mesh scaled(const Mesh& mesh, float scale);

/// Rotate every point of the paths around the center, angle is given in radians
PANDA_CORE_API void rotate(Mesh& mesh, const Point& center, float angle);
PANDA_CORE_API Mesh rotated(const Mesh& mesh, const Point& center, float angle);

//****************************************************************************//

inline Mesh::Mesh() { }

inline Mesh::Edge Mesh::makeEdge(PointID p1, PointID p2)
{ return { { p1, p2 } }; }

inline Mesh::Triangle Mesh::makeTriangle(PointID p1, PointID p2, PointID p3)
{ return { { p1, p2, p3 } }; }

inline Mesh::PointID Mesh::addPoint(const Point& point)
{ m_points.push_back(point); return m_points.size() - 1; }

inline void Mesh::addPoints(const SeqPoints& pts)
{ m_points.insert(m_points.end(), pts.begin(), pts.end()); }

inline Mesh::EdgeID Mesh::addEdge(const Edge& e)
{ m_edges.push_back(e); return m_edges.size() - 1; }

inline Mesh::EdgeID Mesh::addEdge(PointID a, PointID b)
{ return addEdge(makeEdge(a, b)); }

inline void Mesh::addEdges(const SeqEdges& edges)
{ m_edges.insert(m_edges.end(), edges.begin(), edges.end()); }

inline Mesh::TriangleID Mesh::addTriangle(const Triangle& t)
{ m_triangles.push_back(t); return m_triangles.size() - 1; }

inline Mesh::TriangleID Mesh::addTriangle(PointID p1, PointID p2, PointID p3)
{ return addTriangle(makeTriangle(p1, p2, p3)); }

inline void Mesh::addTriangles(const SeqTriangles& triangles)
{ m_triangles.insert(m_triangles.end(), triangles.begin(), triangles.end()); }

inline int Mesh::nbPoints() const
{ return m_points.size(); }

inline int Mesh::nbEdges() const
{ return m_edges.size(); }

inline int Mesh::nbTriangles() const
{ return m_triangles.size(); }

inline const Mesh::SeqPoints& Mesh::getPoints() const
{ return m_points; }

inline const Mesh::SeqEdges& Mesh::getEdges() const
{ return m_edges; }

inline const Mesh::SeqTriangles& Mesh::getTriangles() const
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
{ return helper::indexOf(m_points, pt); }

inline Mesh::EdgeID Mesh::getEdgeIndex(PointID a, PointID b) const
{ return getEdgeIndex(makeEdge(a, b)); }

inline Mesh::TriangleID Mesh::getTriangleIndex(const Triangle& p) const
{ return helper::indexOf(m_triangles, p); }

inline const std::vector<Mesh::EdgesInTriangle>& Mesh::getEdgesInTriangleList() const
{ return m_edgesInTriangle; }

inline const std::vector<Mesh::EdgesIndicesList>& Mesh::getEdgesAroundPointList() const
{ return m_edgesAroundPoint; }

inline const std::vector<Mesh::TrianglesIndicesList>& Mesh::getTrianglesAroundPointList() const
{ return m_trianglesAroundPoint; }

inline const std::vector<Mesh::TrianglesIndicesList>& Mesh::getTrianglesAroundEdgeList() const
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
