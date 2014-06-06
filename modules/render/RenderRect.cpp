#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>

#include <QOpenGLShaderProgram>

namespace panda {

using types::Color;
using types::Rect;
using types::Shader;

class RenderRect : public Renderer
{
public:
	PANDA_CLASS(RenderRect, Renderer)

	RenderRect(PandaDocument *parent)
		: Renderer(parent)
		, rect(initData(&rect, "rectangle", "Position and size of the rectangle"))
		, lineWidth(initData(&lineWidth, "lineWidth", "Width of the line"))
		, color(initData(&color, "color", "Color of the rectangle"))
		, shader(initData(&shader, "shader", "Shaders used during the rendering"))
	{
		addInput(&rect);
		addInput(&lineWidth);
		addInput(&color);
		addInput(&shader);

		rect.getAccessor().push_back(Rect(100, 100, 150, 150));
		color.getAccessor().push_back(Color::black());
		lineWidth.getAccessor().push_back(0.0);

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(QOpenGLShader::Vertex, ":/shaders/PT_uniColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(QOpenGLShader::Fragment, ":/shaders/PT_uniColor_noTex.f.glsl");
	}

	void render()
	{
		const QVector<Rect>& listRect = rect.getValue();
		const QVector<Color>& listColor = color.getValue();
		const QVector<PReal>& listWidth = lineWidth.getValue();

		int nbRect = listRect.size();
		int nbColor = listColor.size();
		int nbWidth = listWidth.size();

		if(nbRect && nbColor || nbWidth)
		{
			if(!shader.getValue().apply(shaderProgram))
				return;

			if(nbColor < nbRect) nbColor = 1;
			if(nbWidth < nbRect) nbWidth = 1;
			PReal verts[8];

			shaderProgram.bind();
			shaderProgram.setUniformValue("MVP", getMVPMatrix());

			shaderProgram.enableAttributeArray("vertex");
			shaderProgram.setAttributeArray("vertex", verts, 2);

			int colorLocation = shaderProgram.uniformLocation("color");

			for(int i=0; i<nbRect; ++i)
			{
				auto color = listColor[i % nbColor];
				shaderProgram.setUniformValue(colorLocation, color.r, color.g, color.b, color.a);

				glLineWidth(listWidth[i % nbWidth]);

				Rect rect = listRect[i % nbRect];
				verts[0*2+0] = rect.right(); verts[0*2+1] = rect.top();
				verts[1*2+0] = rect.left(); verts[1*2+1] = rect.top();
				verts[2*2+0] = rect.left(); verts[2*2+1] = rect.bottom();
				verts[3*2+0] = rect.right(); verts[3*2+1] = rect.bottom();

				glDrawArrays(GL_LINE_LOOP, 0, 4);
			}

			shaderProgram.disableAttributeArray("vertex");
			shaderProgram.release();
		}
	}

protected:
	Data< QVector<Rect> > rect;
	Data< QVector<PReal> > lineWidth;
	Data< QVector<Color> > color;
	Data< Shader > shader;

	QOpenGLShaderProgram shaderProgram;
};

int RenderRectClass = RegisterObject<RenderRect>("Render/Rectangle").setDescription("Draw a rectangle");

//****************************************************************************//

class RenderFilledRect : public Renderer
{
public:
	PANDA_CLASS(RenderFilledRect, Renderer)

	RenderFilledRect(PandaDocument *parent)
		: Renderer(parent)
		, rect(initData(&rect, "rectangle", "Position and size of the rectangle"))
		, color(initData(&color, "color", "Color of the rectangle"))
		, shader(initData(&shader, "shader", "Shaders used during the rendering"))
	{
		addInput(&rect);
		addInput(&color);
		addInput(&shader);

		rect.getAccessor().push_back(Rect(100, 100, 150, 150));
		color.getAccessor().push_back(Color::black());

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(QOpenGLShader::Vertex, ":/shaders/PT_uniColor_noTex.v.glsl");
		shaderAcc->setSourceFromFile(QOpenGLShader::Fragment, ":/shaders/PT_uniColor_noTex.f.glsl");

		m_texCoords[0*2+0] = 1; m_texCoords[0*2+1] = 1;
		m_texCoords[1*2+0] = 0; m_texCoords[1*2+1] = 1;
		m_texCoords[3*2+0] = 0; m_texCoords[3*2+1] = 0;
		m_texCoords[2*2+0] = 1; m_texCoords[2*2+1] = 0;
	}

	void render()
	{
		const QVector<Rect>& listRect = rect.getValue();
		const QVector<Color>& listColor = color.getValue();

		int nbRect = listRect.size();
		int nbColor = listColor.size();

		if(nbRect && nbColor)
		{
			if(!shader.getValue().apply(shaderProgram))
				return;

			if(nbColor < nbRect) nbColor = 1;
			PReal verts[8];

			shaderProgram.setUniformValue("MVP", getMVPMatrix());

			shaderProgram.enableAttributeArray("vertex");
			shaderProgram.setAttributeArray("vertex", verts, 2);

			int colorLocation = shaderProgram.uniformLocation("color");

			int texCoordLocation = shaderProgram.attributeLocation("texCoord");
			if(texCoordLocation != -1)
			{
				shaderProgram.enableAttributeArray(texCoordLocation);
				shaderProgram.setAttributeArray(texCoordLocation, m_texCoords, 2);
			}

			for(int i=0; i<nbRect; ++i)
			{
				auto color = listColor[i % nbColor];
				shaderProgram.setUniformValue(colorLocation, color.r, color.g, color.b, color.a);

				Rect rect = listRect[i % nbRect];
				verts[0*2+0] = rect.right(); verts[0*2+1] = rect.top();
				verts[1*2+0] = rect.left(); verts[1*2+1] = rect.top();
				verts[2*2+0] = rect.right(); verts[2*2+1] = rect.bottom();
				verts[3*2+0] = rect.left(); verts[3*2+1] = rect.bottom();

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}

			shaderProgram.disableAttributeArray("vertex");
			if(texCoordLocation != -1)
				shaderProgram.disableAttributeArray(texCoordLocation);
			shaderProgram.release();
		}
	}

protected:
	Data< QVector<Rect> > rect;
	Data< QVector<Color> > color;
	Data< Shader > shader;

	GLfloat m_texCoords[8];

	QOpenGLShaderProgram shaderProgram;
};

int RenderFilledRectClass = RegisterObject<RenderFilledRect>("Render/Filled rectangle").setDescription("Draw a filled rectangle");


} // namespace panda
