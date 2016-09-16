#include <GL/glew.h>

#include <panda/document/RenderedDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Color.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/IntVector.h>
#include <panda/types/Path.h>
#include <panda/types/Shader.h>
#include <panda/object/Renderer.h>
#include <panda/graphics/Buffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/graphics/VertexArrayObject.h>

namespace panda {

using types::Color;
using types::ImageWrapper;
using types::IntVector;
using types::Point;
using types::Path;
using types::Shader;

class RenderConvexPolygon : public Renderer
{
public:
	PANDA_CLASS(RenderConvexPolygon, Renderer)

	RenderConvexPolygon(RenderedDocument* parent)
		: Renderer(parent)
		, m_polygons(initData("polygon", "Convex polygon to render"))
		, m_color(initData("color", "Color of the polygon"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_polygons);
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
		const std::vector<Path>& inPolygons = m_polygons.getValue();
		std::vector<Color> listColor = m_color.getValue();

		int nbPolygons = inPolygons.size();
		int nbColor = listColor.size();

		if(nbPolygons && nbColor)
		{
			if (nbColor < nbPolygons)
				nbColor = 1;

			if (!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			int colorLocation = m_shaderProgram.uniformLocation("color");
			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			m_VAO.bind();

			for (int i = 0; i < nbPolygons; ++i)
			{
				const auto& points = inPolygons[i].points;
				int nbPts = points.size();
				if (points.back() == points.front())
					--nbPts;

				m_verticesVBO.bind();
				m_verticesVBO.write(points);

				m_shaderProgram.setUniformValueArray(colorLocation, listColor[i % nbColor].data(), 1, 4);

				glDrawArrays(GL_TRIANGLE_FAN, 0, nbPts);
			}

			m_shaderProgram.release();
			m_VAO.release();
		}
	}

protected:
	Data< std::vector<Path> > m_polygons;
	Data< std::vector<Color> > m_color;
	Data< Shader > m_shader;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO;
};

int RenderConvexPolygonClass = RegisterObject<RenderConvexPolygon, RenderedDocument>("Render/Filled/Plain convex polygon").setDescription("Draw a convex polygon");

//****************************************************************************//

class RenderConvexPolygon_ColoredPoints : public Renderer
{
public:
	PANDA_CLASS(RenderConvexPolygon_ColoredPoints, Renderer)

	RenderConvexPolygon_ColoredPoints(RenderedDocument* parent)
		: Renderer(parent)
		, m_points(initData("points", "Points of the polygons"))
		, m_indices(initData("indices", "Indices to create the polygons"))
		, m_color(initData("color", "Color of the points"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_points);
		addInput(m_indices);
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
		const auto& inPoints = m_points.getValue();
		const auto& inIndices = m_indices.getValue();
		std::vector<Color> listColor = m_color.getValue();

		int nbPts = inPoints.size();
		int nbPoly = inIndices.size();
		int nbColor = listColor.size();

		if (nbPts && nbPoly && nbColor)
		{
			if (nbColor < nbPts)
				listColor.assign(nbPts, listColor[0]);

			if (!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			m_verticesVBO.bind();
			m_verticesVBO.write(inPoints);

			m_colorsVBO.bind();
			m_colorsVBO.write(listColor);

			m_VAO.bind();

			for (int i = 0; i < nbPoly; ++i)
			{
				const auto& indices = inIndices[i].values;
				int nbIds = indices.size();
				if (indices.back() == indices.front())
					--nbIds;

				glDrawElements(GL_TRIANGLE_FAN, nbIds, GL_UNSIGNED_INT, indices.data());
			}

			m_shaderProgram.release();
			m_VAO.release();
		}
	}

protected:
	Data< std::vector<Point> > m_points;
	Data< std::vector<IntVector> > m_indices;
	Data< std::vector<Color> > m_color;
	Data< Shader > m_shader;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_colorsVBO;
};

int RenderConvexPolygon_ColoredPointsClass = RegisterObject<RenderConvexPolygon_ColoredPoints, RenderedDocument>("Render/Filled/Convex polygon").setDescription("Draw a convex polygon, each point having its own color");

//****************************************************************************//

class RenderConvexPolygon_Textured : public Renderer
{
public:
	PANDA_CLASS(RenderConvexPolygon_Textured, Renderer)

	RenderConvexPolygon_Textured(RenderedDocument* parent)
		: Renderer(parent)
		, m_points(initData("points", "Points of the polygons"))
		, m_indices(initData("indices", "Indices to create the polygons"))
		, m_uvCoords(initData("UV Coords", "Texture coordinates"))
		, m_texture(initData("texture", "Texture applied to the polygon"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_points);
		addInput(m_indices);
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

		m_VAO.release();
	}

	void render()
	{
		const auto& inPoints = m_points.getValue();
		const auto& inIndices = m_indices.getValue();
		std::vector<Point> listCoords = m_uvCoords.getValue();
		const int texId = m_texture.getValue().getTextureId();

		int nbPts = inPoints.size();
		int nbPoly = inIndices.size();
		int nbCoords = listCoords.size();

		if (nbPts && (nbPts == nbCoords) && texId)
		{
			if (!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			m_verticesVBO.bind();
			m_verticesVBO.write(inPoints);

			m_texCoordsVBO.bind();
			m_texCoordsVBO.write(listCoords);

			m_VAO.bind();

			glBindTexture(GL_TEXTURE_2D, texId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			m_shaderProgram.setUniformValue("tex0", 0);

			for (int i = 0; i < nbPoly; ++i)
			{
				const auto& indices = inIndices[i].values;
				int nbIds = indices.size();
				if (indices.back() == indices.front())
					--nbIds;

				glDrawElements(GL_TRIANGLE_FAN, nbIds, GL_UNSIGNED_INT, indices.data());
			}

			m_VAO.release();
		}
	}

protected:
	Data< std::vector<Point> > m_points;
	Data< std::vector<IntVector> > m_indices;
	Data< std::vector<Point> > m_uvCoords;
	Data< ImageWrapper > m_texture;
	Data< Shader > m_shader;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_texCoordsVBO;
};

int RenderConvexPolygon_TexturedClass = RegisterObject<RenderConvexPolygon_Textured, RenderedDocument>("Render/Textured/Convex polygon")
	.setName("Textured convex polygon").setDescription("Draw a textured convex polygon");

} // namespace panda
