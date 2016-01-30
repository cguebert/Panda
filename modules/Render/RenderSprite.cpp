#include <GL/glew.h>

#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Shader.h>
#include <panda/graphics/Buffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/graphics/VertexArrayObject.h>

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
		, m_position(initData("position", "Position of the sprite"))
		, m_size(initData("size", "Size of the sprite" ))
		, m_color(initData("color", "Color of the sprite"))
		, m_texture(initData("texture", "Texture of the sprite"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_position);
		addInput(m_size);
		addInput(m_color);
		addInput(m_texture);
		addInput(m_shader);

		m_position.getAccessor().push_back(Point(100, 100));
		m_size.getAccessor().push_back(5.0);
		m_color.getAccessor().push_back(Color::white());

		m_shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = m_shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/sprite.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/sprite.f.glsl");
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

		m_sizeVBO.create();
		m_sizeVBO.bind();
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(2);

		m_VAO.release();
	}

	void render()
	{
		const std::vector<Point>& listPosition = m_position.getValue();
		const std::vector<PReal>& listSize = m_size.getValue();
		const std::vector<Color>& listColor = m_color.getValue();
		GLuint texId = m_texture.getValue().getTextureId();

		int nbPosition = listPosition.size();
		int nbSize = listSize.size();
		int nbColor = listColor.size();

		if(nbPosition && nbSize && nbColor && texId)
		{
			if(!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_verticesVBO.bind();
			m_verticesVBO.write(listPosition);

			m_sizeVBO.bind();
			if (nbSize < nbPosition)
			{
				 std::vector<PReal> tmpSize(nbPosition, listSize[0]);
				 m_sizeVBO.write(tmpSize);
			}
			else
				m_sizeVBO.write(listSize);

			m_colorsVBO.bind();
			if (nbColor < nbPosition)
			{
				std::vector<types::Color> tmpColors(nbPosition, listColor[0]);
				m_colorsVBO.write(tmpColors);
			}
			else
				m_colorsVBO.write(listColor);

			m_VAO.bind();

			glEnable(GL_POINT_SPRITE);
			glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

			m_shaderProgram.bind();
			m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

			glBindTexture(GL_TEXTURE_2D, texId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			m_shaderProgram.setUniformValue("tex0", 0);
			
			glDrawArrays(GL_POINTS, 0, nbPosition);

			m_shaderProgram.release();
			m_VAO.release();

			glDisable(GL_POINT_SPRITE);
			glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
		}
	}

protected:
	Data< std::vector<Point> > m_position;
	Data< std::vector<PReal> > m_size;
	Data< std::vector<Color> > m_color;
	Data< ImageWrapper > m_texture;
	Data< Shader > m_shader;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_colorsVBO, m_sizeVBO;
};

int RenderSpriteClass = RegisterObject<RenderSprite>("Render/Textured/Sprite").setDescription("Draw a sprite");

} // namespace panda
