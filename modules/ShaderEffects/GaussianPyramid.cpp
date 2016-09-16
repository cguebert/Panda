#include <panda/document/RenderedDocument.h>
#include <panda/object/ObjectFactory.h>

#include "ShaderEffects.h"

namespace panda {

using types::ImageWrapper;

class ModifierImage_GaussianPyramid : public ShaderEffects
{
public:
	PANDA_CLASS(ModifierImage_GaussianPyramid, ShaderEffects)

	ModifierImage_GaussianPyramid(PandaDocument* doc)
		: ShaderEffects(doc)
		, m_input(initData("input", "The original image"))
		, m_gaussian(initData("gaussian", "List of scaled down images"))
		, m_laplacian(initData("laplacian", "List of substracted images"))
		, m_levels(initData(4, "levels", "Number of levels to use"))
	{
		addInput(m_input);
		addInput(m_levels);

		addOutput(m_gaussian);
		addOutput(m_laplacian);
	}

	void initializeGL()
	{
		m_downscaleProgram.addShaderFromFile(graphics::ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		m_downscaleProgram.addShaderFromFile(graphics::ShaderType::Fragment, "shaders/Downsample2.f.glsl");
		m_downscaleProgram.link();

		m_upscaleProgram.addShaderFromFile(graphics::ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		m_upscaleProgram.addShaderFromFile(graphics::ShaderType::Fragment, "shaders/PT_noColor_Tex.f.glsl");
		m_upscaleProgram.link();

		m_differenceProgram.addShaderFromFile(graphics::ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		m_differenceProgram.addShaderFromFile(graphics::ShaderType::Fragment, "shaders/Difference.f.glsl");
		m_differenceProgram.link();
	}

	void update()
	{
		const auto& inputVal = m_input.getValue();

		GLuint inputTexId = m_input.getValue().getTextureId();
		int nbLevels = std::min(m_levels.getValue(), 32);
		if(!inputTexId || nbLevels <= 0)
		{
			m_gaussian.getAccessor().clear();
			m_laplacian.getAccessor().clear();
			return;
		}

		auto inputSize = inputVal.size();
		auto gaussianAcc = m_gaussian.getAccessor();
		auto laplacianAcc = m_laplacian.getAccessor();
		if(gaussianAcc.size() != nbLevels)
		{
			gaussianAcc.resize(nbLevels);
			laplacianAcc.resize(nbLevels);
			m_blurred.resize(nbLevels);
		}

		graphics::FramebufferFormat floatFormat;
		floatFormat.internalFormat = GL_RGBA16F;

		auto size = inputSize;
		GLuint texId = inputTexId;
		for(int i=0; i<nbLevels; ++i)
		{
			auto& gaussianImg = gaussianAcc[i];
			auto& laplacianImg = laplacianAcc[i];
			auto& blurred = m_blurred[i];
			resizeFBO(blurred, size);
			resizeFBO(laplacianImg, size, floatFormat);
			size /= 2;
			resizeFBO(gaussianImg, size);

			// Downscaling to get the gaussian
			renderImage(*gaussianImg.getFbo(), m_downscaleProgram, texId);
			auto gaussTexId = gaussianImg.getTextureId();

			// Upscaling the gaussian to get the blurred image
			renderImage(blurred, m_upscaleProgram, gaussTexId);

			// Substraction of the original and the blurred
			renderImage(*laplacianImg.getFbo(), m_differenceProgram, texId, blurred.texture());

			texId = gaussTexId;
		}
	}

protected:
	graphics::ShaderProgram m_downscaleProgram, m_upscaleProgram, m_differenceProgram;
	std::vector<graphics::Framebuffer> m_blurred;

	Data< ImageWrapper > m_input;
	Data< std::vector< ImageWrapper > > m_gaussian, m_laplacian;
	Data< int > m_levels;
};

int ModifierImage_GaussianPyramidClass = RegisterObject<ModifierImage_GaussianPyramid, RenderedDocument>("Modifier/Image/Gaussian & Laplacian pyramids")
		.setDescription("Build the Gaussian and Laplacian pyramid of an image");

//****************************************************************************//

class ModifierImage_CollapsePyramid : public ShaderEffects
{
public:
	PANDA_CLASS(ModifierImage_CollapsePyramid, ShaderEffects)

	ModifierImage_CollapsePyramid(PandaDocument* doc)
		: ShaderEffects(doc)
		, m_gaussian(initData("gaussian", "List of scaled down images"))
		, m_laplacian(initData("laplacian", "List of substracted images"))
		, m_output(initData("output", "The reconstructed image"))
	{
		addInput(m_gaussian);
		addInput(m_laplacian);

		addOutput(m_output);
	}

	void initializeGL()
	{
		ShaderEffects::initializeGL();

		m_upscaleProgram.addShaderFromFile(graphics::ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		m_upscaleProgram.addShaderFromFile(graphics::ShaderType::Fragment, "shaders/PT_noColor_Tex.f.glsl");
		m_upscaleProgram.link();

		m_additionProgram.addShaderFromFile(graphics::ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		m_additionProgram.addShaderFromFile(graphics::ShaderType::Fragment, "shaders/Addition.f.glsl");
		m_additionProgram.link();
	}

	void update()
	{
		auto output = m_output.getAccessor();
		const auto& gaussian = m_gaussian.getValue();
		const auto& laplacian = m_laplacian.getValue();

		int nbLevels = laplacian.size();
		if(!nbLevels)
		{
			output->clear();
			return;
		}

		if(m_upscaled.size() != nbLevels)
		{
			m_upscaled.resize(nbLevels);
			m_recomposed.resize(nbLevels-1);
		}

		GLuint prevLevelTexId = gaussian.back().getTextureId();
		for(int i=nbLevels-1; i>=0; --i)
		{
			const auto& laplacianImg = laplacian[i];
			auto& upscaled = m_upscaled[i];
			auto size = laplacianImg.size();
			resizeFBO(upscaled, size);
			if(i)
				resizeFBO(m_recomposed[i-1], size);
			else
				resizeFBO(output.wref(), size);

			// Upscaling the gaussian
			renderImage(upscaled, m_upscaleProgram, prevLevelTexId);

			// Addition of the original and the upscaled
			auto& recomposed = (i ? m_recomposed[i-1] : *output->getFbo());
			renderImage(recomposed, m_additionProgram, upscaled.texture(), laplacianImg.getTextureId());
			prevLevelTexId = recomposed.texture();
		}
	}

protected:
	graphics::ShaderProgram m_upscaleProgram, m_additionProgram;
	std::vector<graphics::Framebuffer> m_upscaled, m_recomposed;

	Data< std::vector< ImageWrapper > > m_gaussian, m_laplacian;
	Data< ImageWrapper > m_output;
};

int ModifierImage_CollapsePyramidClass = RegisterObject<ModifierImage_CollapsePyramid, RenderedDocument>("Modifier/Image/Collapse pyramids")
		.setDescription("Reconstruct an image from the Gaussian and Laplacian pyramids");

} // namespace Panda
