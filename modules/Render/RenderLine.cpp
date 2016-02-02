#include <GL/glew.h>

#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/Gradient.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Path.h>
#include <panda/types/Shader.h>
#include <panda/helper/algorithm.h>
#include <panda/helper/GradientCache.h>
#include <panda/graphics/Buffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/graphics/VertexArrayObject.h>

namespace panda {

using types::Color;
using types::Gradient;
using types::ImageWrapper;
using types::Point;
using types::Path;
using types::Shader;
using helper::GradientCache;

class RenderLine : public Renderer
{
public:
	PANDA_CLASS(RenderLine, Renderer)

	RenderLine(PandaDocument* parent)
		: Renderer(parent)
		, m_inputA(initData("point 1", "Start of the line"))
		, m_inputB(initData("point 2", "End of the line"))
		, m_width(initData("width", "Width of the line"))
		, m_color(initData("color", "Color of the line"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_inputA);
		addInput(m_inputB);
		addInput(m_width);
		addInput(m_color);
		addInput(m_shader);

		m_color.getAccessor().push_back(Color::black());
		m_width.getAccessor().push_back(1.f);

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

	void extrude(const Point& ptA, const Point& ptB, float width)
	{
		Point dir = ptB - ptA;
		dir.normalize();
		Point disp = Point(-dir.y, dir.x) * (width / 2);
		m_verticesBuffer.push_back(ptA + disp); // Duplicated to cut the strip
		m_verticesBuffer.push_back(ptA + disp);
		m_verticesBuffer.push_back(ptA - disp);
		m_verticesBuffer.push_back(ptB + disp);
		m_verticesBuffer.push_back(ptB - disp);
		m_verticesBuffer.push_back(ptB - disp); // Duplicated to cut the strip
	}

	void update()
	{
		const std::vector<Point>& valA = m_inputA.getValue();
		const std::vector<Point>& valB = m_inputB.getValue();
		const std::vector<Color>& listColor = m_color.getValue();
		const std::vector<PReal>& listWidth = m_width.getValue();

		m_verticesBuffer.clear();
		m_colorsBuffer.clear();

		int nbA = valA.size(), nbB = valB.size();
		int nbLines = std::min(valA.size(), valB.size());
		bool useTwoLists = true;
		if (nbA && !nbB)
		{
			useTwoLists = false;
			nbLines = nbA / 2;
		}

		if (nbLines && !listColor.empty() && !listWidth.empty())
		{
			int nbVertices = nbLines * 2;
			int nbWidth = listWidth.size();
			if (nbWidth < nbLines) nbWidth = 1;

			m_verticesBuffer.reserve(nbVertices * 3);
			if (useTwoLists)
			{
				for (int i = 0; i < nbLines; ++i)
					extrude(valA[i], valB[i], listWidth[i % nbWidth]);
			}
			else
			{
				for (int i = 0; i < nbLines; ++i)
					extrude(valA[i*2], valA[i*2+1], listWidth[i % nbWidth]);
			}

			m_colorsBuffer.reserve(nbVertices * 3);
			int nbColor = listColor.size();
			if (nbColor < nbLines) // One color
			{
				m_colorsBuffer.assign(nbVertices * 3, listColor[0]);
			}
			else if (nbColor >= nbVertices) // A color per point
			{
				for (int i = 0; i < nbVertices; ++i)
				{
					auto color = listColor[i];
					for (int j = 0; j < 3; ++j)
						m_colorsBuffer.push_back(color);
				}
			}
			else // A color per line
			{
				for (int i = 0; i < nbLines; ++i)
				{
					auto color = listColor[i];
					for (int j = 0; j < 6; ++j)
						m_colorsBuffer.push_back(color);
				}
			}
		}
	}

	void render()
	{
		if (!m_verticesBuffer.empty() && !m_colorsBuffer.empty())
		{
			if (!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_verticesVBO.bind();
			m_verticesVBO.write(m_verticesBuffer);

			m_colorsVBO.bind();
			m_colorsVBO.write(m_colorsBuffer);

			m_shaderProgram.bind();
			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			m_VAO.bind();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, m_verticesBuffer.size());
			m_VAO.release();

			m_shaderProgram.release();
		}
	}

protected:
	Data< std::vector<Point> > m_inputA, m_inputB;
	Data< std::vector<PReal> > m_width;
	Data< std::vector<Color> > m_color;
	Data< Shader > m_shader;

	std::vector<types::Point> m_verticesBuffer;
	std::vector<types::Color> m_colorsBuffer;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_colorsVBO;
};

int RenderLineClass = RegisterObject<RenderLine>("Render/Line/Line").setDescription("Draw a line between 2 points");

//****************************************************************************//

class RenderPath : public Renderer
{
public:
	PANDA_CLASS(RenderPath, Renderer)

