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
			program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/GBlurH.v.glsl");
			program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/GBlur.f.glsl");
			program.link();
		}
		else
		{
			program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/GBlurV.v.glsl");
			program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/GBlur.f.glsl");
			program.link();
		}
	}

	void prepareUpdate(QSize size)
	{
		PReal radius = m_radius.getValue() / 7;
		m_radiusScaleW = radius / size.width();
		m_radiusScaleH = radius / size.height();
	}

	void preparePass(int passId, QOpenGLShaderProgram& program)
	{
		if(!passId)
			program.setUniformValue("radiusScale", m_radiusScaleH);
		else
			program.setUniformValue("radiusScale", m_radiusScaleW);
	}

protected:
	Data< PReal > m_radius;
	PReal m_radiusScaleW, m_radiusScaleH;
};

int ModifierImage_GaussianBlurClass = RegisterObject<ModifierImage_GaussianBlur>("Modifier/Image/Gaussian blur")
		.setDescription("Apply a gaussian blur to an image");

} // namespace Panda
