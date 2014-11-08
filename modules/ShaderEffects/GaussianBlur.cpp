#include "ShaderEffects.h"
#include <panda/ObjectFactory.h>

#include <QFile>

namespace panda {

using types::ImageWrapper;

class ModifierImage_GaussianBlur : public ShaderEffects
{
public:
	PANDA_CLASS(ModifierImage_GaussianBlur, ShaderEffects)

	ModifierImage_GaussianBlur(PandaDocument* doc)
		: ShaderEffects(doc, 2)
		, m_radius(initData(&m_radius, (PReal)10, "radius", "Radius of the blur"))
		, m_currentRadius(-1)
		, m_kernelSize(0)
	{
		addInput(&m_radius);

		m_vertexShader = QSharedPointer<QOpenGLShader>::create(QOpenGLShader::Vertex);
		m_vertexShader->compileSourceFile(":/shaders/GBlur.v.glsl");

		m_fragmentShader = QSharedPointer<QOpenGLShader>::create(QOpenGLShader::Fragment);

		m_shaderProgram = QSharedPointer<QOpenGLShaderProgram>::create();
	}

	~ModifierImage_GaussianBlur()
	{
		// Make sure to clear the program first so that the shaders can free themselves
		m_shaderProgram.reset();
	}

	// TODO: only create half of it
	std::vector<float> computeKernel()
	{
		int halfKernelSize = ceil(m_currentRadius / 2);
		m_kernelSize = halfKernelSize * 2 + 1;

		std::vector<float> kernel;
		kernel.resize( m_kernelSize );

		const float cPI = 3.14159265358979323846f;
		float mean		= halfKernelSize;
		float sigma		= 0.8 + 0.3 * (m_currentRadius / 2 - 1);
		double sum		= 0.0;
		for (int x = 0; x < m_kernelSize; ++x)
		{
			kernel[x] = (float)sqrt( exp( -0.5 * (pow((x-mean)/sigma, 2.0) + pow((mean)/sigma,2.0)) )
				/ (2 * cPI * sigma * sigma) );
			sum += kernel[x];
		}
		float sumf = (float)sum;
		for (int x = 0; x < m_kernelSize; ++x)
			kernel[x] /= sumf;

		return kernel;
	}

	void updateShaders()
	{
		PReal radius = m_radius.getValue();
		m_currentRadius = std::max(radius, (PReal)0.1);

		QFile file(":/shaders/GBlur.f.glsl");
		if(file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QString source = QTextStream(&file).readAll();
			std::vector<float> kernel = computeKernel();

			std::vector<float> oneSideInputs;
			for (int i = (m_kernelSize / 2); i >= 0; --i)
			{
				if(i == (m_kernelSize / 2))
					oneSideInputs.push_back(kernel[i] * 0.5f);
				else
					oneSideInputs.push_back(kernel[i]);
			}
			if(oneSideInputs.size() % 2)
				oneSideInputs.push_back(0);

			int numSamples = (int)oneSideInputs.size() / 2;
			std::vector<float> weights(numSamples);
			for (int i = 0; i < numSamples; ++i)
				weights[i] = oneSideInputs[i*2+0] + oneSideInputs[i*2+1];

			std::vector<float> offsets(numSamples);
			for (int i = 0; i < numSamples; ++i)
				offsets[i] = i*2.0f + oneSideInputs[i*2+1] / weights[i];

			source.replace("~~1~~", QString::number(numSamples));

			QString weightsString;
			for(const auto& w : weights)
			{
				if(!weightsString.isEmpty())
					weightsString += ", ";
				weightsString += QString::number(w, 'f');
			}
			source.replace("~~2~~", weightsString);

			QString offsetsString;
			for(const auto& off : offsets)
			{
				if(!offsetsString.isEmpty())
					offsetsString += ", ";
				offsetsString += QString::number(off, 'f');
			}
			source.replace("~~3~~", offsetsString);

			m_fragmentShader->compileSourceCode(source);
		}

		m_shaderProgram->removeAllShaders();
		m_shaderProgram->addShader(m_vertexShader.data());
		m_shaderProgram->addShader(m_fragmentShader.data());
	}

	void prepareUpdate(QSize size)
	{
		m_size = size;
		if(m_currentRadius < 0 || m_radius.getValue() != m_currentRadius)
			updateShaders();
	}

	QOpenGLShaderProgram& preparePass(int passId)
	{
		m_shaderProgram->bind();
		if(!passId)
		{ // H
			m_shaderProgram->setUniformValue("pixelOffset", QPointF(1.0/m_size.width(), 0));
		}
		else
		{ // V
			m_shaderProgram->setUniformValue("pixelOffset", QPointF(0, 1.0/m_size.height()));
		}

		return *m_shaderProgram.data();
	}

protected:
	Data< PReal > m_radius;
	PReal m_currentRadius;
	int m_kernelSize;
	QSize m_size;
	QSharedPointer<QOpenGLShader> m_vertexShader, m_fragmentShader;
	QSharedPointer<QOpenGLShaderProgram> m_shaderProgram;
};

int ModifierImage_GaussianBlurClass = RegisterObject<ModifierImage_GaussianBlur>("Modifier/Image/Gaussian blur")
		.setDescription("Apply a gaussian blur to an image");

} // namespace Panda
