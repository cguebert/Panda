#include <panda/ObjectFactory.h>
#include "ShaderEffects.h"

namespace panda {

using types::ImageWrapper;

ShaderEffects::ShaderEffects(PandaDocument* doc, int nbPasses)
	: PandaObject(doc)
	, m_nbPasses(nbPasses)
	, m_input(initData(&m_input, "input", "The original image"))
	, m_output(initData(&m_output, "output", "Image created by the operation"))
{
	addInput(&m_input);
	addOutput(&m_output);

	m_texCoords[0*2+0] = 1; m_texCoords[0*2+1] = 1;
	m_texCoords[1*2+0] = 0; m_texCoords[1*2+1] = 1;
	m_texCoords[3*2+0] = 0; m_texCoords[3*2+1] = 0;
	m_texCoords[2*2+0] = 1; m_texCoords[2*2+1] = 0;
}

void ShaderEffects::postCreate()
{
	PandaObject::postCreate();

	for(int i=0; i<m_nbPasses; ++i)
	{
		auto program = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram());
		initShaderProgram(i, *program.data());
		m_shaderPrograms.push_back(program);
	}
}

void ShaderEffects::update()
{
	const auto& inputVal = m_input.getValue();
	GLuint inputTexId = inputVal.getTextureId();
	if(inputTexId)
	{
		QSize inputSize = inputVal.size();
		auto outputAcc = m_output.getAccessor();
		QOpenGLFramebufferObject* outputFbo = outputAcc->getFbo();
		QOpenGLFramebufferObject* intermediaryFbo = m_intermediaryFbo.data();
		if(!outputFbo || outputFbo->size() != inputSize)
		{
			auto newFbo = QSharedPointer<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(inputSize));
			outputAcc->setFbo(newFbo);
			outputFbo = newFbo.data();

			if(m_nbPasses > 1)
			{
				m_intermediaryFbo.reset(new QOpenGLFramebufferObject(inputSize));
				intermediaryFbo = m_intermediaryFbo.data();
			}
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

		prepareUpdate(inputSize);

		for(int i=0; i<m_nbPasses; ++i)
		{
			QOpenGLFramebufferObject* destFbo = nullptr;
			GLuint texId = 0;

			if(m_nbPasses % 2) // odd # of passes, go first to output, then ping-pong
			{
				if(!i)			{ texId = inputTexId;					destFbo = outputFbo; }
				else if(i % 2)	{ texId = outputFbo->texture();			destFbo = intermediaryFbo; }
				else			{ texId = intermediaryFbo->texture();	destFbo = outputFbo; }
			}
			else // even: go to mid, then ping-pong
			{
				if(!i)			{ texId = inputTexId;					destFbo = intermediaryFbo; }
				else if(i % 2)	{ texId = intermediaryFbo->texture();	destFbo = outputFbo; }
				else			{ texId = outputFbo->texture();			destFbo = intermediaryFbo; }
			}

			auto& program = *m_shaderPrograms[i].data();
			program.bind();
			destFbo->bind();

			preparePass(i, program);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindTexture(GL_TEXTURE_2D, texId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			program.setUniformValue("MVP", mvp);
			program.setUniformValue("tex0", 0);

			program.enableAttributeArray("vertex");
			program.setAttributeArray("vertex", verts, 2);

			program.enableAttributeArray("texCoord");
			program.setAttributeArray("texCoord", m_texCoords, 2);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			program.disableAttributeArray("vertex");
			program.disableAttributeArray("texCoord");
			program.release();

			destFbo->release();
		}
	}
}

panda::ModuleHandle shadersEffectsModule = panda::RegisterModule("ShaderEffects")
		.setDescription("Object to manipulate images using shaders.")
		.setLicense("GPL")
		.setVersion("1.0");


} // namespace Panda
