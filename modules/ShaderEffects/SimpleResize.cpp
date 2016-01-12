#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>

#include "ShaderEffects.h"

namespace panda {

using types::ImageWrapper;

class ModifierImage_Downscale : public OGLObject
{
public:
	PANDA_CLASS(ModifierImage_Downscale, OGLObject)

	ModifierImage_Downscale(PandaDocument* doc)
		: OGLObject(doc)
		, m_input(initData("input", "The original image"))
		, m_output(initData("output", "Image created by the operation"))
		, m_nbOfDownscales(initData(1, "downscales", "Number of times the image is to be downscaled"))
	{
		addInput(m_input);
		addInput(m_nbOfDownscales);

		addOutput(m_output);
	}

	void initializeGL()
	{
		m_shaderProgram2x.addShaderFromFile(graphics::ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		m_shaderProgram2x.addShaderFromFile(graphics::ShaderType::Fragment, "shaders/Downsample2.f.glsl");
		m_shaderProgram2x.link();

		m_shaderProgram4x.addShaderFromFile(graphics::ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		m_shaderProgram4x.addShaderFromFile(graphics::ShaderType::Fragment, "shaders/Downsample4.f.glsl");
		m_shaderProgram4x.link();
	}

	void update()
	{
		GLuint texId = m_input.getValue().getTextureId();
		if(!texId)
		{
			m_output.getAccessor()->clear();
			return;
		}

		int nbOfDownscales = m_nbOfDownscales.getValue();

		if(nbOfDownscales <= 0)
		{ // Copy input to output
			m_output.getAccessor().wref() = m_input.getValue();
			return;
		}

		unsigned int nb4 = nbOfDownscales / 2, nb2 = nbOfDownscales % 2;
		unsigned int nbOp = nb4 + nb2;
		if(nbOp != m_FBOs.size())
			m_FBOs.resize(nbOp);

		auto renderSize = m_input.getValue().size();
		for(unsigned int i=0; i<nb4; ++i)
		{
			renderSize /= 4;
			auto& fbo = m_FBOs[i];
			resizeFBO(fbo, renderSize);
			renderImage(fbo, m_shaderProgram4x, texId);
			texId = fbo.texture();
		}

		if(nb2)
		{
			renderSize /= 2;
			auto& fbo = m_FBOs.back();
			resizeFBO(fbo, renderSize);
			renderImage(fbo, m_shaderProgram2x, texId);
		}

		m_output.getAccessor()->setFbo(m_FBOs.back());
	}

protected:
	Data< ImageWrapper > m_input, m_output;
	Data< int > m_nbOfDownscales;

	graphics::ShaderProgram m_shaderProgram2x, m_shaderProgram4x;
	std::vector<graphics::Framebuffer> m_FBOs;
};

int ModifierImage_DownscaleClass = RegisterObject<ModifierImage_Downscale>("Modifier/Image/Downscale")
		.setDescription("Divide by 2 the size of the image");

//****************************************************************************//

class ModifierImage_Upscale : public OGLObject
{
public:
	PANDA_CLASS(ModifierImage_Upscale, OGLObject)

	ModifierImage_Upscale(PandaDocument* doc)
		: OGLObject(doc)
		, m_input(initData("input", "The original image"))
		, m_output(initData("output", "Image created by the operation"))
		, m_nbOfUpscales(initData(1, "upscales", "Number of times the image is to be upscaled"))
	{
		addInput(m_input);
		addInput(m_nbOfUpscales);

		addOutput(m_output);
	}

	void initializeGL()
	{
		m_shaderProgram.addShaderFromFile(graphics::ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		m_shaderProgram.addShaderFromFile(graphics::ShaderType::Fragment, "shaders/PT_noColor_Tex.f.glsl");
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

		auto renderSize = m_input.getValue().size();
		for(int i=0, nb = m_nbOfUpscales.getValue(); i < nb; ++i)
			renderSize *= 2;
		if(resizeFBO(m_outputFBO, renderSize))
			m_output.getAccessor()->setFbo(m_outputFBO);

		renderImage(m_outputFBO, m_shaderProgram, texId);
	}

protected:
	Data< ImageWrapper > m_input, m_output;
	Data< int > m_nbOfUpscales;

	graphics::ShaderProgram m_shaderProgram;
	graphics::Framebuffer m_outputFBO;
};

int ModifierImage_UpscaleClass = RegisterObject<ModifierImage_Upscale>("Modifier/Image/Upscale")
		.setDescription("Multiply by 2 the size of the image");

} // namespace Panda
