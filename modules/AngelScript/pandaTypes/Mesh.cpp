#include "DataWrapper.h"
#include "ObjectWrapper.h"
#include "Types.h"

#include <panda/types/Mesh.h>

#include <new>

using panda::types::Point;
using panda::types::Mesh;
using panda::BaseDataWrapper;
using panda::ObjectWrapper;

namespace panda
{
	class MeshDataWrapper;
	class MeshVectorDataWrapper;
}

namespace
{
	BaseDataWrapper* createMeshData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<Mesh, panda::MeshDataWrapper>(input, name, help); }

	BaseDataWrapper* createMeshVectorData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<std::vector<Mesh>, panda::MeshVectorDataWrapper>(input, name, help); }

	void EdgeDefaultConstructor(Mesh::Edge* self)
	{ new(self) Mesh::Edge({ 0, 0 }); }

	void EdgeCopyConstructor(const Mesh::Edge& other, Mesh::Edge* self)
	{ new(self) Mesh::Edge(other); }

	void EdgeInitConstructor(unsigned int p0, unsigned int p1, Mesh::Edge* self)
	{ new(self) Mesh::Edge({ p0, p1 }); }

	void TriangleDefaultConstructor(Mesh::Triangle* self)
	{ new(self) Mesh::Triangle({ 0, 0 }); }

	void TriangleCopyConstructor(const Mesh::Triangle& other, Mesh::Triangle* self)
	{ new(self) Mesh::Triangle(other); }

	void TriangleInitConstructor(unsigned int p0, unsigned int p1, unsigned int p2, Mesh::Triangle* self)
	{ new(self) Mesh::Triangle({ p0, p1, p2 }); }

}

namespace panda 
{

	// Mesh wrapper class
	class MeshWrapper
	{
	public:
		using points_vector = aatc::container::tempspec::vector<Point>;
		using edges_vector = aatc::container::tempspec::vector<Mesh::Edge>;
		using triangles_vector = aatc::container::tempspec::vector<Mesh::Triangle>;
		using id_vector = aatc::container::tempspec::vector<unsigned int>;

		static MeshWrapper* factory()
		{ return new MeshWrapper(); }

		static MeshWrapper* create(const panda::types::Mesh& mesh)
		{ return new MeshWrapper(mesh); }

		id_vector* createIdVector(const std::vector<unsigned int>& input)
		{
			auto vec = new id_vector();
			vec->container = input;
			return vec;
		}

		MeshWrapper() {}
		MeshWrapper(const panda::types::Mesh& mesh)
			: m_mesh(mesh) {}

		const panda::types::Mesh& mesh() const 
		{ return m_mesh; }

		void addRef()
		{ m_refCount++; }

		void release()
		{ if (--m_refCount == 0) delete this; }
		
		points_vector* getPoints() const
		{
			auto vec = new points_vector();
			vec->container = m_mesh.getPoints();
			return vec; 
		}
	
		void setPoints(const points_vector* vec)
		{ m_mesh.clearPoints(); m_mesh.addPoints(vec->container); }

		edges_vector* getEdges() const
		{
			auto vec = new edges_vector();
			vec->container = m_mesh.getEdges();
			return vec; 
		}
	
		void setEdges(const edges_vector* vec)
		{ m_mesh.clearEdges(); m_mesh.addEdges(vec->container); }

		triangles_vector* getTriangles() const
		{
			auto vec = new triangles_vector();
			vec->container = m_mesh.getTriangles();
			return vec; 
		}
	
		void setTriangles(const triangles_vector* vec)
		{ m_mesh.clearTriangles(); m_mesh.addTriangles(vec->container); }

		bool operator==(const MeshWrapper* wrapper) const
		{ return m_mesh == wrapper->m_mesh; }

		MeshWrapper& assign(const MeshWrapper* other)
		{ m_mesh = other->m_mesh; return *this; }

		MeshWrapper* translatedMesh(const Point& center) const
		{ return create(translated(m_mesh, center)); }
		void translateMesh(const Point& center)
		{ translate(m_mesh, center); }

		MeshWrapper* rotatedMesh(const Point& center, float angle) const
		{ return create(rotated(m_mesh, center, angle)); }
		void rotateMesh(const Point& center, float angle)
		{ rotate(m_mesh, center, angle); }

