#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/Path.h>
#include <panda/types/Shader.h>

#include <QOpenGLShaderProgram>

namespace panda {

using types::Color;
using types::Point;
using types::Path;
using types::Shader;

class RenderLine : public Renderer
{
public:
	PANDA_CLASS(RenderLine, Renderer)

	RenderLine(PandaDocument *parent)
		: Renderer(parent)
		, inputA(initData(&inputA, "point 1", "Start of the line"))
		, inputB(initData(&inputB, "point 2", "End of the line"))
		, width(initData(&width, (PReal)1.0, "width", "Width of the line" ))
		, color(initData(&color, "color", "Color of the line"))
		, shader(initData(&shader, "shader", "Shaders used during the rendering"))
	{
		addInput(&inputA);
		addInput(&inputB);
		addInput(&width);
		addInput(&color);
		addInput(&shader);

		color.getAccessor().push_back(Color::black());

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(QOpenGLShader::Vertex, ":/shaders/PT_attColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(QOpenGLShader::Fragment, ":/shaders/PT_attColor_noTex.f.glsl");
	}

	void render()
	{
		const QVector<Point>& valA = inputA.getValue();
		const QVector<Point>& valB = inputB.getValue();
		const QVector<Color>& listColor = color.getValue();

		int nbA = valA.size(), nbB = valB.size();
		int nbLines = qMin(valA.size(), valB.size());
		bool useTwoLists = true;
		if(nbA && !nbB)
		{
			useTwoLists = false;
			nbLines = nbA / 2;
		}

		int nbColor = listColor.size();

		if(nbLines && nbColor)
		{
			if(!shader.getValue().apply(shaderProgram))
				return;

			if(nbColor < nbLines) nbColor = 1;
			int nbVertices = nbLines * 2;

			shaderProgram.bind();
			shaderProgram.setUniformValue("MVP", getMVPMatrix());

			QVector<Point> tmpVertices;
			int vertexLocation = shaderProgram.attributeLocation("vertex");
			shaderProgram.enableAttributeArray(vertexLocation);
			if(useTwoLists)
			{
				tmpVertices.resize(nbVertices);
				for(int i=0; i<nbLines; ++i)
				{
					tmpVertices[i*2  ] = valA[i];
					tmpVertices[i*2+1] = valB[i];
				}
				shaderProgram.setAttributeArray(vertexLocation, tmpVertices.front().data(), 2);
			}
			else
				shaderProgram.setAttributeArray(vertexLocation, valA.front().data(), 2);

			QVector<Color> tmpColors;
			int colorLocation = shaderProgram.attributeLocation("color");
			shaderProgram.enableAttributeArray(colorLocation);
			if(nbColor >= nbVertices) // We have one color for each vertex
				shaderProgram.setAttributeArray(colorLocation, listColor.front().data(), 4);
			else
			{
				tmpColors.resize(nbVertices);
				for(int i=0; i<nbLines; ++i)
					tmpColors[i*2] = tmpColors[i*2+1] = listColor[i%nbColor];
				shaderProgram.setAttributeArray(colorLocation, tmpColors.front().data(), 4);
			}

			glLineWidth(qMax((PReal)1.0, width.getValue()));
			glDrawArrays(GL_LINES, 0, nbVertices);
			glLineWidth(0);

			shaderProgram.disableAttributeArray(vertexLocation);
			shaderProgram.disableAttributeArray(colorLocation);
			shaderProgram.release();
		}
	}

protected:
	Data< QVector<Point> > inputA, inputB;
	Data<PReal> width;
	Data< QVector<Color> > color;
	Data< Shader > shader;

	QOpenGLShaderProgram shaderProgram;
};

int RenderLineClass = RegisterObject<RenderLine>("Render/Line").setDescription("Draw a line between 2 points");

//****************************************************************************//

class RenderPath : public Renderer
{
public:
	PANDA_CLASS(RenderPath, Renderer)

	RenderPath(PandaDocument *parent)
		: Renderer(parent)
		, input(initData(&input, "path", "Path to be drawn"))
		, lineWidth(initData(&lineWidth, "width", "Width of the line" ))
		, color(initData(&color, "color", "Color of the line"))
		, shader(initData(&shader, "shader", "Shaders used during the rendering"))
	{
		addInput(&input);
		addInput(&lineWidth);
		addInput(&color);
		addInput(&shader);

		color.getAccessor().push_back(Color::black());
		lineWidth.getAccessor().push_back(0.0);

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(QOpenGLShader::Vertex, ":/shaders/PT_uniColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(QOpenGLShader::Fragment, ":/shaders/PT_uniColor_noTex.f.glsl");
	}

	void render()
	{
		const QVector<Path>& paths = input.getValue();
		const QVector<Color>& listColor = color.getValue();
		const QVector<PReal>& listWidth = lineWidth.getValue();

		int nbPaths = paths.size();
		int nbColor = listColor.size();
		int nbWidth = listWidth.size();

		if(nbPaths && nbColor && nbWidth)
		{
			if(!shader.getValue().apply(shaderProgram))
				return;

			if(nbColor < nbPaths) nbColor = 1;
			if(nbWidth < nbPaths) nbWidth = 1;

			shaderProgram.bind();
			shaderProgram.setUniformValue("MVP", getMVPMatrix());

			int vertexLocation = shaderProgram.attributeLocation("vertex");
			shaderProgram.enableAttributeArray(vertexLocation);

			int colorLocation = shaderProgram.uniformLocation("color");


			for(int i=0; i<nbPaths; ++i)
			{
				glLineWidth(listWidth[i % nbWidth]);

				auto color = listColor[i % nbColor];
				shaderProgram.setUniformValue(colorLocation, color.r, color.g, color.b, color.a);

				const Path& path = paths[i];
				shaderProgram.setAttributeArray(vertexLocation, path.front().data(), 2);

				glDrawArrays(GL_LINE_STRIP, 0, path.size());
			}

			glLineWidth(0);
			shaderProgram.disableAttributeArray(vertexLocation);
			shaderProgram.release();
		}
	}

protected:
	Data< QVector<Path> > input;
	Data< QVector<PReal> > lineWidth;
	Data< QVector<Color> > color;
	Data< Shader > shader;

	QOpenGLShaderProgram shaderProgram;
};

int RenderPathClass = RegisterObject<RenderPath>("Render/Path").setDescription("Draw a path");

} // namespace panda
