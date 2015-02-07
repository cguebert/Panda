#include <panda/types/Mesh.h>

#include <panda/DataFactory.h>
#include <panda/Data.h>

#include <QMap>
#include <set>

namespace panda
{

namespace types
{

Mesh::EdgeID Mesh::getEdgeIndex(const Edge& e) const
{
	Edge e2 = makeEdge(e[0], e[1]);
	int id = m_edges.indexOf(e);
	if(id < 0)
		return m_edges.indexOf(e2);
	return id;
}

const Mesh::EdgesInTriangle& Mesh::getEdgesInTriangle(TriangleID index)
{
	if(hasEdgesInTriangle())
		createEdgesInTriangleList();

	if((int)index < nbTriangles() && (int)index >= m_edgesInTriangle.size())
		createEdgesInTriangleList();

	return m_edgesInTriangle[index];
}

const Mesh::EdgesIndicesList& Mesh::getEdgesAroundPoint(PointID index)
{
	if(!hasEdgesAroundPoint())
		createEdgesAroundPointList();

	if((int)index < nbPoints() && (int)index >= m_edgesAroundPoint.size())
		createEdgesAroundPointList();

	return m_edgesAroundPoint[index];
}

const Mesh::TrianglesIndicesList& Mesh::getTrianglesAroundPoint(PointID index)
{
	if(!hasTrianglesAroundPoint())
		createTrianglesAroundPointList();

	if((int)index < nbPoints() && (int)index >= m_trianglesAroundPoint.size())
		createTrianglesAroundPointList();

	return m_trianglesAroundPoint[index];
}

const Mesh::TrianglesIndicesList& Mesh::getTrianglesAroundEdge(EdgeID index)
{
	if(!hasTrianglesAroundEdge())
		createTrianglesAroundEdgeList();

	if((int)index < nbEdges() && (int)index >= m_trianglesAroundEdge.size())
		createTrianglesAroundEdgeList();

	return m_trianglesAroundEdge[index];
}

const QVector<Mesh::PointID>& Mesh::getPointsOnBorder()
{
	if(!hasBorderElementsLists())
		createElementsOnBorder();

	return m_pointsOnBorder;
}

const QVector<Mesh::EdgeID>& Mesh::getEdgesOnBorder()
{
	if(!hasBorderElementsLists())
		createElementsOnBorder();

	return m_edgesOnBorder;
}

const QVector<Mesh::TriangleID>& Mesh::getTrianglesOnBorder()
{
	if(!hasBorderElementsLists())
		createElementsOnBorder();

	return m_trianglesOnBorder;
}

Mesh::TrianglesIndicesList Mesh::getTrianglesAroundTriangle(TriangleID index)
{
	if(!hasTrianglesAroundPoint())
		createTrianglesAroundPointList();

	std::set<int> trianSet;
	TrianglesIndicesList trianList;

	const Triangle& trian = m_triangles[index];
	for(int pt : trian)
	{
		const TrianglesIndicesList& trianAP = m_trianglesAroundPoint[pt];

		for(int t : trianAP)
		{
			if(t != index && trianSet.find(t) == trianSet.end())
			{
				trianSet.insert(t);
				trianList.push_back(t);
			}
		}
	}

	return trianList;
}

Mesh::TrianglesIndicesList Mesh::getTrianglesAroundTriangles(const TrianglesIndicesList& listID)
{
	std::set<TriangleID> trianSet;
	for(auto index : listID)
	{
		const TrianglesIndicesList list = getTrianglesAroundTriangle(index);
		trianSet.insert(list.begin(), list.end());
	}

	for(auto index : listID)
		trianSet.erase(index);

	TrianglesIndicesList trianList;
	for(auto trianId : trianSet)
		trianList.push_back(trianId);

	return trianList;
}

Mesh::TrianglesIndicesList Mesh::getTrianglesConnectedToTriangle(TriangleID index)
{
	if(!hasTrianglesAroundPoint())
		createTrianglesAroundPointList();

	TrianglesIndicesList trianAll, trianOnFront, trianPrev, trianNext;
	bool end = false;
	int cpt = 1, nb = m_triangles.size();

	trianAll.push_back(index);
	trianOnFront.push_back(index);

	while(!end && cpt < nb)
	{
		trianNext = getTrianglesAroundTriangles(trianOnFront);

		for(int t : trianNext)
		{
			if(!trianAll.contains(t))
			{
				trianAll.push_back(t);
				trianPrev.push_back(t);
			}
		}

		cpt += trianPrev.size();

		if(trianPrev.empty())
			end = true;

		trianOnFront = trianPrev;
		trianPrev.clear();
	}

	return trianAll;
}

Mesh::PointID Mesh::getOtherPointInEdge(const Edge &edge, PointID point) const
{
	if(edge[0] == point)
		return edge[1];
	else if(edge[1] == point)
		return edge[0];
	else
		return -1;
}

PReal Mesh::areaOfTriangle(const Triangle& trian) const
{ // Half the cross product of the sides
	Point p1 = getPoint(trian[0]), p2 = getPoint(trian[1]), p3 = getPoint(trian[2]);
	Point p12 = p2 - p1, p13 = p3 - p1;
	return p12.cross(p13) / 2;
}

Point Mesh::centroidOfTriangle(const Triangle& trian) const
{
	Point p1 = getPoint(trian[0]), p2 = getPoint(trian[1]), p3 = getPoint(trian[2]);
	return (p1 + p2 + p3) / 3;
}

bool Mesh::triangleContainsPoint(const Triangle &trian, Point pt) const
{
	for(int i1=0, i0=2; i1<3; i0=i1++)
	{
		const Point &p0 = getPoint(trian[i0]), &p1 = getPoint(trian[i1]);
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
	clearEdgesInTriangle();
	clearTrianglesAroundPoint();
	clearTrianglesAroundEdge();
	clearBorderElementLists();

	QMap<PointID, PointID> pointsMap;
	SeqPoints newPoints;
	int nbPoints = 0;
	for(Triangle& t : m_triangles)
	{
		for(auto& id : t)
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

void Mesh::createEdgeList()
{
	if(!hasTriangles())
		return;

	if(hasEdges())
	{
		clearEdges();
		clearEdgesAroundPoint();
		clearEdgesInTriangle();
		clearTrianglesAroundEdge();
	}

	QMap<Edge, int> edgeMap;
	for(Triangle t : m_triangles)
	{
		for(int i=0; i<3; ++i)
		{
			const int p1 = t[i];
			const int p2 = t[(i+1)%3];

			const Edge e = ((p1<p2) ? makeEdge(p1, p2) : makeEdge(p2, p1));

			if(!edgeMap.contains(e))
			{
				const int edgeId = edgeMap.size();
				edgeMap[e] = edgeId;
				m_edges.push_back(makeEdge(p1, p2));
			}
		}
	}
}

void Mesh::createEdgesInTriangleList()
{
	if(hasEdgesInTriangle())
		clearEdgesInTriangle();

	int nbTri = m_triangles.size();
	m_edgesInTriangle.resize(nbTri);

	if(!hasEdges())
	{
		QMap<Edge, int> edgeMap;

		for(int i=0; i<nbTri; ++i)
		{
			const Triangle& t = m_triangles[i];
			for(int j=0; j<3; ++j)
			{
				const int p1 = t[j];
				const int p2 = t[(j+1)%3];

				const Edge e = ((p1<p2) ? makeEdge(p1, p2) : makeEdge(p2, p1));

				if(!edgeMap.contains(e))
				{
					const int edgeId = edgeMap.size();
					edgeMap[e] = edgeId;
					m_edges.push_back(makeEdge(p1, p2));
				}

				m_edgesInTriangle[i][j] = edgeMap[e];
			}
		}
	}
	else
	{
		if(!hasEdgesAroundPoint())
			createEdgesAroundPointList();

		const QVector<EdgesIndicesList>& eapl = getEdgesAroundPointList();

		for(int i=0; i<nbTri; ++i)
		{
			EdgesInTriangle& eit = m_edgesInTriangle[i];
			const Triangle& t = m_triangles[i];
			for(int j=0; j<3; ++j)
			{
				bool found = false;
				const int p1 = t[j], p2 = t[(j+1)%3];
				const EdgesIndicesList& eap = eapl[p1];
				const int nbE = eap.size();
				for(int k=0; k<nbE; ++k)
				{
					const int eid = eap[k];
					const Edge& e = m_edges[eid];
					if(e[0] == p1 && e[1] == p2 || e[0] == p2 && e[1] == p1)
					{
						found = true;
						eit[j] = eid;
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

	m_edgesAroundPoint.resize(nbPoints());
	for(int i=0, nb = nbEdges(); i < nb; ++i)
	{
		m_edgesAroundPoint[m_edges[i][0]].push_back(i);
		m_edgesAroundPoint[m_edges[i][1]].push_back(i);
	}
}

void Mesh::createTrianglesAroundPointList()
{
	if(!hasTriangles())
		return;

	if(hasTrianglesAroundPoint())
		clearTrianglesAroundPoint();

	m_trianglesAroundPoint.resize(nbPoints());
	int nbTri = nbTriangles();
	for(int i=0; i<nbTri; ++i)
	{
		const Triangle& t = m_triangles[i];
		for(int j=0; j<3; ++j)
			m_trianglesAroundPoint[t[j]].push_back(i);
	}
}

void Mesh::createTrianglesAroundEdgeList()
{
	if(!hasTriangles())
		return;

	if(!hasEdges())
		createEdgeList();

	if(!hasEdgesInTriangle())
		createEdgesInTriangleList();

	const int nbTri = nbTriangles();
	const int nbE = nbEdges();

	m_trianglesAroundEdge.resize(nbE);
	for(int i=0; i<nbTri; ++i)
	{
		const EdgesInTriangle& eit = m_edgesInTriangle[i];
		for(int j=0; j<3; ++j)
			m_trianglesAroundEdge[eit[j]].push_back(i);
	}
}

void Mesh::createElementsOnBorder()
{
	if(!hasTrianglesAroundEdge())
		createTrianglesAroundEdgeList();

	if(!hasTrianglesAroundPoint())
		createTrianglesAroundPointList();

	m_pointsOnBorder.clear();
	m_edgesOnBorder.clear();
	m_trianglesOnBorder.clear();

	for(int i=0, nb=nbEdges(); i<nb; ++i)
	{
		if(m_trianglesAroundEdge[i].size() == 1) // On a border
		{
			m_edgesOnBorder.push_back(i);

			const int pt1Id = m_edges[i][0];
			if(!m_pointsOnBorder.contains(pt1Id))
				m_pointsOnBorder.push_back(pt1Id);

			const int pt2Id = m_edges[i][1];
			if(!m_pointsOnBorder.contains(pt2Id))
				m_pointsOnBorder.push_back(pt2Id);
		}
	}

	for(auto pt : m_pointsOnBorder)
	{
		for(auto trian : m_trianglesAroundPoint[pt])
		{
			if(!m_trianglesOnBorder.contains(trian))
				m_trianglesOnBorder.push_back(trian);
		}
	}
}

void Mesh::createTriangles()
{
	if(!hasEdges())
		return;

	if(!hasEdgesAroundPoint())
		createEdgesAroundPointList();

	if(hasTriangles())
		clearTriangles();

	QList<int> tmpEdgesId;
	const int nbE = nbEdges();
	for(int i=0; i<nbE; ++i)
		tmpEdgesId.push_back(i);

	clearTrianglesAroundEdge();
	m_trianglesAroundEdge.resize(nbE);

	while(!tmpEdgesId.empty())
	{
		int eid = tmpEdgesId.front();
		Edge e = getEdge(eid);
		tmpEdgesId.pop_front();

		const EdgesIndicesList& neighbors = getEdgesAroundPoint(e[0]);
		int nbNgh = neighbors.size();
		for(int i=0; i<nbNgh; ++i)
		{
			int e2id = neighbors[i];
			Edge e2 = getEdge(e2id);
			int p2id = getOtherPointInEdge(e2, e[0]);
			int e3id = getEdgeIndex(p2id, e[1]);

			if(e3id != -1)
			{
				Triangle tri = makeTriangle(e[0], p2id, e[1]);

				if(!m_trianglesAroundEdge[eid].empty())
				{
					int tri2 = m_trianglesAroundEdge[eid][0];
					if(tri == getTriangle(tri2))
						continue;
				}

				if(areaOfTriangle(tri) < 0)
					reorientTriangle(tri);

				int triId = addTriangle(tri);

				m_trianglesAroundEdge[eid].push_back(triId);
				if(m_trianglesAroundEdge[eid].size() == 2)
					tmpEdgesId.removeOne(eid);

				m_trianglesAroundEdge[e2id].push_back(triId);
				if(m_trianglesAroundEdge[e2id].size() == 2)
					tmpEdgesId.removeOne(e2id);

				m_trianglesAroundEdge[e3id].push_back(triId);
				if(m_trianglesAroundEdge[e3id].size() == 2)
					tmpEdgesId.removeOne(e3id);
			}
		}
	}
}

void Mesh::clearBorderElementLists()
{
	m_pointsOnBorder.clear();
	m_edgesOnBorder.clear();
	m_trianglesOnBorder.clear();
}

void Mesh::clear()
{
	clearPoints();
	clearEdges();
	clearTriangles();
	clearEdgesInTriangle();
	clearEdgesAroundPoint();
	clearTrianglesAroundPoint();
	clearTrianglesAroundEdge();
	clearBorderElementLists();
}

//****************************************************************************//

void translate(Mesh& mesh, const Point& delta)
{
	for(int i=0, nb = mesh.nbPoints(); i < nb; ++i)
		mesh.getPoint(i) += delta;
}

Mesh translated(const Mesh& mesh, const Point& delta)
{
	Mesh tmp = mesh;
	for(int i=0, nb = tmp.nbPoints(); i < nb; ++i)
		tmp.getPoint(i) += delta;
	return tmp;
}

void scale(Mesh& mesh, PReal scale)
{
	for(int i=0, nb = mesh.nbPoints(); i < nb; ++i)
		mesh.getPoint(i) *= scale;
}

Mesh scaled(const Mesh& mesh, PReal scale)
{
	Mesh tmp = mesh;
	for(int i=0, nb = tmp.nbPoints(); i < nb; ++i)
		tmp.getPoint(i) *= scale;
	return tmp;
}

void rotate(Mesh& mesh, const Point& center, PReal angle)
{
	PReal ca = cos(angle), sa = sin(angle);
	for(int i=0, nb = mesh.nbPoints(); i < nb; ++i)
	{
		Point& point = mesh.getPoint(i);
		Point pt = point - center;
		point = center + Point(pt.x*ca-pt.y*sa, pt.x*sa+pt.y*ca);
	}
}

Mesh rotated(const Mesh& mesh, const Point& center, PReal angle)
{
	PReal ca = cos(angle), sa = sin(angle);
	Mesh tmp = mesh;
	for(int i=0, nb = tmp.nbPoints(); i < nb; ++i)
	{
		Point& point = tmp.getPoint(i);
		Point pt = point - center;
		point = center + Point(pt.x*ca-pt.y*sa, pt.x*sa+pt.y*ca);
	}
	return tmp;
}

//****************************************************************************//

template<> PANDA_CORE_API QString DataTrait<Mesh>::valueTypeName() { return "mesh"; }
template<> PANDA_CORE_API QString DataTrait<Mesh>::valueTypeNamePlural() { return "meshes"; }

template<>
PANDA_CORE_API void DataTrait<Mesh>::writeValue(QDomDocument& doc, QDomElement& elem, const Mesh& v)
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
		edgeNode.setAttribute("p1", e[0]);
		edgeNode.setAttribute("p2", e[1]);
	}

	for(const auto& t : v.getTriangles())
	{
		QDomElement triangleNode = doc.createElement("Triangle");
		elem.appendChild(triangleNode);
		triangleNode.setAttribute("p1", t[0]);
		triangleNode.setAttribute("p2", t[1]);
		triangleNode.setAttribute("p3", t[2]);
	}
}

template<>
PANDA_CORE_API void DataTrait<Mesh>::readValue(QDomElement& elem, Mesh& v)
{
	Mesh tmpMesh;

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
		tmpMesh.addPoint(pt);
		ptNode = ptNode.nextSiblingElement("Point");
	}

	QDomElement edgeNode = elem.firstChildElement("Edge");
	while(!edgeNode.isNull())
	{
		Mesh::Edge edge;
		edge[0] = edgeNode.attribute("p1").toInt();
		edge[1] = edgeNode.attribute("p2").toInt();
		tmpMesh.addEdge(edge);
		edgeNode = edgeNode.nextSiblingElement("Edge");
	}

	QDomElement triangleNode = elem.firstChildElement("Triangle");
	while(!triangleNode.isNull())
	{
		Mesh::Triangle triangle;
		triangle[0] = triangleNode.attribute("p1").toInt();
		triangle[1] = triangleNode.attribute("p2").toInt();
		triangle[2] = triangleNode.attribute("p3").toInt();
		tmpMesh.addTriangle(triangle);
		triangleNode = triangleNode.nextSiblingElement("Triangle");
	}

	v = std::move(tmpMesh);
}

template class Data< Mesh >;
template class Data< QVector<Mesh> >;

int meshDataClass = RegisterData< Mesh >();
int meshVectorDataClass = RegisterData< QVector<Mesh> >();

} // namespace types

} // namespace panda