	RenderPath(PandaDocument* parent)
		: Renderer(parent)
		, m_input(initData("path", "Path to be drawn"))
		, m_lineWidth(initData("width", "Width of the line"))
		, m_color(initData("color", "Color of the line"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_input);
		addInput(m_lineWidth);
		addInput(m_color);
		addInput(m_shader);

		m_color.getAccessor().push_back(Color::black());
		m_lineWidth.getAccessor().push_back(1.f);

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

	void extrude(const Point& ptA, const Point& ptB, float width)
	{
		Point dir = ptB - ptA;
		dir.normalize();
		Point disp = Point(-dir.y, dir.x) * (width / 2);
		m_verticesBuffer.push_back(ptA + disp);
		m_verticesBuffer.push_back(ptA - disp);
		m_verticesBuffer.push_back(ptB + disp);
		m_verticesBuffer.push_back(ptB - disp);
	}

	void update()
	{
		const std::vector<Path>& listPaths = m_input.getValue();
		const std::vector<Color>& listColor = m_color.getValue();
		const std::vector<PReal>& listWidth = m_lineWidth.getValue();

		m_verticesBuffer.clear();

		m_firstBuffer.clear();
		m_countBuffer.clear();

		if (!listPaths.empty() && !listColor.empty() && !listWidth.empty())
		{
			int nbPaths = listPaths.size();
			int nbWidth = listWidth.size();
			if (nbWidth < nbPaths) nbWidth = 1;

			for (int i = 0; i < nbPaths; ++i)
			{
				const auto& path = listPaths[i];
				PReal width = listWidth[i % nbWidth];
				m_firstBuffer.push_back(m_verticesBuffer.size());

				int nbPts = path.size();
				if(nbPts < 2)
				{
					m_countBuffer.push_back(0);
					continue;
				}

				m_countBuffer.push_back((nbPts - 1) * 4);

				for (int j = 0; j < nbPts - 1; ++j)
					extrude(path[j], path[j + 1], width);
			}
		}
	}

	void render()
	{
		const std::vector<Path>& listPaths = m_input.getValue();
		const std::vector<Color>& listColor = m_color.getValue();

		int nbPaths = listPaths.size();
		int nbColor = listColor.size();

		if (!m_verticesBuffer.empty() && nbColor)
		{
			if (!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_verticesVBO.bind();
			m_verticesVBO.write(m_verticesBuffer);

			m_VAO.bind();

			m_shaderProgram.bind();
			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			if (nbColor < nbPaths) nbColor = 1;
			int colorLocation = m_shaderProgram.uniformLocation("color");

			if (nbColor == 1)
			{
				m_shaderProgram.setUniformValueArray(colorLocation, listColor[0].data(), 1, 4);
				glMultiDrawArrays(GL_TRIANGLE_STRIP, m_firstBuffer.data(), m_countBuffer.data(), m_countBuffer.size());
			}
			else
			{
				for (int i = 0; i < nbPaths; ++i)
				{
					if (!m_countBuffer[i])
						continue;

					m_shaderProgram.setUniformValueArray(colorLocation, listColor[i % nbColor].data(), 1, 4);
					glDrawArrays(GL_TRIANGLE_STRIP, m_firstBuffer[i], m_countBuffer[i]);
				}
			}

			m_shaderProgram.release();
			m_VAO.release();
		}
	}

protected:
	Data< std::vector<Path> > m_input;
	Data< std::vector<PReal> > m_lineWidth;
	Data< std::vector<Color> > m_color;
	Data< Shader > m_shader;

	std::vector<types::Point> m_verticesBuffer;
	std::vector<GLint> m_firstBuffer;
	std::vector<GLsizei> m_countBuffer;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO;
};

int RenderPathClass = RegisterObject<RenderPath>("Render/Line/Path").setDescription("Draw a path");


//****************************************************************************//

class RenderGradientPath : public Renderer
{
public:
	PANDA_CLASS(RenderGradientPath, Renderer)

	RenderGradientPath(PandaDocument* parent)
		: Renderer(parent)
		, m_input(initData("path", "Path to be drawn"))
		, m_lineWidth(initData("width", "Width of the line"))
		, m_gradient(initData("gradient", "Gradient applied to the line"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_input);
		addInput(m_lineWidth);
		addInput(m_gradient);
		addInput(m_shader);

		m_gradient.getAccessor().push_back(Gradient::defaultGradient());
		m_lineWidth.getAccessor().push_back(1.f);

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

	void extrude(const Point& ptA, const Point& ptB, float width)
	{
		Point dir = ptB - ptA;
		dir.normalize();
		Point disp = Point(-dir.y, dir.x) * (width / 2);
		m_verticesBuffer.push_back(ptA + disp);
		m_verticesBuffer.push_back(ptA - disp);
		m_verticesBuffer.push_back(ptB + disp);
		m_verticesBuffer.push_back(ptB - disp);
	}

	void update()
	{
		const std::vector<Path>& listPaths = m_input.getValue();
		const std::vector<Gradient>& listGradient = m_gradient.getValue();
		const std::vector<PReal>& listWidth = m_lineWidth.getValue();
		int nbPaths = listPaths.size();

		m_verticesBuffer.clear();
		m_texCoordsBuffer.clear();

		m_firstBuffer.clear();
		m_firstBuffer.reserve(nbPaths);
		m_countBuffer.clear();
		m_countBuffer.reserve(nbPaths);

		if (!listPaths.empty() && !listGradient.empty() && !listWidth.empty())
		{
			int nbWidth = listWidth.size();
			if (nbWidth < nbPaths) nbWidth = 1;

			m_pathLengths.clear();
			m_pathLengths.resize(nbPaths);

			for (int i = 0; i < nbPaths; ++i)
			{
				const auto& path = listPaths[i];
				PReal width = listWidth[i % nbWidth];
				int start = m_verticesBuffer.size();

				m_pathLengths[i] = 0;
				m_firstBuffer.push_back(start);
				m_countBuffer.push_back(0); // Will be modified later

				int nbPts = path.size();
				if (nbPts < 2)
					continue;
				
				// Compute the texture coordinates
				std::vector<types::Point> uvList;
				uvList.reserve((nbPts - 1) * 4);
				uvList.push_back(Point(0.f, 0.f));
				uvList.push_back(Point(0.f, 0.f));
				
				PReal length = 0;
				for (int j = 0; j < nbPts - 1; ++j)
				{
					length += (path[j+1] - path[j]).norm();
					auto pt = Point(length, 0);

					for (int k = 0; k < 4; ++k)
						uvList.push_back(pt);
				}

				if (length < 1e-3)
					continue; // Ignore this path

				m_pathLengths[i] = length;

				for(auto& uv : uvList)
					uv.x /= length;

				for (int j = 0; j < nbPts - 1; ++j)
					extrude(path[j], path[j + 1], width);

				m_countBuffer.back() = (nbPts - 1) * 4;

				m_texCoordsBuffer.insert(m_texCoordsBuffer.end(), uvList.begin(), uvList.end());
			}
		}
	}

	void render()
	{
		const std::vector<Gradient>& listGradient = m_gradient.getValue();
		int nbGradient = listGradient.size();

		if (!m_verticesBuffer.empty() && nbGradient)
		{
			if (!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_verticesVBO.bind();
			m_verticesVBO.write(m_verticesBuffer);

			m_texCoordsVBO.bind();
			m_texCoordsVBO.write(m_texCoordsBuffer);

			int nbPaths = m_countBuffer.size();
			if (nbGradient < nbPaths) nbGradient = 1;

			m_shaderProgram.bind();
			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			m_shaderProgram.setUniformValue("tex0", 0);

			m_VAO.bind();

			// Optimization when we use only one gradient
			if (nbGradient == 1)
			{
				PReal maxLength = *std::max_element(m_pathLengths.begin(), m_pathLengths.end());
				maxLength = helper::bound(64.f, std::ceil(maxLength), 1024.f); // We limit the texture at 1024 pixels
				GLuint texture = GradientCache::getInstance()->getTexture(listGradient.front(), static_cast<int>(maxLength));

				glBindTexture(GL_TEXTURE_2D, texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				glMultiDrawArrays(GL_TRIANGLE_STRIP, m_firstBuffer.data(), m_countBuffer.data(), m_countBuffer.size());
			}
			else
			{
				for (int i = 0; i < nbPaths; ++i)
				{
					if (!m_countBuffer[i])
						continue;

					PReal length = helper::bound(64.f, std::ceil(m_pathLengths[i]), 1024.f); // We limit the texture at 1024 pixels
					GLuint texture = GradientCache::getInstance()->getTexture(listGradient[i % nbGradient], static_cast<int>(length));

					glBindTexture(GL_TEXTURE_2D, texture);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

					glDrawArrays(GL_LINE_STRIP, m_firstBuffer[i], m_countBuffer[i]);
				}
			}

			m_shaderProgram.release();
			m_VAO.release();
		}
	}

protected:
	Data< std::vector<Path> > m_input;
	Data< std::vector<PReal> > m_lineWidth;
	Data< std::vector<Gradient> > m_gradient;
	Data< Shader > m_shader;

	std::vector<PReal> m_pathLengths;
	std::vector<types::Point> m_verticesBuffer, m_texCoordsBuffer;
	std::vector<GLint> m_firstBuffer;
	std::vector<GLsizei> m_countBuffer;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_texCoordsVBO;
};

int RenderGradientPathClass = RegisterObject<RenderGradientPath>("Render/Gradient/Path")
		.setName("Gradient path").setDescription("Draw a path using a gradient");

//****************************************************************************//

class RenderTexturedPath : public Renderer
{
public:
	PANDA_CLASS(RenderTexturedPath, Renderer)

	RenderTexturedPath(PandaDocument* parent)
		: Renderer(parent)
		, m_input(initData("path", "Path to be drawn"))
		, m_lineWidth(initData("width", "Width of the line"))
		, m_texture(initData("gradient", "Gradient applied to the line"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_input);
		addInput(m_lineWidth);
		addInput(m_texture);
		addInput(m_shader);

		m_lineWidth.getAccessor().push_back(1.f);

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

	void extrude(const Point& ptA, const Point& ptB, float width)
	{
		Point dir = ptB - ptA;
		dir.normalize();
		Point disp = Point(-dir.y, dir.x) * (width / 2);
		m_verticesBuffer.push_back(ptA + disp);
		m_verticesBuffer.push_back(ptA - disp);
		m_verticesBuffer.push_back(ptB + disp);
		m_verticesBuffer.push_back(ptB - disp);
	}

	void update()
	{
		const std::vector<Path>& listPaths = m_input.getValue();
		const std::vector<ImageWrapper>& listTextures = m_texture.getValue();
		const std::vector<PReal>& listWidth = m_lineWidth.getValue();
		int nbPaths = listPaths.size();

		m_verticesBuffer.clear();
		m_texCoordsBuffer.clear();

		m_firstBuffer.clear();
		m_firstBuffer.reserve(nbPaths);
		m_countBuffer.clear();
		m_countBuffer.reserve(nbPaths);

		if (!listPaths.empty() && !listTextures.empty() && !listWidth.empty())
		{
			int nbWidth = listWidth.size();
			if (nbWidth < nbPaths) nbWidth = 1;

			m_pathLengths.clear();
			m_pathLengths.resize(nbPaths);

			for (int i = 0; i < nbPaths; ++i)
			{
				const auto& path = listPaths[i];
				PReal width = listWidth[i % nbWidth];
				int start = m_verticesBuffer.size();

				m_pathLengths[i] = 0;
				m_firstBuffer.push_back(start);
				m_countBuffer.push_back(0); // Will be modified later

				int nbPts = path.size();
				if (nbPts < 2)
					continue;
				
				// Compute the texture coordinates
				std::vector<types::Point> uvList;
				uvList.reserve((nbPts - 1) * 4);
				uvList.push_back(Point(0.f, 1.f));
				uvList.push_back(Point(0.f, 0.f));
				
				PReal length = 0;
				for (int j = 0; j < nbPts - 1; ++j)
				{
					length += (path[j+1] - path[j]).norm();
					auto pt1 = Point(length, 1);
					auto pt0 = Point(length, 0);

					uvList.push_back(pt1);
					uvList.push_back(pt0);
					uvList.push_back(pt1);
					uvList.push_back(pt0);
				}

				if (length < 1e-3)
					continue; // Ignore this path

				m_pathLengths[i] = length;

				for(auto& uv : uvList)
					uv.x /= length;

				for (int j = 0; j < nbPts - 1; ++j)
					extrude(path[j], path[j + 1], width);

				m_countBuffer.back() = (nbPts - 1) * 4;

				m_texCoordsBuffer.insert(m_texCoordsBuffer.end(), uvList.begin(), uvList.end());
			}
		}
	}

	void render()
	{
		const auto& listTextures = m_texture.getValue();
		int nbTextures = listTextures.size();

		if (!m_verticesBuffer.empty() && nbTextures)
		{
			if (!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_verticesVBO.bind();
			m_verticesVBO.write(m_verticesBuffer);

			m_texCoordsVBO.bind();
			m_texCoordsVBO.write(m_texCoordsBuffer);

			int nbPaths = m_countBuffer.size();
			if (nbTextures < nbPaths) nbTextures = 1;

			m_shaderProgram.bind();
			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			m_shaderProgram.setUniformValue("tex0", 0);

			m_VAO.bind();

			// Optimization when we use only one texture
			if (nbTextures == 1)
			{
				GLuint texture = listTextures.front().getTextureId();

				glBindTexture(GL_TEXTURE_2D, texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				glMultiDrawArrays(GL_TRIANGLE_STRIP, m_firstBuffer.data(), m_countBuffer.data(), m_countBuffer.size());
			}
			else
			{
				for (int i = 0; i < nbPaths; ++i)
				{
					if (!m_countBuffer[i])
						continue;

					GLuint texture = listTextures[i % nbTextures].getTextureId();

					glBindTexture(GL_TEXTURE_2D, texture);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

					glDrawArrays(GL_LINE_STRIP, m_firstBuffer[i], m_countBuffer[i]);
				}
			}

			m_shaderProgram.release();
			m_VAO.release();
		}
	}

protected:
	Data< std::vector<Path> > m_input;
	Data< std::vector<PReal> > m_lineWidth;
	Data< std::vector<ImageWrapper> > m_texture;
	Data< Shader > m_shader;

	std::vector<PReal> m_pathLengths;
	std::vector<types::Point> m_verticesBuffer, m_texCoordsBuffer;
	std::vector<GLint> m_firstBuffer;
	std::vector<GLsizei> m_countBuffer;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_texCoordsVBO;
};

int RenderTexturedPathClass = RegisterObject<RenderTexturedPath>("Render/Textured/Path")
		.setName("Textured path").setDescription("Draw a textured path");

} // namespace panda
