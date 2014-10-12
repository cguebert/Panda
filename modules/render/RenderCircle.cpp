#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/Point.h>
#include <panda/types/Shader.h>

#include <QOpenGLShaderProgram>
#include <cmath>

namespace panda {

using types::Color;
using types::Point;
using types::Shader;

class RenderCircle : public Renderer
{
public:
	PANDA_CLASS(RenderCircle, Renderer)

	RenderCircle(PandaDocument* parent)
		: Renderer(parent)
		, center(initData(&center, "center", "Center position of the circle"))
		, radius(initData(&radius, "radius", "Radius of the circle" ))
		, lineWidth(initData(&lineWidth, "lineWidth", "Width of the line"))
		, color(initData(&color, "color", "Color of the circle"))
		, shader(initData(&shader, "shader", "Shaders used during the rendering"))
	{
		addInput(&center);
		addInput(&radius);
		addInput(&lineWidth);
		addInput(&color);
		addInput(&shader);

		center.getAccessor().push_back(Point(100, 100));
		radius.getAccessor().push_back(5.0);
		lineWidth.getAccessor().push_back(0.0);
		color.getAccessor().push_back(Color::black());

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(QOpenGLShader::Vertex, ":/share/shaders/PT_uniColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(QOpenGLShader::Fragment, ":/share/shaders/PT_uniColor_noTex.f.glsl");
	}

	void update()
	{
		const QVector<Point>& listCenter = center.getValue();
		const QVector<PReal>& listRadius = radius.getValue();
		const QVector<PReal>& listWidth = lineWidth.getValue();
		const QVector<Color>& listColor = color.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();
		int nbWidth = listWidth.size();
		int nbColor = listColor.size();

		vertexBuffer.clear();
		widthBuffer.clear();
		colorBuffer.clear();
		firstBuffer.clear();
		countBuffer.clear();

		if(nbCenter && nbRadius && nbColor && nbWidth)
		{
			if(nbRadius < nbCenter) nbRadius = 1;
			if(nbColor < nbCenter) nbColor = 1;
			if(nbWidth < nbCenter) nbWidth = 1;

			PReal PI2 = static_cast<PReal>(M_PI) * 2;
			for(int i=0; i<nbCenter; ++i)
			{
				PReal valRadius = listRadius[i % nbRadius];
				int nbSeg = static_cast<int>(floor(valRadius * PI2));
				if(nbSeg < 3) continue;

				colorBuffer.push_back(listColor[i % nbColor]);
				widthBuffer.push_back(listWidth[i % nbWidth]);

				int nbVertices = vertexBuffer.size();
				firstBuffer.push_back(nbVertices);
				countBuffer.push_back(nbSeg);

				vertexBuffer.resize(nbVertices + nbSeg);

				const Point& valCenter = listCenter[i];
				PReal angle = PI2 / nbSeg;
				PReal ca = cos(angle), sa = sin(angle);
				Point dir = Point(valRadius, 0);

				for(int i=0; i<=nbSeg; ++i)
				{
					Point pt = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
					vertexBuffer[nbVertices + i] = valCenter + pt;
					dir = pt;
				}
			}
		}
	}

	void render()
	{

		if(vertexBuffer.empty())
			return;

		if(!shader.getValue().apply(shaderProgram))
			return;

		glEnable(GL_LINE_SMOOTH);
		shaderProgram.bind();
		shaderProgram.setUniformValue("MVP", getMVPMatrix());

		shaderProgram.enableAttributeArray("vertex");
		shaderProgram.setAttributeArray("vertex", vertexBuffer.front().data(), 2);

		int colorLocation = shaderProgram.uniformLocation("color");

		int nb = countBuffer.size();
		for(int i=0; i<nb; ++i)
		{
			glLineWidth(widthBuffer[i]);

			auto color = colorBuffer[i];
			shaderProgram.setUniformValue(colorLocation, color.r, color.g, color.b, color.a);

			glDrawArrays(GL_LINE_LOOP, firstBuffer[i], countBuffer[i]);
		}

		shaderProgram.disableAttributeArray("vertex");
		shaderProgram.release();
		glDisable(GL_LINE_SMOOTH);
	}

protected:
	Data< QVector<Point> > center;
	Data< QVector<PReal> > radius, lineWidth;
	Data< QVector<Color> > color;
	Data< Shader > shader;

	QVector<Point> vertexBuffer;
	QVector<PReal> widthBuffer;
	QVector<Color> colorBuffer;
	QVector<GLint> firstBuffer;
	QVector<GLsizei> countBuffer;

	QOpenGLShaderProgram shaderProgram;
};

int RenderCircleClass = RegisterObject<RenderCircle>("Render/Line/Circle").setDescription("Draw a circle");

} // namespace panda
