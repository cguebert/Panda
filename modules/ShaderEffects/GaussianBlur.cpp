#include "ShaderEffects.h"
#include <panda/ObjectFactory.h>

namespace panda {

using types::ImageWrapper;

class ModifierImage_GaussianBlur : public ShaderEffects
{
public:
	PANDA_CLASS(ModifierImage_GaussianBlur, ShaderEffects)

	ModifierImage_GaussianBlur(PandaDocument* doc)
		: ShaderEffects(doc, 2)
		, m_radius(initData(&m_radius, (PReal)10, "radius", "Radius of the blur"))
	{
		addInput(&m_radius);
	}

	void initShaderProgram(int passId, QOpenGLShaderProgram& program)
	{
		if(!passId)
		{
			program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/GBlur.v.glsl");
			program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/GBlur.f.glsl");
			program.link();
		}
		else
		{
			program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/GBlur.v.glsl");
			program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/GBlur.f.glsl");
			program.link();
		}
	}

	void prepareUpdate(QSize size)
	{
		m_size = size;
	}

	void preparePass(int passId, QOpenGLShaderProgram& program)
	{
		if(!passId)
		{ // H
			program.setUniformValue("pixelOffset", QPointF(1.0/m_size.width(), 0));
		}
		else
		{ // V
			program.setUniformValue("pixelOffset", QPointF(0, 1.0/m_size.height()));
		}
	}

protected:
	Data< PReal > m_radius;
	QSize m_size;
};

int ModifierImage_GaussianBlurClass = RegisterObject<ModifierImage_GaussianBlur>("Modifier/Image/Gaussian blur")
		.setDescription("Apply a gaussian blur to an image");

} // namespace Panda
