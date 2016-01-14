#include <GL/glew.h>
#include <panda/helper/gl.h>

#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Shader.h>
#include <panda/graphics/ShaderProgram.h>

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>

#include <array>
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

	RenderSprite(PandaDocument* parent)
		: Renderer(parent)
		, position(initData("position", "Position of the sprite"))
		, size(initData("size", "Size of the sprite" ))
		, color(initData("color", "Color of the sprite"))
		, texture(initData("texture", "Texture of the sprite"))
		, shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(position);
		addInput(size);
		addInput(color);
		addInput(texture);
		addInput(shader);

		position.getAccessor().push_back(Point(100, 100));
		size.getAccessor().push_back(5.0);
		color.getAccessor().push_back(Color::white());

		posBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		posBuffer.create();

		sizeBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
		sizeBuffer.create();

		shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/sprite.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/sprite.f.glsl");
	}

	inline std::array<float, 4> colorToVector4(const Color& c)
	{
		return std::array<float, 4>{c.r, c.g, c.b, c.a};
	}

	void render()
	{
		const std::vector<Point>& listPosition = position.getValue();
		std::vector<PReal> listSize = size.getValue();
		const std::vector<Color>& listColor = color.getValue();
		GLuint texId = texture.getValue().getTextureId();

		int nbPosition = listPosition.size();
		int nbSize = listSize.size();
		int nbColor = listColor.size();

		if(nbPosition && nbSize && nbColor && texId)
		{
			if(!shader.getValue().apply(shaderProgram))
				return;

			attribute_pos = shaderProgram.attributeLocation("position");
			attribute_size = shaderProgram.attributeLocation("size");
			attribute_color = shaderProgram.attributeLocation("color");
			uniform_texture = shaderProgram.uniformLocation("tex0");
			uniform_MVP = shaderProgram.uniformLocation("MVP");

			if(nbSize < nbPosition)
				listSize.resize(nbPosition, listSize[0]);

			std::vector<std::array<float, 4>> tmpColors(nbPosition);
			if(nbColor < nbPosition)
				tmpColors.resize(nbPosition, colorToVector4(listColor[0]));
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

			glBindTexture(GL_TEXTURE_2D, texId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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
			shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			shaderProgram.enableAttributeArray(attribute_color);
			shaderProgram.setAttributeArray(attribute_color, GL_FLOAT, tmpColors.data(), 4);

			glDrawArrays(GL_POINTS, 0, nbPosition);

			shaderProgram.disableAttributeArray(attribute_pos);
			shaderProgram.disableAttributeArray(attribute_size);
			shaderProgram.disableAttributeArray(attribute_color);
			shaderProgram.release();

			glDisable(GL_POINT_SPRITE);
			glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
		}
	}

protected:
	Data< std::vector<Point> > position;
	Data< std::vector<PReal> > size;
	Data< std::vector<Color> > color;
	Data< ImageWrapper > texture;
	Data< Shader > shader;

	QOpenGLBuffer posBuffer, sizeBuffer;
	graphics::ShaderProgram shaderProgram;
	int attribute_pos, attribute_color, attribute_size;
	int uniform_texture, uniform_MVP;
};

int RenderSpriteClass = RegisterObject<RenderSprite>("Render/Textured/Sprite").setDescription("Draw a sprite");

} // namespace panda