		MeshWrapper* scaledMesh(float angle) const
		{ return create(scaled(m_mesh, angle)); }
		void scaleMesh(float angle)
		{ scale(m_mesh, angle); }

		unsigned int addPoint(const Point& point)
		{ return m_mesh.addPoint(point); }
		void addPoints(const points_vector* vec)
		{ m_mesh.addPoints(vec->container); }

		unsigned int addEdge(const Mesh::Edge& e)
		{ return m_mesh.addEdge(e); }
		unsigned int addEdge(unsigned int a, unsigned int b)
		{ return m_mesh.addEdge(a, b); }
		void addEdges(const edges_vector* vec)
		{ m_mesh.addEdges(vec->container); }

		unsigned int addTriangle(const Mesh::Triangle& t)
		{ return m_mesh.addTriangle(t); }
		unsigned int addTriangle(unsigned int p1, unsigned int p2, unsigned int p3)
		{ return m_mesh.addTriangle(p1, p2, p3); }
		void addTriangles(const triangles_vector* vec)
		{ m_mesh.addTriangles(vec->container); }

		int nbPoints() const
		{ return m_mesh.nbPoints(); }
		int nbEdges() const
		{ return m_mesh.nbEdges(); }
		int nbTriangles() const
		{ return m_mesh.nbTriangles(); }

		Point getPoint(unsigned int index) const
		{ return m_mesh.getPoint(index); }
		Mesh::Edge getEdge(unsigned int index) const
		{ return m_mesh.getEdge(index); }
		Mesh::Triangle getTriangle(unsigned int index) const
		{ return m_mesh.getTriangle(index); }

		unsigned int getPointIndex(const Point& pt) const
		{ return m_mesh.getPointIndex(pt); }
		unsigned int getEdgeIndex(unsigned int a, unsigned int b) const
		{ return m_mesh.getEdgeIndex(a, b); }
		unsigned int getEdgeIndex(const Mesh::Edge& e) const
		{ return m_mesh.getEdgeIndex(e); }
		unsigned int getTriangleIndex(const Mesh::Triangle& p) const
		{ return m_mesh.getTriangleIndex(p); }
		
		id_vector* getEdgesInTriangle(unsigned int index)
		{ 
			auto tmp = m_mesh.getEdgesInTriangle(index);
			std::vector<unsigned int> tmpVec(tmp.begin(), tmp.end());
			return createIdVector(tmpVec); 
		}

		id_vector* getEdgesAroundPoint(unsigned int index)
		{ return createIdVector(m_mesh.getEdgesAroundPoint(index)); }
		id_vector* getTrianglesAroundPoint(unsigned int index)
		{ return createIdVector(m_mesh.getTrianglesAroundPoint(index)); }
		id_vector* getTrianglesAroundEdge(unsigned int index)
		{ return createIdVector(m_mesh.getTrianglesAroundEdge(index)); }

		id_vector* getPointsOnBorder()
		{ return createIdVector(m_mesh.getPointsOnBorder()); }
		id_vector* getEdgesOnBorder()
		{ return createIdVector(m_mesh.getEdgesOnBorder()); }
		id_vector* getTrianglesOnBorder()
		{ return createIdVector(m_mesh.getTrianglesOnBorder()); }

		id_vector* getTrianglesAroundTriangle(unsigned int index, bool shareEdge)
		{ return createIdVector(m_mesh.getTrianglesAroundTriangle(index, shareEdge)); }
		id_vector* getTrianglesAroundTriangles(const id_vector* listID, bool shareEdge)
		{ return createIdVector(m_mesh.getTrianglesAroundTriangles(listID->container, shareEdge)); }
		id_vector* getTrianglesConnectedToTriangle(unsigned int index, bool shareEdge)
		{ return createIdVector(m_mesh.getTrianglesConnectedToTriangle(index, shareEdge)); }

