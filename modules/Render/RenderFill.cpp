#include <GL/glew.h>

#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Gradient.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>
#include <panda/helper/GradientCache.h>
#include <panda/graphics/Buffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/graphics/VertexArrayObject.h>

namespace panda {

using types::ImageWrapper;
using types::Gradient;
using types::Point;
using types::Rect;
using types::Shader;

class BaseRenderFill : public Renderer
{
public:
	PANDA_CLASS(BaseRenderFill, Renderer)

	BaseRenderFill(PandaDocument* parent)
		: Renderer(parent)
		, m_area(initData("area", "Area to paint the gradient in. If null, the render area is used instead"))
		, m_shader(initData("shader", "Shaders used during the rendering"))
	{
		addInput(m_area);
		addInput(m_shader);

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

	Rect getArea()
	{
		Rect area = m_area.getValue();
		if(area.empty())
		{
			auto size = getLayerSize();
			area = Rect(0, 0, static_cast<float>(size.width()), static_cast<float>(size.height()));
		}

		return area;
	}

	void renderFill(GLuint texture, std::vector<Point> texCoords = {})
	{
		if(!texture)
			return;

		Rect area = m_area.getValue();
		if(area.empty())
		{
			auto size = getLayerSize();
			area = Rect(0, 0, static_cast<float>(size.width()), static_cast<float>(size.height()));
		}

		if(!m_shader.getValue().apply(m_shaderProgram))
			return;

		if (!m_VAO)
			initGL();

		std::vector<Point> pts = { area.topRight(), area.topLeft(), area.bottomRight(), area.bottomLeft() };
		m_verticesVBO.bind();
		m_verticesVBO.write(pts);

		// Textures coordinates are inversed vertically
		if(texCoords.empty())
			texCoords = { {1, 1}, {0, 1}, {1, 0}, {0, 0} };
		m_texCoordsVBO.bind();
		m_texCoordsVBO.write(texCoords);

		m_shaderProgram.bind();
		m_shaderProgram.setUniformValueMat4("MVP", getMVPMatrix().data());

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		m_shaderProgram.setUniformValue("tex0", 0);

		m_VAO.bind();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		m_VAO.release();

		m_shaderProgram.release();
	}

protected:
	Data<Rect> m_area;
	Data<Shader> m_shader;

	graphics::ShaderProgram m_shaderProgram;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_texCoordsVBO;
};

//****************************************************************************//

class RenderGradient_Horizontal : public BaseRenderFill
{
public:
	PANDA_CLASS(RenderGradient_Horizontal, BaseRenderFill)

	RenderGradient_Horizontal(PandaDocument* parent)
		: BaseRenderFill(parent)
		, m_gradient(initData("gradient", "Gradient to paint on the screen"))
	{
		addInput(m_gradient);
		addData(&m_gradient, 0);
	}

	void render()
	{
		const Gradient& grad = m_gradient.getValue();
		int width = static_cast<int>(std::ceil(getArea().width()));
		GLuint texture = helper::GradientCache::getInstance()->getTexture(grad, width);

		renderFill(texture);
	}

protected:
	Data<Gradient> m_gradient;
};

int RenderGradient_HorizontalClass = RegisterObject<RenderGradient_Horizontal>("Render/Gradient/Horizontal")
		.setName("Horizontal gradient").setDescription("Draw a horizontal gradient in an area");

//****************************************************************************//

class RenderGradient_Vertical : public BaseRenderFill
{
public:
	PANDA_CLASS(RenderGradient_Vertical, BaseRenderFill)

	RenderGradient_Vertical(PandaDocument* parent)
		: BaseRenderFill(parent)
		, m_gradient(initData("gradient", "Gradient to paint on the screen"))
	{
		addInput(m_gradient);
		addData(&m_gradient, 0);
	}

	void render()
	{
		const Gradient& grad = m_gradient.getValue();
		int height = static_cast<int>(std::ceil(getArea().height()));
		GLuint texture = helper::GradientCache::getInstance()->getTexture(grad, height);

		renderFill(texture, { {0, 1}, {0, 0}, {1, 1}, {1, 0} });
	}

protected:
	Data<Gradient> m_gradient;
};

int RenderGradient_VerticalClass = RegisterObject<RenderGradient_Vertical>("Render/Gradient/Vertical")
		.setName("Vertical gradient").setDescription("Draw a vertical gradient in an area");

//****************************************************************************//

class RenderFill_Motif : public BaseRenderFill
{
public:
	PANDA_CLASS(RenderFill_Motif, BaseRenderFill)

	RenderFill_Motif(PandaDocument* parent)
		: BaseRenderFill(parent)
		, m_texture(initData("texture", "Motif to paint on the screen"))
	{
		addInput(m_texture);
		addData(&m_texture, 0);
	}

	void render()
	{
		Rect area = getArea();

		const auto& texValue = m_texture.getValue();
		GLuint texId = texValue.getTextureId();
		if(!texId)
			return;

		auto texSize = texValue.size();
		float w = area.width() / texSize.width(), h = area.height() / texSize.height();
		renderFill(texId, { {w, 0}, {0, 0}, {w, -h}, {0, -h} });
	}

protected:
	Data<ImageWrapper> m_texture;
};

int RenderFill_MotifClass = RegisterObject<RenderFill_Motif>("Render/Textured/Fill with motif")
		.setDescription("Fill an area with a motif");


} // namespace panda
