#include <GL/glew.h>

#include <panda/document/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/types/Rect.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Shader.h>
#include <panda/graphics/Buffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/graphics/VertexArrayObject.h>

namespace panda {

using types::Point;
using types::Rect;
using types::ImageWrapper;
using types::Shader;

class RenderImage : public Renderer
{
public:
	PANDA_CLASS(RenderImage, Renderer)

	RenderImage(PandaDocument* parent)
		: Renderer(parent)
		, m_image(initData("image", "Image to render on screen" ))
		, m_position(initData("position", "Position of the image"))
		, m_rotation(initData("rotation", "Rotation of the image"))
		, m_drawCentered(initData(0, "drawCentered", "If non zero use the center of the image, else use the top-left corner"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_image);
		addInput(m_position);
		addInput(m_rotation);
		addInput(m_drawCentered);
		addInput(m_shader);

		m_drawCentered.setWidget("checkbox");

		m_position.getAccessor().push_back(Point(0, 0));

		m_shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = m_shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/PT_noColor_Tex.f.glsl");

		setUpdateOnMainThread(true); // Because we manipulate images that may have to be converted to textures
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

	void createVertices(Point pos, graphics::Size size, bool centered)
	{
		float w = static_cast<float>(size.width()), h = static_cast<float>(size.height());
		float l, r, t, b;
		if (centered)
		{
			l = pos.x - w / 2.f;
			r = pos.x + w / 2.f;
			t = pos.y - h / 2.f;
			b = pos.y + h / 2.f;
		}
		else
		{
			l = pos.x;
			r = pos.x + w;
			t = pos.y;
			b = pos.y + h;
		}

		m_verticesBuffer.emplace_back(r, t);
		m_verticesBuffer.emplace_back(l, t);
		m_verticesBuffer.emplace_back(r, b);
		m_verticesBuffer.emplace_back(l, b);
	}

	inline Point rotated(Point pt, Point center, float ca, float sa)
	{
		Point trPt = pt - center;
		return center + Point(trPt.x * ca - trPt.y * sa, trPt.x * sa + trPt.y * ca);
	}

	void createVertices(Point pos, graphics::Size size, float angle, bool centered)
	{
		float w = static_cast<float>(size.width()), h = static_cast<float>(size.height());
		Rect rect;
		if (centered)
			rect = Rect(pos.x - w / 2.f, pos.y - h / 2.f, pos.x + w / 2.f, pos.y + h / 2.f);
		else
			rect = Rect(pos, w, h);

		const float PI180 = static_cast<float>(M_PI) / 180;
		float a = angle * PI180;
		float ca = cos(a), sa = sin(a);

		m_verticesBuffer.emplace_back(rotated(rect.topRight(),    pos, ca, sa));
		m_verticesBuffer.emplace_back(rotated(rect.topLeft(),	  pos, ca, sa));
		m_verticesBuffer.emplace_back(rotated(rect.bottomRight(), pos, ca, sa));
		m_verticesBuffer.emplace_back(rotated(rect.bottomLeft(),  pos, ca, sa));
	}

	void update()
	{
		const std::vector<ImageWrapper>& listImage = m_image.getValue();
		const std::vector<Point>& listPosition = m_position.getValue();
		const std::vector<float>& listRotation = m_rotation.getValue();

		m_verticesBuffer.clear();
		m_texCoordsBuffer.clear();
		m_firstBuffer.clear();
		m_countBuffer.clear();

		if (!listImage.empty() && !listPosition.empty())
		{
			int nbImage = listImage.size();
			int nbPosition = listPosition.size();
			int nbRotation = listRotation.size();
			int nb = std::max(nbImage, nbPosition);
			if(nbImage < nb) nbImage = 1;
			if(nbPosition < nb) nbPosition = 1;
			if(nbRotation && nbRotation < nb) nbRotation = 1;

			m_firstBuffer.resize(nb);
			for (int i = 0; i < nb; ++i)
				m_firstBuffer[i] = i * 4;

			m_countBuffer.assign(nb, 4);

			m_texCoordsBuffer.reserve(nb * 4);
			for (int i = 0; i < nb; ++i)
			{
				m_texCoordsBuffer.emplace_back(1.f, 1.f);
				m_texCoordsBuffer.emplace_back(0.f, 1.f);
				m_texCoordsBuffer.emplace_back(1.f, 0.f);
				m_texCoordsBuffer.emplace_back(0.f, 0.f);
			}

			bool centered = (m_drawCentered.getValue() != 0);
			m_verticesBuffer.reserve(nb * 4);
			if (nbRotation)
			{
				for (int i = 0; i < nb; ++i)
					createVertices(listPosition[i], listImage[i % nbImage].size(), listRotation[i % nbRotation], centered);
			}
			else
			{
				for (int i = 0; i < nb; ++i)
					createVertices(listPosition[i % nbPosition], listImage[i % nbImage].size(), centered);
			}
		}
	}

	void render()
	{
		const std::vector<ImageWrapper>& listImage = m_image.getValue();
		const std::vector<Point>& listPosition = m_position.getValue();

		bool centered = (m_drawCentered.getValue() != 0);

		int nbImage = listImage.size();
		int nbPosition = listPosition.size();

		if(nbImage && nbPosition)
		{
			if(nbImage < nbPosition) nbImage = 1;
			if(nbPosition < nbImage) nbPosition = 1;

			if(!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_verticesVBO.bind();
			m_verticesVBO.write(m_verticesBuffer);

			m_texCoordsVBO.bind();
			m_texCoordsVBO.write(m_texCoordsBuffer);
			
			m_VAO.bind();

			m_shaderProgram.bind();
			const graphics::Mat4x4& MVP = getMVPMatrix();
			m_shaderProgram.setUniformValueMat4("MVP", MVP.data());

			m_shaderProgram.setUniformValue("tex0", 0);

			if (nbImage == 1)
			{
				glBindTexture(GL_TEXTURE_2D, listImage[0].getTextureId());
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

				glMultiDrawArrays(GL_TRIANGLE_STRIP, m_firstBuffer.data(), m_countBuffer.data(), m_countBuffer.size());
			}
			else
			{
				for (int i = 0; i < nbImage; ++i)
				{
					glBindTexture(GL_TEXTURE_2D, listImage[i].getTextureId());
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

					glDrawArrays(GL_TRIANGLE_STRIP, m_firstBuffer[i], m_countBuffer[i]);
				}
			}

			m_shaderProgram.release();
			m_VAO.release();
		}
	}

protected:
	Data< std::vector<ImageWrapper> > m_image;
	Data< std::vector<Point> > m_position;
	Data< std::vector<float> > m_rotation;
	Data< int > m_drawCentered;
	Data< Shader > m_shader;

	std::vector<types::Point> m_verticesBuffer, m_texCoordsBuffer;
	std::vector<GLint> m_firstBuffer;
	std::vector<GLsizei> m_countBuffer;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_texCoordsVBO;
};

int RenderImageClass = RegisterObject<RenderImage>("Render/Textured/Image").setDescription("Renders an image at a position");

//****************************************************************************//

class RenderImage_InRect : public Renderer
{
public:
	PANDA_CLASS(RenderImage_InRect, Renderer)

	RenderImage_InRect(PandaDocument* parent)
		: Renderer(parent)
		, m_image(initData("image", "Image to render on screen" ))
		, m_rectangle(initData("rectangle", "Rectangle defining the position and size of the rendered image"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_image);
		addInput(m_rectangle);
		addInput(m_shader);

		m_shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = m_shader.getAccessor();
		shaderAcc->setSourceFromFile(Shader::ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		shaderAcc->setSourceFromFile(Shader::ShaderType::Fragment, "shaders/PT_noColor_Tex.f.glsl");

		setUpdateOnMainThread(true); // Because we manipulate images that may have to be converted to textures
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
		const std::vector<ImageWrapper>& listImage = m_image.getValue();
		const std::vector<Rect>& listRectangle = m_rectangle.getValue();

		m_verticesBuffer.clear();
		m_texCoordsBuffer.clear();
		m_firstBuffer.clear();
		m_countBuffer.clear();

		if (!listImage.empty() && !listRectangle.empty())
		{
			int nbImage = listImage.size();
			int nbRectangle = listRectangle.size();
			if(nbImage < nbRectangle) nbImage = 1;

			m_firstBuffer.resize(nbRectangle);
			for (int i = 0; i < nbRectangle; ++i)
				m_firstBuffer[i] = i * 4;

			m_countBuffer.assign(nbRectangle, 4);

			m_texCoordsBuffer.reserve(nbRectangle * 4);
			for (int i = 0; i < nbRectangle; ++i)
			{
				m_texCoordsBuffer.emplace_back(1.f, 1.f);
				m_texCoordsBuffer.emplace_back(0.f, 1.f);
				m_texCoordsBuffer.emplace_back(1.f, 0.f);
				m_texCoordsBuffer.emplace_back(0.f, 0.f);
			}

			m_verticesBuffer.reserve(nbRectangle * 4);
			for (const auto& rect : listRectangle)
			{
				m_verticesBuffer.emplace_back(rect.right(), rect.top());
				m_verticesBuffer.emplace_back(rect.left(),  rect.top());
				m_verticesBuffer.emplace_back(rect.right(), rect.bottom());
				m_verticesBuffer.emplace_back(rect.left(),  rect.bottom());
			}
		}
	}

	void render()
	{
		const std::vector<ImageWrapper>& listImage = m_image.getValue();
		const std::vector<Rect>& listRectangle = m_rectangle.getValue();

		int nbImage = listImage.size();
		int nbRectangle = listRectangle.size();

		if(nbImage && nbRectangle)
		{
			if(nbImage < nbRectangle) nbImage = 1;

			if(!m_shader.getValue().apply(m_shaderProgram))
				return;

			if (!m_VAO)
				initGL();

			m_verticesVBO.bind();
			m_verticesVBO.write(m_verticesBuffer);

			m_texCoordsVBO.bind();
			m_texCoordsVBO.write(m_texCoordsBuffer);
			
			m_VAO.bind();

			m_shaderProgram.bind();
			const graphics::Mat4x4& MVP = getMVPMatrix();
			m_shaderProgram.setUniformValueMat4("MVP", MVP.data());

			m_shaderProgram.setUniformValue("tex0", 0);

			if (nbImage == 1)
			{
				glBindTexture(GL_TEXTURE_2D, listImage[0].getTextureId());
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

				glMultiDrawArrays(GL_TRIANGLE_STRIP, m_firstBuffer.data(), m_countBuffer.data(), m_countBuffer.size());
			}
			else
			{
				for (int i = 0; i < nbImage; ++i)
				{
					glBindTexture(GL_TEXTURE_2D, listImage[i].getTextureId());
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

					glDrawArrays(GL_TRIANGLE_STRIP, m_firstBuffer[i], m_countBuffer[i]);
				}
			}

			m_shaderProgram.release();
			m_VAO.release();
		}
	}

protected:
	Data< std::vector<ImageWrapper> > m_image;
	Data< std::vector<Rect> > m_rectangle;
	Data< Shader > m_shader;

	std::vector<types::Point> m_verticesBuffer, m_texCoordsBuffer;
	std::vector<GLint> m_firstBuffer;
	std::vector<GLsizei> m_countBuffer;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_texCoordsVBO;
};

int RenderImage_InRectClass = RegisterObject<RenderImage_InRect>("Render/Textured/Image in rectangle")
	.setDescription("Renders an image inside a rectangle");

} // namespace panda