		unsigned int getOtherPointInEdge(const Mesh::Edge& edge, unsigned int point) const
		{ return m_mesh.getOtherPointInEdge(edge, point); }
		float areaOfTriangle(const Mesh::Triangle& poly) const
		{ return m_mesh.areaOfTriangle(poly); }
		void reorientTriangle(Mesh::Triangle& poly)
		{ m_mesh.reorientTriangle(poly); }
		Point centroidOfTriangle(const Mesh::Triangle& poly) const
		{ return m_mesh.centroidOfTriangle(poly); }
		bool triangleContainsPoint(const Mesh::Triangle& poly, Point pt) const
		{ return m_mesh.triangleContainsPoint(poly, pt); }

		void removeUnusedPoints()
		{ m_mesh.removeUnusedPoints(); }

		bool hasPoints() const
		{ return m_mesh.hasPoints(); }
		bool hasEdges() const
		{ return m_mesh.hasEdges(); }
		bool hasTriangles() const
		{ return m_mesh.hasTriangles(); }
		bool hasEdgesInTriangle() const
		{ return m_mesh.hasEdgesInTriangle(); }
		bool hasEdgesAroundPoint() const
		{ return m_mesh.hasEdgesAroundPoint(); }
		bool hasTrianglesAroundPoint() const
		{ return m_mesh.hasTrianglesAroundPoint(); }
		bool hasTrianglesAroundEdge() const
		{ return m_mesh.hasTrianglesAroundEdge(); }
		bool hasBorderElementsLists() const
		{ return m_mesh.hasBorderElementsLists(); }

		void createEdgeList()
		{ m_mesh.createEdgeList(); }
		void createEdgesInTriangleList()
		{ m_mesh.createEdgesInTriangleList(); }
		void createEdgesAroundPointList()
		{ m_mesh.createEdgesAroundPointList(); }
		void createTrianglesAroundPointList()
		{ m_mesh.createTrianglesAroundPointList(); }
		void createTrianglesAroundEdgeList()
		{ m_mesh.createTrianglesAroundEdgeList(); }
		void createElementsOnBorder()
		{ m_mesh.createElementsOnBorder(); }
		void createTriangles()
		{ m_mesh.createTriangles(); }

		void clearPoints()
		{ m_mesh.clearPoints(); }
		void clearEdges()
		{ m_mesh.clearEdges(); }
		void clearTriangles()
		{ m_mesh.clearTriangles(); }
		void clearEdgesInTriangle()
		{ m_mesh.clearEdgesInTriangle(); }
		void clearEdgesAroundPoint()
		{ m_mesh.clearEdgesAroundPoint(); }
		void clearTrianglesAroundPoint()
		{ m_mesh.clearTrianglesAroundPoint(); }
		void clearTrianglesAroundEdge()
		{ m_mesh.clearTrianglesAroundEdge(); }
		void clearBorderElementLists()
		{ m_mesh.clearBorderElementLists(); }
		void clear()
		{ m_mesh.clear(); }

	private:
		int m_refCount = 1;
		panda::types::Mesh m_mesh;
	};

	class MeshDataWrapper : public BaseDataWrapper
	{
	public:
		MeshDataWrapper(Data<types::Mesh>* data, asIScriptEngine* engine)
			: m_data(data), m_engine(engine) { }

		MeshWrapper* getValue() const
		{ return MeshWrapper::create(m_data->getValue()); }

		void setValue(const MeshWrapper* wrapper)
		{ m_data->setValue(wrapper->mesh()); }

		int getCounter() const
		{ return m_data->getCounter(); }

	private:
		Data<types::Mesh>* m_data = nullptr;
		asIScriptEngine* m_engine = nullptr;
	};

	class MeshVectorDataWrapper : public BaseDataWrapper
	{
	public:
		using value_type = std::vector<Mesh>;
		using data_type = Data<value_type>;
		using script_vector = aatc::container::templated::vector;

		MeshVectorDataWrapper(data_type* data, asIScriptEngine* engine)
			: m_data(data), m_engine(engine)
		{ }

		script_vector* getValue() const
		{
			auto vec = new script_vector(m_engine->GetTypeInfoByName("Mesh"));
			const auto& meshes = m_data->getValue();
			for (const auto& mesh : meshes)
				vec->container.push_back(MeshWrapper::create(mesh));
			return vec;
		}
	
