#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include "ShaderEffects.h"

namespace panda {

using types::ImageWrapper;

class ModifierImage_GaussianPyramid : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_GaussianPyramid, PandaObject)

	ModifierImage_GaussianPyramid(PandaDocument* doc)
		: PandaObject(doc)
		, m_input(initData("input", "The original image"))
		, m_gaussian(initData("gaussian", "List of scaled down images"))
		, m_laplacian(initData("laplacian", "List of substracted images"))
		, m_levels(initData(4, "levels", "Number of levels to use"))
	{
		addInput(m_input);
		addInput(m_levels);

		addOutput(m_gaussian);
		addOutput(m_laplacian);

		m_downscaleProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/PT_noColor_Tex.v.glsl");
		m_downscaleProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/Downsample2.f.glsl");
		m_downscaleProgram.link();

		m_upscaleProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/PT_noColor_Tex.v.glsl");
		m_upscaleProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/PT_noColor_Tex.f.glsl");
		m_upscaleProgram.link();

		m_differenceProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/PT_noColor_Tex.v.glsl");
		m_differenceProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/Difference.f.glsl");
		m_differenceProgram.link();
	}

	void update()
	{
		cleanDirty();

		const auto& inputVal = m_input.getValue();

		GLuint inputTexId = m_input.getValue().getTextureId();
		int nbLevels = std::min(m_levels.getValue(), 32);
		if(!inputTexId || nbLevels <= 0)
		{
			m_gaussian.getAccessor().clear();
			m_laplacian.getAccessor().clear();
			return;
		}

		QSize inputSize = inputVal.size();
		auto gaussianAcc = m_gaussian.getAccessor();
		auto laplacianAcc = m_laplacian.getAccessor();
		if(gaussianAcc.size() != nbLevels)
		{
			gaussianAcc.resize(nbLevels);
			laplacianAcc.resize(nbLevels);
			m_blurred.resize(nbLevels);
		}

		QOpenGLFramebufferObjectFormat floatFormat;
		floatFormat.setInternalTextureFormat(GL_RGB16);

		QSize size = inputSize;
		GLuint texId = inputTexId;
		for(int i=0; i<nbLevels; ++i)
		{
			auto& gaussianImg = gaussianAcc[i];
			auto& laplacianImg = laplacianAcc[i];
			auto& blurred = m_blurred[i];
			resizeFBO(m_blurred[i], size);
			resizeFBO(laplacianImg, size, floatFormat);
			size /= 2;
			resizeFBO(gaussianImg, size);

			// Downscaling to get the gaussian
			renderImage(*gaussianImg.getFbo(), m_downscaleProgram, texId);
			auto gaussTexId = gaussianImg.getTextureId();

			// Upscaling the gaussian to get the blurred image
			renderImage(*blurred.data(), m_upscaleProgram, gaussTexId);

			// Substraction of the original and the blurred
			renderImage(*laplacianImg.getFbo(), m_differenceProgram, texId, blurred->texture());

			texId = gaussTexId;
		}
	}

protected:
	QOpenGLShaderProgram m_downscaleProgram, m_upscaleProgram, m_differenceProgram;
	QVector<QSharedPointer<QOpenGLFramebufferObject>> m_blurred;
	Data< ImageWrapper > m_input;
	Data< QVector< ImageWrapper > > m_gaussian, m_laplacian;
	Data< int > m_levels;
};

int ModifierImage_GaussianPyramidClass = RegisterObject<ModifierImage_GaussianPyramid>("Modifier/Image/Gaussian & Laplacian pyramids")
		.setDescription("Build the Gaussian and Laplacian pyramid of an image");

} // namespace Panda
