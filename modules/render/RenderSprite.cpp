#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Shader.h>

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>

#include <cmath>

namespace panda {

using types::Color;
using types::ImageWrapper;
using types::Point;
using types::Shader;

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
		, shader(initData(&shader, "shader", "Shaders used during the rendering"))
		, functions(QOpenGLContext::currentContext())
	{
		addInput(&position);
		addInput(&size);
		addInput(&color);
		addInput(&texture);
		addInput(&shader);

		position.getAccessor().push_back(Point(100, 100));
		size.getAccessor().push_back(5.0);
		color.getAccessor().push_back(Color::white());

		posBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		posBuffer.create();

		sizeBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		sizeBuffer.create();

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->addSource(QOpenGLShader::Vertex,
							 "in vec2 position;\n"
							 "in float size;\n"
							 "in vec4 color;\n"
							 "out vec4 f_color;\n"
							 "uniform mat4 MVP;\n"
							 "void main(void)\n"
							 "{\n"
							 "	f_color = color;\n"
							 "	gl_Position = MVP * vec4(position.xy, 0, 1);\n"
							 "	gl_PointSize = max(1.0, size);\n"
							 "}"
							);

		shaderAcc->addSource(QOpenGLShader::Fragment,
							 "uniform sampler2D tex0;\n"
							 "in vec4 f_color;\n"
							 "out vec4 fragColor;\n"
							 "void main(void)\n"
							 "{\n"
							 "   fragColor = texture(tex0, vec2(gl_PointCoord.x, 1-gl_PointCoord.y)) * f_color;\n"
							 "}"
							);
	}

	inline QVector4D colorToVector4(const Color& c)
	{
		return QVector4D(c.r, c.g, c.b, c.a);
	}

	void render()
	{
		const QVector<Point>& listPosition = position.getValue();
		QVector<PReal> listSize = size.getValue();
		const QVector<Color>& listColor = color.getValue();
		GLuint texId = texture.getValue().getTexture();

		int nbPosition = listPosition.size();
		int nbSize = listSize.size();
		int nbColor = listColor.size();

		if(nbPosition && nbSize && nbColor && texId)
		{
			shader.getValue().apply(shaderProgram);

			attribute_pos = shaderProgram.attributeLocation("position");
			attribute_size = shaderProgram.attributeLocation("size");
			attribute_color = shaderProgram.attributeLocation("color");
			uniform_texture = shaderProgram.uniformLocation("tex0");
			uniform_MVP = shaderProgram.uniformLocation("MVP");

			if(nbSize < nbPosition)
				listSize.fill(listSize[0], nbPosition);

			QVector<QVector4D> tmpColors(nbPosition);
			if(nbColor < nbPosition)
				tmpColors.fill(colorToVector4(listColor[0]), nbPosition);
			else
			{
				for(int i = 0; i < nbPosition; ++i)
					tmpColors[i] = colorToVector4(listColor[i]);
			}

			int posBytes = nbPosition * sizeof(PReal) * 2;
			int sizeBytes = nbPosition * sizeof(PReal);
			if(posBuffer.size() < posBytes)
			{
				posBuffer.bind();
				posBuffer.allocate(listPosition.data(), posBytes);
				sizeBuffer.bind();
				sizeBuffer.allocate(listSize.data(), sizeBytes);
			}
			else
			{
				posBuffer.bind();
				posBuffer.write(0, listPosition.data(), posBytes);
				sizeBuffer.bind();
				sizeBuffer.write(0, listSize.data(), sizeBytes);
			}
			QOpenGLBuffer::release(QOpenGLBuffer::VertexBuffer);

			glEnable(GL_POINT_SPRITE);
			glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

			functions.glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texId);
			glEnable(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			shaderProgram.bind();

			posBuffer.bind();
			shaderProgram.enableAttributeArray(attribute_pos);
			shaderProgram.setAttributeArray(attribute_pos, GL_PREAL, nullptr, 2);
			posBuffer.release();

			sizeBuffer.bind();
			shaderProgram.enableAttributeArray(attribute_size);
			shaderProgram.setAttributeArray(attribute_size, GL_PREAL, nullptr, 1);
			sizeBuffer.release();

			shaderProgram.setUniformValue(uniform_texture, 0);
			shaderProgram.setUniformValue(uniform_MVP, getMVPMatrix());

			shaderProgram.enableAttributeArray(attribute_color);
			shaderProgram.setAttributeArray(attribute_color, tmpColors.data());

			glDrawArrays(GL_POINTS, 0, nbPosition);

			shaderProgram.release();

			functions.glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);

			shaderProgram.disableAttributeArray(attribute_pos);
			shaderProgram.disableAttributeArray(attribute_size);
			shaderProgram.disableAttributeArray(attribute_color);
			glDisable(GL_POINT_SPRITE);
			glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
		}
	}

protected:
	Data< QVector<Point> > position;
	Data< QVector<PReal> > size;
	Data< QVector<Color> > color;
	Data< ImageWrapper > texture;
	Data< Shader > shader;

	QOpenGLBuffer posBuffer, sizeBuffer;
	QOpenGLShaderProgram shaderProgram;
	QOpenGLFunctions functions;
	int attribute_pos, attribute_color, attribute_size;
	int uniform_texture, uniform_MVP;
};

int RenderSpriteClass = RegisterObject<RenderSprite>("Render/Sprite").setDescription("Draw a sprite");

} // namespace panda
