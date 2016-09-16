#include <GL/glew.h>

#include <panda/document/RenderedDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Color.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Mesh.h>
#include <panda/types/Shader.h>
#include <panda/object/Renderer.h>
#include <panda/graphics/Buffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/graphics/VertexArrayObject.h>

namespace panda {

using types::Color;
using types::ImageWrapper;
using types::Point;
using types::Mesh;
using types::Shader;

class RenderMesh : public Renderer
{
public:
	PANDA_CLASS(RenderMesh, Renderer)

	RenderMesh(RenderedDocument* parent)
		: Renderer(parent)
		, m_meshes(initData("mesh", "Mesh to render"))
		, m_color(initData("color", "Color of the mesh"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_meshes);
		addInput(m_color);
		addInput(m_shader);

		m_color.getAccessor().push_back(Color::black());

		m_shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = m_shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/PT_uniColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/PT_uniColor_noTex.f.glsl");
	}

	void initGL()
	{
		m_VAO.create();
		m_VAO.bind();

		m_verticesVBO.create();
		m_verticesVBO.bind();
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(0);

		m_VAO.release();
	}

	void render()
	{
		const std::vector<Mesh>& inMeshes = m_meshes.getValue();
		std::vector<Color> listColor = m_color.getValue();

		int nbMeshes = inMeshes.size();
		int nbColor = listColor.size();

		if(nbMeshes && nbColor)
		{
			if (nbColor < nbMeshes)
				nbColor = 1;

			if (!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			int colorLocation = m_shaderProgram.uniformLocation("color");
			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			m_VAO.bind();

			for (int i = 0; i < nbMeshes; ++i)
			{
				const auto& mesh = inMeshes[i];
				int nbTri = mesh.nbTriangles();

				m_verticesVBO.bind();
				m_verticesVBO.write(mesh.getPoints());

				m_shaderProgram.setUniformValueArray(colorLocation, listColor[i % nbColor].data(), 1, 4);

				glDrawElements(GL_TRIANGLES, nbTri * 3, GL_UNSIGNED_INT, mesh.getTriangles().data());
			}

			m_shaderProgram.release();
			m_VAO.release();
		}
	}

protected:
	Data< std::vector<Mesh> > m_meshes;
	Data< std::vector<Color> > m_color;
	Data< Shader > m_shader;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO;
};

int RenderMeshClass = RegisterObject<RenderMesh, RenderedDocument>("Render/Filled/Plain mesh").setDescription("Draw a mesh");

//****************************************************************************//

class RenderMeshColoredPoints : public Renderer
{
public:
	PANDA_CLASS(RenderMeshColoredPoints, Renderer)

	RenderMeshColoredPoints(RenderedDocument* parent)
		: Renderer(parent)
		, m_mesh(initData("mesh", "Mesh to render"))
		, m_color(initData("color", "Color of the points"))
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

	void render()
	{
		const Mesh& inMesh = m_mesh.getValue();
		std::vector<Color> listColor = m_color.getValue();

		int nbPts = inMesh.nbPoints();
		int nbTri = inMesh.nbTriangles();
		int nbColor = listColor.size();

		if (nbTri && nbColor)
		{
			if (nbColor < nbPts)
				listColor.assign(nbPts, listColor[0]);

			if (!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			m_verticesVBO.bind();
			m_verticesVBO.write(inMesh.getPoints());

			m_colorsVBO.bind();
			m_colorsVBO.write(listColor);

			m_VAO.bind();

			glDrawElements(GL_TRIANGLES, nbTri * 3, GL_UNSIGNED_INT, inMesh.getTriangles().data());

			m_shaderProgram.release();
			m_VAO.release();
		}
	}

protected:
	Data< Mesh > m_mesh;
	Data< std::vector<Color> > m_color;
	Data< Shader > m_shader;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_colorsVBO;
};

int RenderMeshColoredPointsClass = RegisterObject<RenderMeshColoredPoints, RenderedDocument>("Render/Filled/Mesh").setDescription("Draw a mesh, each point having its own color");

//****************************************************************************//

class RenderMesh_Textured : public Renderer
{
public:
	PANDA_CLASS(RenderMesh_Textured, Renderer)

	RenderMesh_Textured(RenderedDocument* parent)
		: Renderer(parent)
		, m_mesh(initData("mesh", "Mesh to render"))
		, m_uvCoords(initData("UV Coords", "Texture coordinates"))
		, m_texture(initData("texture", "Texture applied to the mesh"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_mesh);
		addInput(m_uvCoords);
		addInput(m_texture);
		addInput(m_shader);

		m_shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = m_shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/PT_noColor_Tex.f.glsl");
	}

	void initGL()
	{
		m_VAO.create();
		m_VAO.bind();

		m_verticesVBO.create();
		m_verticesVBO.bind();
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(0);

		m_texCoordsVBO.create();
		m_texCoordsVBO.bind();
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(1);

		m_trianglesEBO.create(graphics::BufferType::ElementArrayBuffer);
		m_trianglesEBO.bind();

		m_VAO.release();
	}

	void render()
	{
		const Mesh& inMesh = m_mesh.getValue();
		std::vector<Point> listCoords = m_uvCoords.getValue();
		const int texId = m_texture.getValue().getTextureId();

		int nbPts = inMesh.nbPoints();
		int nbTri = inMesh.nbTriangles();
		int nbCoords = listCoords.size();

		if(nbTri && (nbPts == nbCoords) && texId)
		{
			if(!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			m_verticesVBO.bind();
			m_verticesVBO.write(inMesh.getPoints());

			m_texCoordsVBO.bind();
			m_texCoordsVBO.write(listCoords);

			m_trianglesEBO.bind();
			m_trianglesEBO.write(inMesh.getTriangles());

			m_VAO.bind();

			glBindTexture(GL_TEXTURE_2D, texId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			m_shaderProgram.setUniformValue("tex0", 0);

			glDrawElements(GL_TRIANGLES, nbTri * 3, GL_UNSIGNED_INT, nullptr);

			m_VAO.release();
		}
	}

protected:
	Data< Mesh > m_mesh;
	Data< std::vector<Point> > m_uvCoords;
	Data< ImageWrapper > m_texture;
	Data< Shader > m_shader;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_texCoordsVBO, m_trianglesEBO;
};

int RenderMesh_TexturedClass = RegisterObject<RenderMesh_Textured, RenderedDocument>("Render/Textured/Mesh")
		.setName("Textured mesh").setDescription("Draw a textured mesh");

} // namespace panda
