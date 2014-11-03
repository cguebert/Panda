#include <panda/ObjectFactory.h>
#include <panda/PandaObject.h>
#include <panda/types/ImageWrapper.h>
#include <panda/helper/system/FileRepository.h>

#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QSharedPointer>

#include "ShaderEffects.h"

namespace panda {

using types::ImageWrapper;

class ModifierImage_Downsample : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_Downsample, PandaObject)

	ModifierImage_Downsample(PandaDocument* doc)
		: PandaObject(doc)
		, m_input(initData(&m_input, "input", "The original image"))
		, m_output(initData(&m_output, "output", "Image created by the operation"))
		, m_nbOfDownscales(initData(&m_nbOfDownscales, 1, "downscales", "Number of times the image is to be downscaled"))
	{
		addInput(&m_input);
		addInput(&m_nbOfDownscales);

		addOutput(&m_output);

		m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
			helper::system::DataRepository.loadFile("shaders/PT_noColor_Tex.v.glsl"));
		m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
			helper::system::DataRepository.loadFile("shaders/PT_noColor_Tex.f.glsl"));
		m_shaderProgram.link();
	}

	void update()
	{
		GLuint texId = m_input.getValue().getTextureId();
		if(!texId)
		{
			m_output.getAccessor()->clear();
			return;
		}

		QSize inputSize = m_input.getValue().size();
		QSize outputSize = inputSize / 2;
		if(resizeFBO(m_outputFBO, outputSize))
			m_output.getAccessor()->setFbo(m_outputFBO);

		renderImage(m_outputFBO, m_shaderProgram, texId);
	}

protected:
	Data< ImageWrapper > m_input, m_output;
	Data< int > m_nbOfDownscales;

	QOpenGLShaderProgram m_shaderProgram;
	QSharedPointer<QOpenGLFramebufferObject> m_outputFBO;
};

int ModifierImage_DownsampleClass = RegisterObject<ModifierImage_Downsample>("Modifier/Image/Downsample")
		.setDescription("Divide by 2 the size of the image");

//****************************************************************************//

class ModifierImage_Upsample : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_Upsample, PandaObject)

	ModifierImage_Upsample(PandaDocument* doc)
		: PandaObject(doc)
		, m_input(initData(&m_input, "input", "The original image"))
		, m_output(initData(&m_output, "output", "Image created by the operation"))
		, m_nbOfUpscales(initData(&m_nbOfUpscales, 1, "upscales", "Number of times the image is to be upscaled"))
	{
		addInput(&m_input);
		addInput(&m_nbOfUpscales);

		addOutput(&m_output);
		m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
			helper::system::DataRepository.loadFile("shaders/PT_noColor_Tex.v.glsl"));
		m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
			helper::system::DataRepository.loadFile("shaders/PT_noColor_Tex.f.glsl"));
		m_shaderProgram.link();
	}

	void update()
	{
		GLuint texId = m_input.getValue().getTextureId();
		if(!texId)
		{
			m_output.getAccessor()->clear();
			return;
		}

		QSize inputSize = m_input.getValue().size();
		QSize outputSize = inputSize * 2;
		if(resizeFBO(m_outputFBO, outputSize))
			m_output.getAccessor()->setFbo(m_outputFBO);

		renderImage(m_outputFBO, m_shaderProgram, texId);
	}

protected:
	Data< ImageWrapper > m_input, m_output;
	Data< int > m_nbOfUpscales;

	QOpenGLShaderProgram m_shaderProgram;
	QSharedPointer<QOpenGLFramebufferObject> m_outputFBO;
};

int ModifierImage_UpsampleClass = RegisterObject<ModifierImage_Upsample>("Modifier/Image/Upsample")
		.setDescription("Multiply by 2 the size of the image");

} // namespace Panda
