#include <GL/glew.h>

#include <panda/document/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Point.h>
#include <panda/types/Gradient.h>
#include <panda/types/Shader.h>
#include <panda/helper/GradientCache.h>
#include <panda/graphics/Buffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/graphics/VertexArrayObject.h>

namespace panda {

using types::Color;
using types::ImageWrapper;
using types::Point;
using types::Gradient;
using types::Shader;
using helper::GradientCache;

class RenderDisk : public Renderer
{
public:
	PANDA_CLASS(RenderDisk, Renderer)

	RenderDisk(PandaDocument* parent)
		: Renderer(parent)
		, m_center(initData("center", "Center position of the disk"))
		, m_radius(initData("radius", "Radius of the disk"))
		, m_color(initData("color", "Color of the plain disk"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
		, m_approximation(initData(1.f, "approximation", "Maximum distance between the approximation and the perfect circle"))
	{
		addInput(m_center);
		addInput(m_radius);
		addInput(m_color);
		addInput(m_shader);

		m_center.getAccessor().push_back(Point(100, 100));
		m_radius.getAccessor().push_back(5.0);
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

	void update()
	{
		const std::vector<Point>& listCenter = m_center.getValue();
		const std::vector<float>& listRadius = m_radius.getValue();
		const std::vector<Color>& listColor = m_color.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();
		int nbColor = listColor.size();

		m_vertexBuffer.clear();
		m_firstBuffer.clear();
		m_countBuffer.clear();
		m_colorBuffer.clear();

		if(nbCenter && nbRadius && nbColor)
		{
			if(nbRadius < nbCenter) nbRadius = 1;
			if(nbColor < nbCenter) nbColor = 1;

			float maxDist = m_approximation.getValue();
			maxDist = std::max(0.001f, maxDist);

			float PI2 = static_cast<float>(M_PI) * 2;
			for(int i=0; i<nbCenter; ++i)
			{
				float valRadius = listRadius[i % nbRadius];
				
				// Compute an ideal number of segments based on the distance between the approximation and the perfect circle
				int nbSeg = static_cast<int>(PI2 / acosf(1.f - maxDist / valRadius));
				if(nbSeg < 3) continue;

				m_colorBuffer.push_back(listColor[i % nbColor]);

				int nbVertices = m_vertexBuffer.size();
				m_firstBuffer.push_back(nbVertices);
				m_countBuffer.push_back(nbSeg + 2);

				m_vertexBuffer.resize(nbVertices + nbSeg + 2);

				const Point& valCenter = listCenter[i];
				m_vertexBuffer[nbVertices] = valCenter;

				float angle = PI2 / nbSeg;
				float ca = cos(angle), sa = sin(angle);
				Point dir = Point(valRadius, 0);

				for(int i=0; i<=nbSeg; ++i)
				{
					Point pt = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
					m_vertexBuffer[nbVertices + 1 + i] = valCenter + pt;
					dir = pt;
				}
			}
		}
	}

	void render()
	{
		if(m_vertexBuffer.empty())
			return;

		if(!m_shader.getValue().apply(m_shaderProgram))
			return;

		if (!m_VAO)
			initGL();

		m_verticesVBO.bind();
		m_verticesVBO.write(m_vertexBuffer);

		m_VAO.bind();

		m_shaderProgram.bind();
		m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

		int colorLocation = m_shaderProgram.uniformLocation("color");

		int nb = m_countBuffer.size();
		for(int i=0; i<nb; ++i)
		{
			m_shaderProgram.setUniformValueArray(colorLocation, m_colorBuffer[i].data(), 1, 4);

			glDrawArrays(GL_TRIANGLE_FAN, m_firstBuffer[i], m_countBuffer[i]);
		}

		m_shaderProgram.release();
		m_VAO.release();
	}

protected:
	Data< std::vector<Point> > m_center;
	Data< std::vector<float> > m_radius;
	Data< std::vector<Color> > m_color;
	Data< Shader > m_shader;
	Data< float > m_approximation;

	std::vector<Point> m_vertexBuffer;
	std::vector<GLint> m_firstBuffer;
	std::vector<GLsizei> m_countBuffer;
	std::vector<Color> m_colorBuffer;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO;
};

int RenderDiskClass = RegisterObject<RenderDisk>("Render/Filled/Disk").setDescription("Draw a plain disk");

//****************************************************************************//

class RenderDisk_Gradient : public Renderer
{
public:
	PANDA_CLASS(RenderDisk_Gradient, Renderer)

	RenderDisk_Gradient(PandaDocument* parent)
		: Renderer(parent)
		, m_center(initData("center", "Center position of the disk"))
		, m_radius(initData("radius", "Radius of the disk"))
		, m_gradient(initData("gradient", "Gradient used to fill the disk"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
		, m_approximation(initData(1.f, "approximation", "Maximum distance between the approximation and the perfect circle"))
	{
		addInput(m_center);
		addInput(m_radius);
		addInput(m_gradient);
		addInput(m_shader);

		m_center.getAccessor().push_back(Point(100, 100));
		m_radius.getAccessor().push_back(5.0);
		m_gradient.getAccessor().push_back(Gradient());

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

	void update()
	{
		const std::vector<Point>& listCenter = m_center.getValue();
		const std::vector<float>& listRadius = m_radius.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();

		m_vertexBuffer.clear();
		m_texCoordsBuffer.clear();
		m_firstBuffer.clear();
		m_countBuffer.clear();

		if(nbCenter && nbRadius)
		{
			if(nbRadius < nbCenter) nbRadius = 1;

			float maxDist = m_approximation.getValue();
			maxDist = std::max(0.001f, maxDist);

			float PI2 = static_cast<float>(M_PI) * 2;
			for(int i=0; i<nbCenter; ++i)
			{
				float valRadius = listRadius[i % nbRadius];

				// Compute an ideal number of segments based on the distance between the approximation and the perfect circle
				int nbSeg = static_cast<int>(PI2 / acosf(1.f - maxDist / valRadius));
				if(nbSeg < 3)
				{
					m_firstBuffer.push_back(0);
					m_countBuffer.push_back(0);
					continue;
				}

				int nbVertices = m_vertexBuffer.size();
				m_firstBuffer.push_back(nbVertices);
				m_countBuffer.push_back(nbSeg + 2);

				m_vertexBuffer.resize(nbVertices + nbSeg + 2);
				m_texCoordsBuffer.resize(nbVertices + nbSeg + 2);

				const Point& valCenter = listCenter[i];
				m_vertexBuffer[nbVertices] = valCenter;
				m_texCoordsBuffer[nbVertices] = Point(0, 0);

				float angle = PI2 / nbSeg;
				float ca = cos(angle), sa = sin(angle);
				Point dir = Point(valRadius, 0);
				float step = 1.f / nbSeg;
				float texY = 0;

				for(int i=0; i<=nbSeg; ++i)
				{
					Point pt = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
					m_vertexBuffer[nbVertices + 1 + i] = valCenter + pt;
					m_texCoordsBuffer[nbVertices + 1 + i] = Point(1, texY);
					dir = pt;
					texY += step;
				}
			}
		}
	}

	void render()
	{
		const std::vector<float>& listRadius = m_radius.getValue();
		const std::vector<Gradient>& listGradient = m_gradient.getValue();

		int nbRadius = listRadius.size();
		int nbGradient = listGradient.size();
		int nbDisks = m_countBuffer.size();

		if(nbDisks && nbGradient)
		{
			if(nbRadius < nbDisks) nbRadius = 1;
			if(nbGradient < nbDisks) nbGradient = 1;

			if(!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_verticesVBO.bind();
			m_verticesVBO.write(m_vertexBuffer);

			m_texCoordsVBO.bind();
			m_texCoordsVBO.write(m_texCoordsBuffer);

			m_VAO.bind();

			m_shaderProgram.bind();
			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			m_shaderProgram.setUniformValue("tex0", 0);

			// Optimization when we use only one gradient
			if(nbGradient == 1)
			{
				float maxRadius = *std::max_element(listRadius.begin(), listRadius.end());
				GLuint texture = GradientCache::getInstance()->getTexture(listGradient.front(), static_cast<int>(std::ceil(maxRadius)));

				glBindTexture(GL_TEXTURE_2D, texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				for(int i=0; i<nbDisks; ++i)
					glDrawArrays(GL_TRIANGLE_FAN, m_firstBuffer[i], m_countBuffer[i]);
			}
			else
			{

				for(int i=0; i<nbDisks; ++i)
				{
					GLuint texture = GradientCache::getInstance()->getTexture(listGradient[i % nbGradient], static_cast<int>(std::ceil(listRadius[i % nbRadius])));
					if(!texture)
						continue;
					glBindTexture(GL_TEXTURE_2D, texture);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

					glDrawArrays(GL_TRIANGLE_FAN, m_firstBuffer[i], m_countBuffer[i]);
				}
			}

			m_shaderProgram.release();
			m_VAO.release();
		}
	}

protected:
	Data< std::vector<Point> > m_center;
	Data< std::vector<float> > m_radius;
	Data< std::vector<Gradient> > m_gradient;
	Data< Shader > m_shader;
	Data< float > m_approximation;

	std::vector<Point> m_vertexBuffer, m_texCoordsBuffer;
	std::vector<GLint> m_firstBuffer;
	std::vector<GLsizei> m_countBuffer;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_texCoordsVBO;
};

int RenderDisk_GradientClass = RegisterObject<RenderDisk_Gradient>("Render/Gradient/Disk")
		.setName("Gradient disk").setDescription("Draw a disk filled with a radial gradient");

//****************************************************************************//

class RenderDisk_Textured : public Renderer
{
public:
	PANDA_CLASS(RenderDisk_Textured, Renderer)

	RenderDisk_Textured(PandaDocument* parent)
		: Renderer(parent)
		, m_center(initData("center", "Center position of the disk"))
		, m_radius(initData("radius", "Radius of the disk"))
		, m_texture(initData("texture", "Texture applied to the disk"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
		, m_approximation(initData(1.f, "approximation", "Maximum distance between the approximation and the perfect circle"))
	{
		addInput(m_center);
		addInput(m_radius);
		addInput(m_texture);
		addInput(m_shader);

		m_center.getAccessor().push_back(Point(100, 100));
		m_radius.getAccessor().push_back(5.0);

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

	void update()
	{
		const std::vector<Point>& listCenter = m_center.getValue();
		const std::vector<float>& listRadius = m_radius.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();

		m_vertexBuffer.clear();
		m_texCoordsBuffer.clear();
		m_firstBuffer.clear();
		m_countBuffer.clear();

		if(nbCenter && nbRadius)
		{
			if(nbRadius < nbCenter) nbRadius = 1;

			float maxDist = m_approximation.getValue();
			maxDist = std::max(0.001f, maxDist);

			float PI2 = static_cast<float>(M_PI) * 2;
			for(int i=0; i<nbCenter; ++i)
			{
				float valRadius = listRadius[i % nbRadius];

				// Compute an ideal number of segments based on the distance between the approximation and the perfect circle
				int nbSeg = static_cast<int>(PI2 / acosf(1.f - maxDist / valRadius));
				if(nbSeg < 3)
				{
					m_firstBuffer.push_back(0);
					m_countBuffer.push_back(0);
					continue;
				}

				int nbVertices = m_vertexBuffer.size();
				m_firstBuffer.push_back(nbVertices);
				m_countBuffer.push_back(nbSeg + 2);

				m_vertexBuffer.resize(nbVertices + nbSeg + 2);
				m_texCoordsBuffer.resize(nbVertices + nbSeg + 2);

				const Point& valCenter = listCenter[i];
				m_vertexBuffer[nbVertices] = valCenter;
				m_texCoordsBuffer[nbVertices] = Point(0.5, 0.5);

				float angle = PI2 / nbSeg;
				float ca = cos(angle), sa = sin(angle);
				Point dir = Point(1, 0);
				Point texCenter = Point(0.5, 0.5);
				Point texScale = Point(0.5, -0.5);

				for(int i=0; i<=nbSeg; ++i)
				{
					Point pt = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
					m_vertexBuffer[nbVertices + 1 + i] = valCenter + pt * valRadius;
					m_texCoordsBuffer[nbVertices + 1 + i] = texCenter + pt.linearProduct(texScale);
					dir = pt;
				}
			}
		}
	}

	void render()
	{
		const std::vector<float>& listRadius = m_radius.getValue();
		const int texId = m_texture.getValue().getTextureId();

		int nbRadius = listRadius.size();
		int nbDisks = m_countBuffer.size();

		if(nbDisks && texId)
		{
			if(nbRadius < nbDisks) nbRadius = 1;

			if(!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_verticesVBO.bind();
			m_verticesVBO.write(m_vertexBuffer);

			m_texCoordsVBO.bind();
			m_texCoordsVBO.write(m_texCoordsBuffer);

			m_VAO.bind();

			m_shaderProgram.bind();
			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			glBindTexture(GL_TEXTURE_2D, texId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			m_shaderProgram.setUniformValue("tex0", 0);

			glMultiDrawArrays(GL_TRIANGLE_FAN, m_firstBuffer.data(), m_countBuffer.data(), m_countBuffer.size());

			m_shaderProgram.release();
			m_VAO.release();
		}
	}

protected:
	Data< std::vector<Point> > m_center;
	Data< std::vector<float> > m_radius;
	Data< ImageWrapper > m_texture;
	Data< Shader > m_shader;
	Data< float > m_approximation;

	std::vector<Point> m_vertexBuffer, m_texCoordsBuffer;
	std::vector<GLint> m_firstBuffer;
	std::vector<GLsizei> m_countBuffer;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_texCoordsVBO;
};

int RenderDisk_TexturedClass = RegisterObject<RenderDisk_Textured>("Render/Textured/Disk")
		.setName("Textured disk").setDescription("Draw a textured disk");

} // namespace panda
