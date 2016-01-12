#include <panda/ObjectFactory.h>
#include <panda/helper/algorithm.h>
#include <panda/helper/ShaderCache.h>
#include <panda/helper/system/FileRepository.h>

#include "ShaderEffects.h"

namespace panda {

using types::ImageWrapper;

class ModifierImage_GaussianBlur : public ShaderEffects
{
public:
	PANDA_CLASS(ModifierImage_GaussianBlur, ShaderEffects)

	ModifierImage_GaussianBlur(PandaDocument* doc)
		: ShaderEffects(doc, 2)
		, m_radius(initData((PReal)10, "radius", "Radius of the blur"))
		, m_currentRadius(-1)
		, m_halfKernelSize(0)
	{
		addInput(m_radius);
	}

	~ModifierImage_GaussianBlur()
	{
		// Make sure to clear the program first so that the shaders can free themselves
		m_shaderProgram.clear();
	}

	void initializeGL() override
	{
		m_vertexShader = helper::ShaderCache::getInstance()->getShader(graphics::ShaderType::Vertex,
			helper::system::DataRepository.loadFile("shaders/PT_noColor_Tex.v.glsl"));

		m_fragmentSource = helper::system::DataRepository.loadFile("shaders/GBlur.f.glsl");
	}

	std::vector<float> computeHalfKernel()
	{
		m_halfKernelSize = static_cast<int>(std::ceil(m_currentRadius / 2)) + 1;

		std::vector<float> kernel(m_halfKernelSize);
		const float cPI = 3.14159265358979323846f;
		float sigma		= 0.8f + 0.3f * (m_currentRadius / 2 - 1);
		double sum		= 0.0;
		for (int x = 0; x < m_halfKernelSize; ++x)
		{
			float val = (float)sqrt( exp( -0.5 * pow(x/sigma, 2.0) ) / (2 * cPI * sigma * sigma) );
			kernel[x] = val;
			sum += x ? val * 2 : val;
		}
		float sumf = static_cast<float>(sum);
		for (int x = 0; x < m_halfKernelSize; ++x)
			kernel[x] /= sumf;

		return kernel;
	}

	void updateShaders()
	{
		PReal radius = m_radius.getValue();
		m_currentRadius = std::max(radius, (PReal)0.1);

		if (m_fragmentSource.empty())
			return;

		std::string source = m_fragmentSource;
		std::vector<float> halfKernel = computeHalfKernel();
		halfKernel[0] /= 2;

		if(halfKernel.size() % 2)
			halfKernel.push_back(0);

		int numSamples = (int)halfKernel.size() / 2;
		std::vector<float> weights(numSamples);
		for (int i = 0; i < numSamples; ++i)
			weights[i] = halfKernel[i*2+0] + halfKernel[i*2+1];

		std::vector<float> offsets(numSamples);
		for (int i = 0; i < numSamples; ++i)
			offsets[i] = i*2.0f + halfKernel[i*2+1] / weights[i];

		helper::replaceAll<std::string>(source, "~~1~~", std::to_string(numSamples));

		std::string weightsString;
		for(const auto& w : weights)
		{
			if(!weightsString.empty())
				weightsString += ", ";
			weightsString += std::to_string(w);
		}
		helper::replaceAll<std::string>(source, "~~2~~", weightsString);

		std::string offsetsString;
		for(const auto& off : offsets)
		{
			if(!offsetsString.empty())
				offsetsString += ", ";
			offsetsString += std::to_string(off);
		}
		helper::replaceAll<std::string>(source, "~~3~~", offsetsString);

		m_shaderProgram.clear();
		m_shaderProgram.addShader(graphics::ShaderType::Vertex, m_vertexShader);
		m_shaderProgram.addShaderFromMemory(graphics::ShaderType::Fragment, source);
	}

	void prepareUpdate(graphics::Size size)
	{
		m_size = size;
		if(m_currentRadius < 0 || m_radius.getValue() != m_currentRadius)
			updateShaders();
	}

	graphics::ShaderProgram& preparePass(int passId)
	{
		if (!m_shaderProgram.isLinked())
			m_shaderProgram.link();
		m_shaderProgram.bind();
		if(!passId) // H
			m_shaderProgram.setUniformValueArray("pixelOffset", types::Point(1.0f / m_size.width(), 0).data(), 1, 2);
		else // V
			m_shaderProgram.setUniformValueArray("pixelOffset", types::Point(0, 1.0f / m_size.height()).data(), 1, 2);

		return m_shaderProgram;
	}

protected:
	Data< PReal > m_radius;
	PReal m_currentRadius;
	int m_halfKernelSize;
	graphics::Size m_size;
	std::string m_fragmentSource;
	graphics::ShaderId::SPtr m_vertexShader;
	graphics::ShaderProgram m_shaderProgram;
};

int ModifierImage_GaussianBlurClass = RegisterObject<ModifierImage_GaussianBlur>("Modifier/Image/Gaussian blur")
		.setDescription("Apply a gaussian blur to an image");

} // namespace Panda