		void setValue(const script_vector* vec)
		{
			auto acc = m_data->getAccessor();
			acc.clear();
			acc.reserve(vec->container.size());
			for (const auto& ptr : vec->container)
				acc.push_back(static_cast<MeshWrapper*>(ptr)->mesh());
		}

		int getCounter() const 
		{ return m_data->getCounter(); }
	
	private:
		data_type* m_data = nullptr;
		asIScriptEngine* m_engine = nullptr;
	};


	void registerMeshData(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("MeshData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("MeshData", "Mesh@ getValue() const",
			asMETHOD(MeshDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("MeshData", "void setValue(const Mesh&)",
			asMETHOD(MeshDataWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("MeshData", "int getCounter()",
			asMETHOD(MeshDataWrapper, getCounter), asCALL_THISCALL); assert(r >= 0);
	}

	void registerMeshVectorDataType(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("MeshVectorData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("MeshVectorData", str("vector<Mesh@>@ getValue() const"),
			asMETHOD(MeshVectorDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("MeshVectorData", str("void setValue(const vector<Mesh@>&)"),
			asMETHOD(MeshVectorDataWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("MeshVectorData", "int getCounter()",
			asMETHOD(MeshVectorDataWrapper, getCounter), asCALL_THISCALL); assert(r >= 0);
	}

	void registerMeshType(asIScriptEngine* engine)
	{
		int r = 0;
		// Mesh::Edge
		r = engine->RegisterObjectType("Edge", sizeof(Mesh::Edge), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_ALLINTS | asGetTypeTraits<Mesh::Edge>()); assert(r >= 0);
		r = engine->RegisterObjectProperty("Edge", "uint p0", 0); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Edge", "uint p1", sizeof(unsigned int)); assert( r >= 0 );
		aatc::container::tempspec::vector<Mesh::Edge>::Register(engine, "Edge");

		r = engine->RegisterObjectBehaviour("Edge", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(EdgeDefaultConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Edge", asBEHAVE_CONSTRUCT, "void f(const Edge &in)", asFUNCTION(EdgeCopyConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Edge", asBEHAVE_CONSTRUCT, "void f(uint, uint)", asFUNCTION(EdgeInitConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );

		// Mesh::Triangle
		r = engine->RegisterObjectType("Triangle", sizeof(Mesh::Triangle), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_ALLINTS | asGetTypeTraits<Mesh::Triangle>()); assert(r >= 0);
		r = engine->RegisterObjectProperty("Triangle", "uint p0", 0); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Triangle", "uint p1", sizeof(unsigned int)); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Triangle", "uint p2", 2 * sizeof(unsigned int)); assert( r >= 0 );
		aatc::container::tempspec::vector<Mesh::Triangle>::Register(engine, "Triangle");

		r = engine->RegisterObjectBehaviour("Triangle", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(TriangleDefaultConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Triangle", asBEHAVE_CONSTRUCT, "void f(const Triangle &in)", asFUNCTION(TriangleCopyConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Triangle", asBEHAVE_CONSTRUCT, "void f(uint, uint, uint)", asFUNCTION(TriangleInitConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );

		// Mesh
		r = engine->RegisterObjectType("Mesh", 0, asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Mesh", asBEHAVE_FACTORY, "Mesh@ f()", asFUNCTION(MeshWrapper::factory), asCALL_CDECL); assert( r >= 0 );

		r = engine->RegisterObjectBehaviour("Mesh", asBEHAVE_ADDREF, "void f()", asMETHOD(MeshWrapper, addRef), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Mesh", asBEHAVE_RELEASE, "void f()", asMETHOD(MeshWrapper, release), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Mesh", "Mesh& opAssign(const Mesh &in)", asMETHOD(MeshWrapper, assign), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Mesh", "bool opEquals(const Mesh&in) const", asMETHOD(MeshWrapper, operator==), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Mesh", "vector<Point>@ getPoints()", asMETHOD(MeshWrapper, getPoints), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void setPoints(const vector<Point>&in)", asMETHOD(MeshWrapper, setPoints), asCALL_THISCALL); assert(r >= 0);
	
		r = engine->RegisterObjectMethod("Mesh", "vector<Edge>@ getEdges()", asMETHOD(MeshWrapper, getEdges), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void setEdges(const vector<Edge>&in)", asMETHOD(MeshWrapper, setEdges), asCALL_THISCALL); assert(r >= 0);
	
		r = engine->RegisterObjectMethod("Mesh", "vector<Triangle>@ getTriangles()", asMETHOD(MeshWrapper, getTriangles), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void setTriangles(const vector<Triangle>&in)", asMETHOD(MeshWrapper, setTriangles), asCALL_THISCALL); assert(r >= 0);
	
		r = engine->RegisterObjectMethod("Mesh", "Mesh@ translated(const Point &in) const", asMETHOD(MeshWrapper, translatedMesh), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void translate(const Point &in)", asMETHOD(MeshWrapper, translateMesh), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "Mesh@ rotated(const Point &in, float) const", asMETHOD(MeshWrapper, rotatedMesh), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void rotate(const Point &in, float)", asMETHOD(MeshWrapper, rotateMesh), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "Mesh@ scaled(float) const", asMETHOD(MeshWrapper, scaledMesh), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void scale(float)", asMETHOD(MeshWrapper, scaleMesh), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "uint addPoint(const Point &in)", asMETHOD(MeshWrapper, addPoint), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "uint addPoints(const vector<Point>&in)", asMETHOD(MeshWrapper, addPoints), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "uint addEdge(const Edge &in)", asMETHODPR(MeshWrapper, addEdge, (const Mesh::Edge&), unsigned int), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "uint addEdge(uint, uint)", asMETHODPR(MeshWrapper, addEdge, (unsigned int, unsigned int), unsigned int), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "uint addEdges(const vector<Edge>&in)", asMETHOD(MeshWrapper, addEdges), asCALL_THISCALL); assert(r >= 0);
		
		r = engine->RegisterObjectMethod("Mesh", "uint addTriangle(const Triangle &in)", asMETHODPR(MeshWrapper, addTriangle, (const Mesh::Triangle&), unsigned int), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "uint addTriangle(uint, uint)", asMETHODPR(MeshWrapper, addTriangle, (unsigned int, unsigned int, unsigned int), unsigned int), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "uint addTriangles(const vector<Triangle>&in)", asMETHOD(MeshWrapper, addTriangles), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "int nbPoints()", asMETHOD(MeshWrapper, nbPoints), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "int nbEdges()", asMETHOD(MeshWrapper, nbEdges), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "int nbTriangles()", asMETHOD(MeshWrapper, nbTriangles), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "Point getPoint(uint)", asMETHOD(MeshWrapper, getPoint), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "Edge getEdge(uint)", asMETHOD(MeshWrapper, getEdge), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "Triangle getTriangle(uint)", asMETHOD(MeshWrapper, getTriangle), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "uint getPointIndex(const Point &in)", asMETHOD(MeshWrapper, getPointIndex), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "uint getEdgeIndex(const Edge &in)", asMETHODPR(MeshWrapper, getEdgeIndex, (const Mesh::Edge&) const, unsigned int), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "uint getEdgeIndex(uint, uint)", asMETHODPR(MeshWrapper, getEdgeIndex, (unsigned int, unsigned int) const, unsigned int), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "uint getTriangleIndex(const Triangle &in)", asMETHOD(MeshWrapper, getTriangleIndex), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "vector<uint>@ getEdgesInTriangle(uint)", asMETHOD(MeshWrapper, getEdgesInTriangle), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "vector<uint>@ getEdgesAroundPoint(uint)", asMETHOD(MeshWrapper, getEdgesAroundPoint), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "vector<uint>@ getTrianglesAroundPoint(uint)", asMETHOD(MeshWrapper, getTrianglesAroundPoint), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "vector<uint>@ getTrianglesAroundEdge(uint)", asMETHOD(MeshWrapper, getTrianglesAroundEdge), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "vector<uint>@ getPointsOnBorder()", asMETHOD(MeshWrapper, getPointsOnBorder), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "vector<uint>@ getEdgesOnBorder()", asMETHOD(MeshWrapper, getEdgesOnBorder), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "vector<uint>@ getTrianglesOnBorder()", asMETHOD(MeshWrapper, getTrianglesOnBorder), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "vector<uint>@ getTrianglesAroundTriangle(uint, bool)", asMETHOD(MeshWrapper, getTrianglesAroundTriangle), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "vector<uint>@ getTrianglesAroundTriangles(const vector<uint>&, bool)", asMETHOD(MeshWrapper, getTrianglesAroundTriangles), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "vector<uint>@ getTrianglesConnectedToTriangle(uint, bool)", asMETHOD(MeshWrapper, getTrianglesConnectedToTriangle), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "uint getOtherPointInEdge(const Edge &in, uint)", asMETHOD(MeshWrapper, getOtherPointInEdge), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "float areaOfTriangle(const Triangle &in)", asMETHOD(MeshWrapper, areaOfTriangle), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void reorientTriangle(const Triangle &in)", asMETHOD(MeshWrapper, reorientTriangle), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "Point centroidOfTriangle(const Triangle &in)", asMETHOD(MeshWrapper, centroidOfTriangle), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "bool triangleContainsPoint(const Triangle &in, Point)", asMETHOD(MeshWrapper, triangleContainsPoint), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "void removeUnusedPoints()", asMETHOD(MeshWrapper, removeUnusedPoints), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "bool hasPoints()", asMETHOD(MeshWrapper, hasPoints), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "bool hasPhasEdgesoints()", asMETHOD(MeshWrapper, hasEdges), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "bool hasTriangles()", asMETHOD(MeshWrapper, hasTriangles), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "bool hasEdgesInTriangle()", asMETHOD(MeshWrapper, hasEdgesInTriangle), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "bool hasEdgesAroundPoint()", asMETHOD(MeshWrapper, hasEdgesAroundPoint), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "bool hasTrianglesAroundPoint()", asMETHOD(MeshWrapper, hasTrianglesAroundPoint), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "bool hasTrianglesAroundEdge()", asMETHOD(MeshWrapper, hasTrianglesAroundEdge), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "bool hasBorderElementsLists()", asMETHOD(MeshWrapper, hasBorderElementsLists), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "void createEdgeList()", asMETHOD(MeshWrapper, createEdgeList), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void createEdgesInTriangleList()", asMETHOD(MeshWrapper, createEdgesInTriangleList), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void createEdgesAroundPointList()", asMETHOD(MeshWrapper, createEdgesAroundPointList), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void createTrianglesAroundPointList()", asMETHOD(MeshWrapper, createTrianglesAroundPointList), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void createTrianglesAroundEdgeList()", asMETHOD(MeshWrapper, createTrianglesAroundEdgeList), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void createElementsOnBorder()", asMETHOD(MeshWrapper, createElementsOnBorder), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void createTriangles()", asMETHOD(MeshWrapper, createTriangles), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "void clearPoints()", asMETHOD(MeshWrapper, clearPoints), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void clearEdges()", asMETHOD(MeshWrapper, clearEdges), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void clearTriangles()", asMETHOD(MeshWrapper, clearTriangles), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void clearEdgesInTriangle()", asMETHOD(MeshWrapper, clearEdgesInTriangle), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void clearEdgesAroundPoint()", asMETHOD(MeshWrapper, clearEdgesAroundPoint), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void clearTrianglesAroundPoint()", asMETHOD(MeshWrapper, clearTrianglesAroundPoint), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void clearTrianglesAroundEdge()", asMETHOD(MeshWrapper, clearTrianglesAroundEdge), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void clearBorderElementLists()", asMETHOD(MeshWrapper, clearBorderElementLists), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void clear()", asMETHOD(MeshWrapper, clear), asCALL_THISCALL); assert(r >= 0);
	}

	void registerMesh(asIScriptEngine* engine)
	{
		registerMeshType(engine);
		registerMeshData(engine);
		registerMeshVectorDataType(engine);

		int r = engine->RegisterObjectMethod("PandaObject", "MeshData@ createMeshData(bool, const string &in, const string &in)",
			asFUNCTION(createMeshData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		engine->RegisterObjectMethod("PandaObject", "MeshVectorData@ createMeshVectorData(bool, const string &in, const string &in)",
			asFUNCTION(createMeshVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

} // namespace panda
