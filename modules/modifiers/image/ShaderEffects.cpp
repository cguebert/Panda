#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>

#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>

namespace panda {

using types::ImageWrapper;

class ModifierImage_GaussianBlur : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_GaussianBlur, PandaObject)

	ModifierImage_GaussianBlur(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData(&m_input, "input", "The original image"))
		, m_output(initData(&m_output, "output", "Image created by the operation"))
		, m_radius(initData(&m_radius, (PReal)10, "radius", "Radius of the blur"))
	{
		addInput(&m_input);
		addInput(&m_radius);

		addOutput(&m_output);

		m_firstPass.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/effects/GBlurH.v.glsl");
		m_firstPass.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/effects/GBlur.f.glsl");
		m_firstPass.link();

		m_secondPass.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/effects/GBlurV.v.glsl");
		m_secondPass.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/effects/GBlur.f.glsl");
		m_secondPass.link();

		m_texCoords[0*2+0] = 1; m_texCoords[0*2+1] = 1;
		m_texCoords[1*2+0] = 0; m_texCoords[1*2+1] = 1;
		m_texCoords[3*2+0] = 0; m_texCoords[3*2+1] = 0;
		m_texCoords[2*2+0] = 1; m_texCoords[2*2+1] = 0;
	}

	void update()
	{
		const auto& inputVal = m_input.getValue();
		GLuint texId = inputVal.getTextureId();
		if(texId)
		{
			QSize inputSize = inputVal.size();
			auto outputAcc = m_output.getAccessor();
			QOpenGLFramebufferObject* outputFbo = outputAcc->getFbo();
			if(!outputFbo || outputFbo->size() != inputSize)
			{
				m_intermediaryFbo.reset(new QOpenGLFramebufferObject(inputSize));
				auto newFbo = QSharedPointer<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(inputSize));
				outputAcc->setFbo(newFbo);
				outputFbo = newFbo.data();
			}

			glClearColor(0, 0, 0, 0);
			glViewport(0, 0, inputSize.width(), inputSize.height());

			QMatrix4x4 mvp = QMatrix4x4();
			mvp.ortho(0, inputSize.width(), inputSize.height(), 0, -10, 10);

			GLfloat verts[8];
			verts[0*2+0] = inputSize.width();	verts[0*2+1] = 0;
			verts[1*2+0] = 0;					verts[1*2+1] = 0;
			verts[3*2+0] = 0;					verts[3*2+1] = inputSize.height();
			verts[2*2+0] = inputSize.width();	verts[2*2+1] = inputSize.height();

			PReal radius = m_radius.getValue();

		// First pass
			m_firstPass.bind();
			m_intermediaryFbo->bind();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindTexture(GL_TEXTURE_2D, texId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			m_firstPass.setUniformValue("MVP", mvp);
			m_firstPass.setUniformValue("tex0", 0);

			m_firstPass.enableAttributeArray("vertex");
			m_firstPass.setAttributeArray("vertex", verts, 2);

			m_firstPass.enableAttributeArray("texCoord");
			m_firstPass.setAttributeArray("texCoord", m_texCoords, 2);

			m_firstPass.setUniformValue("radiusScale", radius / inputSize.width() / 7);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			m_firstPass.disableAttributeArray("vertex");
			m_firstPass.disableAttributeArray("texCoord");
			m_firstPass.release();

			m_intermediaryFbo->release();

		// Second pass
			m_secondPass.bind();
			outputFbo->bind();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindTexture(GL_TEXTURE_2D, m_intermediaryFbo->texture());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			m_secondPass.setUniformValue("MVP", mvp);
			m_secondPass.setUniformValue("tex0", 0);

			m_secondPass.enableAttributeArray("vertex");
			m_secondPass.setAttributeArray("vertex", verts, 2);

			m_secondPass.enableAttributeArray("texCoord");
			m_secondPass.setAttributeArray("texCoord", m_texCoords, 2);

			m_secondPass.setUniformValue("radiusScale", radius / inputSize.height() / 7);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			m_secondPass.disableAttributeArray("vertex");
			m_secondPass.disableAttributeArray("texCoord");
			m_secondPass.release();

			outputFbo->release();
		}

		cleanDirty();
	}

protected:
	Data< ImageWrapper > m_input, m_output;
	Data< PReal > m_radius;

	QSharedPointer<QOpenGLFramebufferObject> m_intermediaryFbo;
	QOpenGLShaderProgram m_firstPass, m_secondPass;
	GLfloat m_texCoords[8];
};

int ModifierImage_GaussianBlurClass = RegisterObject<ModifierImage_GaussianBlur>("Modifier/Image/Effects/Gaussian blur")
		.setDescription("Apply a gaussian blur to an image");

} // namespace Panda
