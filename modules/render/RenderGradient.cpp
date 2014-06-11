#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Gradient.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>
#include <panda/helper/GradientCache.h>

#include <QOpenGLShaderProgram>

namespace panda {

using types::Gradient;
using types::Rect;
using types::Shader;

class RenderGradient_Horizontal : public Renderer
{
public:
	PANDA_CLASS(RenderGradient_Horizontal, Renderer)

	RenderGradient_Horizontal(PandaDocument *parent)
		: Renderer(parent)
		, m_gradient(initData(&m_gradient, "gradient", "Gradient to paint on the screen"))
		, m_area(initData(&m_area, "area", "Area to paint the gradient in. If null, the render area is used instead"))
		, m_shader(initData(&m_shader, "shader", "Shaders used during the rendering"))
	{
		addInput(&m_gradient);
		addInput(&m_area);
		addInput(&m_shader);

		m_shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = m_shader.getAccessor();
		shaderAcc->setSourceFromFile(QOpenGLShader::Vertex, ":/shaders/PT_noColor_Tex.v.glsl");
		shaderAcc->setSourceFromFile(QOpenGLShader::Fragment, ":/shaders/PT_noColor_Tex.f.glsl");

		m_texCoords[0*2+0] = 1; m_texCoords[0*2+1] = 0;
		m_texCoords[1*2+0] = 0; m_texCoords[1*2+1] = 0;
		m_texCoords[3*2+0] = 0; m_texCoords[3*2+1] = 0;
		m_texCoords[2*2+0] = 1; m_texCoords[2*2+1] = 0;
	}

	void render()
	{
		const Gradient& grad = m_gradient.getValue();
		Rect area = m_area.getValue();
		if(area.empty())
		{
			QSize size = getLayerSize();
			area = Rect(0, 0, size.width(), size.height());
		}

		GLuint texture = helper::GradientCache::getInstance()->getTexture(grad, area.width());
		if(texture == -1)
			return;

		if(!m_shader.getValue().apply(m_shaderProgram))
			return;

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		m_shaderProgram.bind();
		m_shaderProgram.setUniformValue("MVP", getMVPMatrix());

		PReal verts[8];
		verts[0*2+0] = area.right(); verts[0*2+1] = area.top();
		verts[1*2+0] = area.left(); verts[1*2+1] = area.top();
		verts[2*2+0] = area.right(); verts[2*2+1] = area.bottom();
		verts[3*2+0] = area.left(); verts[3*2+1] = area.bottom();

		m_shaderProgram.enableAttributeArray("vertex");
		m_shaderProgram.setAttributeArray("vertex", verts, 2);

		m_shaderProgram.enableAttributeArray("texCoord");
		m_shaderProgram.setAttributeArray("texCoord", m_texCoords, 2);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		m_shaderProgram.disableAttributeArray("vertex");
		m_shaderProgram.disableAttributeArray("texCoord");
		m_shaderProgram.release();
	}

protected:
	Data<Gradient> m_gradient;
	Data<Rect> m_area;
	Data<Shader> m_shader;

	GLfloat m_texCoords[8];
	QOpenGLShaderProgram m_shaderProgram;
};

int RenderGradient_HorizontalClass = RegisterObject<RenderGradient_Horizontal>("Render/Horizontal Gradient").setDescription("Draw a horizontal gradient taking the full screen");

class RenderGradient_Vertical : public Renderer
{
public:
	PANDA_CLASS(RenderGradient_Vertical, Renderer)

	RenderGradient_Vertical(PandaDocument *parent)
		: Renderer(parent)
		, gradient(initData(&gradient, "gradient", "Gradient to paint on the screen"))
		, m_area(initData(&m_area, "area", "Area to paint the gradient in. If null, the render area is used instead"))
		, m_shader(initData(&m_shader, "shader", "Shaders used during the rendering"))
	{
		addInput(&gradient);
		addInput(&m_area);
		addInput(&m_shader);

		m_shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = m_shader.getAccessor();
		shaderAcc->setSourceFromFile(QOpenGLShader::Vertex, ":/shaders/PT_noColor_Tex.v.glsl");
		shaderAcc->setSourceFromFile(QOpenGLShader::Fragment, ":/shaders/PT_noColor_Tex.f.glsl");

		m_texCoords[0*2+0] = 0; m_texCoords[0*2+1] = 0;
		m_texCoords[1*2+0] = 0; m_texCoords[1*2+1] = 0;
		m_texCoords[3*2+0] = 1; m_texCoords[3*2+1] = 0;
		m_texCoords[2*2+0] = 1; m_texCoords[2*2+1] = 0;
	}

	void render()
	{
		const Gradient& grad = gradient.getValue();
		Rect area = m_area.getValue();
		if(area.empty())
		{
			QSize size = getLayerSize();
			area = Rect(0, 0, size.width(), size.height());
		}

		GLuint texture = helper::GradientCache::getInstance()->getTexture(grad, area.height());
		if(texture == -1)
			return;

		if(!m_shader.getValue().apply(m_shaderProgram))
			return;

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		m_shaderProgram.bind();
		m_shaderProgram.setUniformValue("MVP", getMVPMatrix());

		PReal verts[8];
		verts[0*2+0] = area.right(); verts[0*2+1] = area.top();
		verts[1*2+0] = area.left(); verts[1*2+1] = area.top();
		verts[2*2+0] = area.right(); verts[2*2+1] = area.bottom();
		verts[3*2+0] = area.left(); verts[3*2+1] = area.bottom();

		m_shaderProgram.enableAttributeArray("vertex");
		m_shaderProgram.setAttributeArray("vertex", verts, 2);

		m_shaderProgram.enableAttributeArray("texCoord");
		m_shaderProgram.setAttributeArray("texCoord", m_texCoords, 2);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		m_shaderProgram.disableAttributeArray("vertex");
		m_shaderProgram.disableAttributeArray("texCoord");
		m_shaderProgram.release();
	}

protected:
	Data<Gradient> gradient;
	Data<Rect> m_area;
	Data<Shader> m_shader;

	GLfloat m_texCoords[8];
	QOpenGLShaderProgram m_shaderProgram;
};

int RenderGradient_VerticalClass = RegisterObject<RenderGradient_Vertical>("Render/Vertical Gradient").setDescription("Draw a vertical gradient taking the full screen");


} // namespace panda
