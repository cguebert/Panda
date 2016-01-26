#pragma once

#include <panda/graphics/Buffer.h>
#include <panda/graphics/VertexArrayObject.h>

#include <panda/types/Point.h>

namespace panda
{

namespace graphics
{

class PANDA_CORE_API Model
{
public:
	using SPtr = std::shared_ptr<Model>;

	using Vertex = std::array<float, 2>;
	using Vertices = std::vector<Vertex>;
	using Triangle = std::array<unsigned int, 3>;
	using Triangles = std::vector<Triangle>;

	explicit operator bool() const; // Returns true if an OpenGL VAO has been created for this object

	void create();
	void clear();
	void render();

	void setVertices(const Vertices& pts);
	void setVertices(const std::vector<types::Point>& pts);
	void setVertices(const std::vector<float>& pts);

	void setTriangles(const Triangles& triangles);
	void setTriangles(const std::vector<unsigned int>& indices);

	void setTexCoords(const Vertices& texCoords);
	void setTexCoords(const std::vector<float>& coords);

private:
	Vertices m_vertices, m_texCoords;
	Triangles m_triangles;

	VertexArrayObject m_VAO;
	Buffer m_verticesVBO, m_texCoordsVBO, m_indicesEBO;
};

inline Model::operator bool() const
{ return m_VAO.isCreated(); }

} // namespace graphics

} // namespace panda
