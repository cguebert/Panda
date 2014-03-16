#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/ImageWrapper.h>

#include <QPointF>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>

#define _USE_MATH_DEFINES
#include <math.h>

namespace panda {

using types::ImageWrapper;

class RenderSprite : public Renderer
{
public:
	PANDA_CLASS(RenderSprite, Renderer)

	RenderSprite(PandaDocument *parent)
		: Renderer(parent)
		, position(initData(&position, "position", "Position of the sprite"))
		, size(initData(&size, "size", "Size of the sprite" ))
		, color(initData(&color, "color", "Color of the sprite"))
		, texture(initData(&texture, "texture", "Texture of the sprite"))
		, posBuffer(QOpenGLBuffer::VertexBuffer)
		, functions(QOpenGLContext::currentContext())
	{
		addInput(&position);
		addInput(&size);
		addInput(&color);
		addInput(&texture);

		position.getAccessor().push_back(QPointF(100, 100));
		size.getAccessor().push_back(5.0);
		color.getAccessor().push_back(QColor(255, 255, 255));

		posBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		posBuffer.create();

		shader.addShaderFromSourceCode(QOpenGLShader::Vertex,
										"attribute vec2 position;"
										"uniform float size;"
										"uniform mat4 MVP;"
										"void main(void){"
										"	gl_Position = MVP * vec4(position.x, position.y, 0, 1);"
										"	gl_PointSize = max(1.0, size);"
										"}"
									   );
		shader.addShaderFromSourceCode(QOpenGLShader::Fragment,
										"uniform sampler2D texture;"
										"void main(void){"
										"   gl_FragColor = texture2D(texture, gl_PointCoord);"	// vec4(gl_PointCoord.st, 0, 1);
										"}"
									   );

		shader.link();
		attribute_pos = shader.attributeLocation("position");
		uniform_texture = shader.uniformLocation("texture");
		uniform_size = shader.uniformLocation("size");
		uniform_MVP = shader.uniformLocation("MVP");
	}

	void render()
	{
		const QVector<QPointF>& listPosition = position.getValue();
		const QVector<double>& listSize = size.getValue();
		const QVector<QColor>& listColor = color.getValue();
		GLuint texId = texture.getValue().getTexture();

		int nbPosition = listPosition.size();
		int nbSize = listSize.size();
		int nbColor = listColor.size();

		if(nbPosition && nbSize && nbColor && texId)
		{
			if(nbSize < nbPosition) nbSize = 1;
			if(nbColor < nbPosition) nbColor = 1;

			posBuffer.bind();
			int nbBytes = nbPosition * sizeof(qreal) * 2;
			if(posBuffer.size() < nbBytes)
				posBuffer.allocate(listPosition.data(), nbBytes);
			else
				posBuffer.write(0, listPosition.data(), nbBytes);

			glColor3f(1, 1, 1);

//			glEnable(GL_BLEND);
//			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_POINT_SPRITE);
			glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

			functions.glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texId);
			glEnable(GL_TEXTURE_2D);

			shader.bind();

			posBuffer.bind();
			shader.enableAttributeArray(attribute_pos);
			shader.setAttributeArray(attribute_pos, GL_DOUBLE, 0, 2);

			shader.setUniformValue(uniform_size, static_cast<float>(listSize[0]));
			shader.setUniformValue(uniform_texture, 0);
			shader.setUniformValue(uniform_MVP, getMVPMatrix());

			glDrawArrays(GL_POINTS, 0, nbPosition);

			shader.release();
			posBuffer.release();

			functions.glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);

			functions.glDisableVertexAttribArray(attribute_pos);
			glDisable(GL_POINT_SPRITE);
			glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
		}
	}

protected:
	Data< QVector<QPointF> > position;
	Data< QVector<double> > size;
	Data< QVector<QColor> > color;
	Data< ImageWrapper > texture;

	QOpenGLBuffer posBuffer;
	QOpenGLShaderProgram shader;
	QOpenGLFunctions functions;
	int attribute_pos, attribute_color, attribute_size;
	int uniform_texture, uniform_size, uniform_MVP;
};

int RenderSpriteClass = RegisterObject<RenderSprite>("Render/Sprite").setDescription("Draw a sprite");

} // namespace panda
