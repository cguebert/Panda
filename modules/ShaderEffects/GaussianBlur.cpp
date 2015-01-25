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
		, m_radius(initData((PReal)10, "radius", "Radius of the blur"))
		, m_currentRadius(-1)
		, m_halfKernelSize(0)
	{
		addInput(m_radius);

		m_vertexShader = QSharedPointer<QOpenGLShader>::create(QOpenGLShader::Vertex);
		m_vertexShader->compileSourceFile(":/shaders/PT_noColor_Tex.v.glsl");

		m_fragmentShader = QSharedPointer<QOpenGLShader>::create(QOpenGLShader::Fragment);

		m_shaderProgram = QSharedPointer<QOpenGLShaderProgram>::create();
	}

	~ModifierImage_GaussianBlur()
	{
		// Make sure to clear the program first so that the shaders can free themselves
		m_shaderProgram.reset();
	}

	std::vector<float> computeHalfKernel()
	{
		m_halfKernelSize = ceil(m_currentRadius / 2) + 1;

		std::vector<float> kernel(m_halfKernelSize);
		const float cPI = 3.14159265358979323846f;
		float sigma		= 0.8 + 0.3 * (m_currentRadius / 2 - 1);
		double sum		= 0.0;
		for (int x = 0; x < m_halfKernelSize; ++x)
		{
			float val = (float)sqrt( exp( -0.5 * pow(x/sigma, 2.0) ) / (2 * cPI * sigma * sigma) );
			kernel[x] = val;
			sum += x ? val * 2 : val;
		}
		float sumf = (float)sum;
		for (int x = 0; x < m_halfKernelSize; ++x)
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
	int m_halfKernelSize;
	QSize m_size;
	QSharedPointer<QOpenGLShader> m_vertexShader, m_fragmentShader;
	QSharedPointer<QOpenGLShaderProgram> m_shaderProgram;
};

int ModifierImage_GaussianBlurClass = RegisterObject<ModifierImage_GaussianBlur>("Modifier/Image/Gaussian blur")
		.setDescription("Apply a gaussian blur to an image");

} // namespace Panda
