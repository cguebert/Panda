#include <panda/graphics/Model.h>

#include <GL/glew.h>

namespace panda
{

namespace graphics
{

void Model::create()
{
	auto vertSize = m_vertices.size();

	m_VAO.create();
	m_VAO.bind();

	// Vertices
	m_verticesVBO.create(BufferType::ArrayBuffer, BufferUsage::DynamicDraw);
	m_verticesVBO.bind();
	m_verticesVBO.allocate(m_vertices);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Texture coordinates
	if (!m_texCoords.empty())
	{
		m_texCoordsVBO.create(BufferType::ArrayBuffer, BufferUsage::DynamicDraw);
		m_texCoordsVBO.bind();
		m_texCoordsVBO.allocate(m_texCoords);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(1);
	}

	// Indices
	if (!m_triangles.empty())
	{
		m_indicesEBO.create(BufferType::ElementArrayBuffer);
		m_indicesEBO.bind();
		m_indicesEBO.allocate(m_triangles);
	}

	m_VAO.release();
	Buffer::release(BufferType::ArrayBuffer);
	m_indicesEBO.release();
}

void Model::clear()
{
	m_VAO.clear();
	m_verticesVBO.clear();
	m_texCoordsVBO.clear();
	m_indicesEBO.clear();
}

void Model::render()
{
	m_VAO.bind();

	if (!m_triangles.empty())
		glDrawElements(GL_TRIANGLES, m_triangles.size() * 3, GL_UNSIGNED_INT, nullptr);
	else
		glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertices.size());

	m_VAO.release();
}

void Model::setVertices(const Vertices& pts)
{
	m_vertices = pts;

	if (m_verticesVBO)
	{
		m_verticesVBO.bind();
		m_verticesVBO.write(pts);
		m_verticesVBO.release();
	}
}

void Model::setVertices(const std::vector<types::Point>& pts)
{
	m_vertices.resize(pts.size());
	std::memcpy(m_vertices.data(), pts.data(), pts.size() * 2 * sizeof(float));

	if (m_verticesVBO)
	{
		m_verticesVBO.bind();
		m_verticesVBO.write(pts);
		m_verticesVBO.release();
	}
}

void Model::setVertices(const std::vector<float>& pts)
{
	int nb = pts.size() / 2;
	m_vertices.resize(nb);
	std::memcpy(m_vertices.data(), pts.data(), nb * 2 * sizeof(float));

	if (m_verticesVBO)
	{
		m_verticesVBO.bind();
		m_verticesVBO.write(pts);
		m_verticesVBO.release();
	}
}

void Model::setTriangles(const Triangles& triangles)
{
	m_triangles = triangles;

	if (m_indicesEBO)
	{
		m_indicesEBO.bind();
		m_indicesEBO.write(triangles);
		m_indicesEBO.release();
	}
}

void Model::setTriangles(const std::vector<unsigned int>& indices)
{
	int nb = indices.size() / 3;
	m_triangles.resize(nb);
	std::memcpy(m_triangles.data(), indices.data(), nb * 3 * sizeof(unsigned int));

	if (m_indicesEBO)
	{
		m_indicesEBO.bind();
		m_indicesEBO.write(indices);
		m_indicesEBO.release();
	}
}

void Model::setTexCoords(const Vertices& texCoords)
{
	m_texCoords.resize(texCoords.size());
	std::memcpy(m_texCoords.data(), texCoords.data(), texCoords.size() * 2 * sizeof(float));

	if (m_texCoordsVBO)
	{
		m_texCoordsVBO.bind();
		m_texCoordsVBO.write(texCoords);
		m_texCoordsVBO.release();
	}
}

void Model::setTexCoords(const std::vector<float>& coords)
{
	int nb = coords.size() / 2;
	m_texCoords.resize(nb);
	std::memcpy(m_texCoords.data(), coords.data(), nb * 2 * sizeof(float));

	if (m_texCoordsVBO)
	{
		m_texCoordsVBO.bind();
		m_texCoordsVBO.write(coords);
		m_texCoordsVBO.release();
	}
}

} // namespace graphics

} // namespace panda
