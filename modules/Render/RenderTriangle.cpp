#include <GL/glew.h>

#include <panda/document/RenderedDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Color.h>
#include <panda/types/Mesh.h>
#include <panda/types/Shader.h>
#include <panda/object/Renderer.h>
#include <panda/graphics/Buffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/graphics/VertexArrayObject.h>

namespace panda {

using types::Color;
using types::Point;
using types::Mesh;
using types::Shader;

class RenderTriangle : public Renderer
{
public:
	PANDA_CLASS(RenderTriangle, Renderer)

	RenderTriangle(RenderedDocument* parent)
		: Renderer(parent)
		, m_mesh(initData("mesh", "Triangle to render"))
		, m_color(initData("color", "Color of the triangle"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_mesh);
		addInput(m_color);
		addInput(m_shader);

		m_color.getAccessor().push_back(Color::black());

		m_shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = m_shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/PT_attColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/PT_attColor_noTex.f.glsl");
	}

	void initGL()
	{
		m_VAO.create();
		m_VAO.bind();

		m_verticesVBO.create();
		m_verticesVBO.bind();
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(0);

		m_colorsVBO.create();
		m_colorsVBO.bind();
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(1);

		m_VAO.release();
	}

	void update()
	{
		const Mesh& inMesh = m_mesh.getValue();
		const std::vector<Color>& listColor = m_color.getValue();

		m_verticesBuffer.clear();
		m_colorBuffer.clear();

		if (inMesh.hasTriangles() && inMesh.hasPoints() && !listColor.empty())
		{
			int nbTri = inMesh.nbTriangles();
			int nbColor = listColor.size();

			const auto& points = inMesh.getPoints();
			const auto& triangles = inMesh.getTriangles();

			m_verticesBuffer.reserve(nbTri * 3);
			for (const auto& tri : triangles)
			{
				m_verticesBuffer.push_back(points[tri[0]]);
				m_verticesBuffer.push_back(points[tri[1]]);
				m_verticesBuffer.push_back(points[tri[2]]);
			}

			m_colorBuffer.reserve(nbTri * 3);
			if (nbColor < nbTri)
			{
				m_colorBuffer.assign(nbTri * 3, listColor[0]);
			}
			else
			{
				for (const auto& color : listColor)
				{
					for (int i = 0; i < 3; ++i)
						m_colorBuffer.push_back(color);
				}
			}
		}
	}

	void render()
	{
		if(!m_verticesBuffer.empty() && !m_colorBuffer.empty())
		{
			if(!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_verticesVBO.bind();
			m_verticesVBO.write(m_verticesBuffer);

			m_colorsVBO.bind();
			m_colorsVBO.write(m_colorBuffer);

			m_VAO.bind();

			m_shaderProgram.bind();
			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			glDrawArrays(GL_TRIANGLES, 0, m_verticesBuffer.size());

			m_shaderProgram.release();
			m_VAO.release();
		}
	}

protected:
	Data< Mesh > m_mesh;
	Data< std::vector<Color> > m_color;
	Data< Shader > m_shader;

	std::vector<types::Point> m_verticesBuffer;
	std::vector<types::Color> m_colorBuffer;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_colorsVBO;
};

int RenderTriangleClass = RegisterObject<RenderTriangle, RenderedDocument>("Render/Filled/Triangle").setDescription("Draw a triangle");

} // namespace panda
