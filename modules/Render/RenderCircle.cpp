#include <GL/glew.h>


#include <panda/document/RenderedDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/Point.h>
#include <panda/types/Shader.h>
#include <panda/graphics/Buffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/graphics/VertexArrayObject.h>
#include <panda/helper/algorithm.h>

namespace panda {

using types::Color;
using types::Point;
using types::Shader;

class RenderCircle : public Renderer
{
public:
	PANDA_CLASS(RenderCircle, Renderer)

	RenderCircle(RenderedDocument* parent)
		: Renderer(parent)
		, m_center(initData("center", "Center position of the circle"))
		, m_radius(initData("radius", "Radius of the circle" ))
		, m_lineWidth(initData("lineWidth", "Width of the line"))
		, m_color(initData("color", "Color of the circle"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
		, m_approximation(initData(1.f, "approximation", "Maximum distance between the approximation and the perfect circle"))
	{
		addInput(m_center);
		addInput(m_radius);
		addInput(m_lineWidth);
		addInput(m_color);
		addInput(m_shader);

		m_center.getAccessor().push_back(Point(100, 100));
		m_radius.getAccessor().push_back(5.0);
		m_lineWidth.getAccessor().push_back(1.0);
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
		const std::vector<float>& listWidth = m_lineWidth.getValue();
		const std::vector<Color>& listColor = m_color.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();
		int nbWidth = listWidth.size();
		int nbColor = listColor.size();

		m_vertexBuffer.clear();
		m_colorBuffer.clear();
		m_firstBuffer.clear();
		m_countBuffer.clear();

		if(nbCenter && nbRadius && nbColor && nbWidth)
		{
			if(nbRadius < nbCenter) nbRadius = 1;
			if(nbColor < nbCenter) nbColor = 1;
			if(nbWidth < nbCenter) nbWidth = 1;

			float maxDist = m_approximation.getValue();
			maxDist = std::max(0.001f, maxDist);

			float PI2 = static_cast<float>(M_PI) * 2;
			for(int i = 0; i < nbCenter; ++i)
			{
				float valRadius = listRadius[i % nbRadius];
				float maxWidth = valRadius - 0.5f;
				float width = helper::bound(0.5f, listWidth[i % nbWidth], maxWidth);

				// Compute an ideal number of segments based on the distance between the approximation and the perfect circle
				int nbSeg = static_cast<int>(PI2 / acosf(1.f - maxDist / (valRadius + width)));
				if(nbSeg < 3) continue;

				m_colorBuffer.push_back(listColor[i % nbColor]);

				int nbVertices = m_vertexBuffer.size();
				int count = 2 * (nbSeg + 1);
				m_firstBuffer.push_back(nbVertices);
				m_countBuffer.push_back(count);

				m_vertexBuffer.resize(nbVertices + count);

				const Point& valCenter = listCenter[i];
				float angle = PI2 / nbSeg;
				float ca = cos(angle), sa = sin(angle);
				Point inDir = Point(valRadius + width, 0), outDir = Point(valRadius - width, 0);

				for(int i=0; i<=nbSeg; ++i)
				{
					outDir = Point(outDir.x * ca + outDir.y * sa, outDir.y * ca - outDir.x * sa);
					m_vertexBuffer[nbVertices + i * 2] = valCenter + outDir;

					inDir = Point(inDir.x * ca + inDir.y * sa, inDir.y * ca - inDir.x * sa);
					m_vertexBuffer[nbVertices + i * 2 + 1] = valCenter + inDir;
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

		m_shaderProgram.bind();
		m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

		m_verticesVBO.bind();
		m_verticesVBO.write(m_vertexBuffer);

		m_VAO.bind();

		int colorLocation = m_shaderProgram.uniformLocation("color");

		int nb = m_countBuffer.size();
		for(int i=0; i<nb; ++i)
		{
			m_shaderProgram.setUniformValueArray(colorLocation, m_colorBuffer[i].data(), 1, 4);

			glDrawArrays(GL_TRIANGLE_STRIP, m_firstBuffer[i], m_countBuffer[i]);
		}

		m_shaderProgram.release();
		m_VAO.release();
	}

protected:
	Data< std::vector<Point> > m_center;
	Data< std::vector<float> > m_radius, m_lineWidth;
	Data< std::vector<Color> > m_color;
	Data< Shader > m_shader;
	Data< float > m_approximation;

	std::vector<Point> m_vertexBuffer;
	std::vector<Color> m_colorBuffer;
	std::vector<GLint> m_firstBuffer;
	std::vector<GLsizei> m_countBuffer;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO;
};

int RenderCircleClass = RegisterObject<RenderCircle, RenderedDocument>("Render/Line/Circle").setDescription("Draw a circle");

} // namespace panda
