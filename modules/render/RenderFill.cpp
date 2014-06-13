#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Gradient.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>
#include <panda/helper/GradientCache.h>

#include <QOpenGLShaderProgram>

namespace panda {

using types::ImageWrapper;
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
		if(!texture)
			return;

		if(!m_shader.getValue().apply(m_shaderProgram))
			return;

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		m_shaderProgram.setUniformValue("tex0", 0);

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

int RenderGradient_HorizontalClass = RegisterObject<RenderGradient_Horizontal>("Render/Gradient/Horizontal")
		.setName("Horizontal gradient").setDescription("Draw a horizontal gradient in an area");

//****************************************************************************//

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
		if(!texture)
			return;

		if(!m_shader.getValue().apply(m_shaderProgram))
			return;

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		m_shaderProgram.setUniformValue("tex0", 0);

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

int RenderGradient_VerticalClass = RegisterObject<RenderGradient_Vertical>("Render/Gradient/Vertical")
		.setName("Vertical gradient").setDescription("Draw a vertical gradient in an area");

//****************************************************************************//

class RenderFill_Motif : public Renderer
{
public:
	PANDA_CLASS(RenderFill_Motif, Renderer)

	RenderFill_Motif(PandaDocument *parent)
		: Renderer(parent)
		, texture(initData(&texture, "texture", "Motif to paint on the screen"))
		, m_area(initData(&m_area, "area", "Area to paint the gradient in. If null, the render area is used instead"))
		, m_shader(initData(&m_shader, "shader", "Shaders used during the rendering"))
	{
		addInput(&texture);
		addInput(&m_area);
		addInput(&m_shader);

		m_shader.setWidgetData("Vertex;Fragment");
		auto shaderAcc = m_shader.getAccessor();
		shaderAcc->setSourceFromFile(QOpenGLShader::Vertex, ":/shaders/PT_noColor_Tex.v.glsl");
		shaderAcc->setSourceFromFile(QOpenGLShader::Fragment, ":/shaders/PT_noColor_Tex.f.glsl");
	}

	void render()
	{
		Rect area = m_area.getValue();
		if(area.empty())
		{
			QSize size = getLayerSize();
			area = Rect(0, 0, size.width(), size.height());
		}

		const auto& texValue = texture.getValue();
		GLuint texId = texValue.getTextureId();
		if(!texId)
			return;

		if(!m_shader.getValue().apply(m_shaderProgram))
			return;

		QSize texSize = texValue.size();

		glBindTexture(GL_TEXTURE_2D, texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		m_shaderProgram.setUniformValue("tex0", 0);

		m_shaderProgram.bind();
		m_shaderProgram.setUniformValue("MVP", getMVPMatrix());

		PReal verts[8], texCoords[8];
		verts[0*2+0] = area.right(); verts[0*2+1] = area.top();
		verts[1*2+0] = area.left(); verts[1*2+1] = area.top();
		verts[2*2+0] = area.right(); verts[2*2+1] = area.bottom();
		verts[3*2+0] = area.left(); verts[3*2+1] = area.bottom();

		float tw = area.width() / texSize.width(), th = area.height() / texSize.height();
		texCoords[0*2+0] = tw;	texCoords[0*2+1] = th;
		texCoords[1*2+0] = 0;	texCoords[1*2+1] = th;
		texCoords[3*2+0] = 0;	texCoords[3*2+1] = 0;
		texCoords[2*2+0] = tw;	texCoords[2*2+1] = 0;

		m_shaderProgram.enableAttributeArray("vertex");
		m_shaderProgram.setAttributeArray("vertex", verts, 2);

		m_shaderProgram.enableAttributeArray("texCoord");
		m_shaderProgram.setAttributeArray("texCoord", texCoords, 2);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		m_shaderProgram.disableAttributeArray("vertex");
		m_shaderProgram.disableAttributeArray("texCoord");
		m_shaderProgram.release();
	}

protected:
	Data<ImageWrapper> texture;
	Data<Rect> m_area;
	Data<Shader> m_shader;

	QOpenGLShaderProgram m_shaderProgram;
};

int RenderFill_MotifClass = RegisterObject<RenderFill_Motif>("Render/Textured/Fill with motif")
		.setDescription("Fill an area with a motif");


} // namespace panda
