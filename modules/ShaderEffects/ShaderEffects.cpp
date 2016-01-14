#include "ShaderEffects.h"

#include <panda/object/ObjectFactory.h>
#include <panda/graphics/ShaderProgram.h>

#include <iostream>

#include <QMatrix4x4>

namespace panda {

using graphics::Framebuffer;
using graphics::FramebufferFormat;
using graphics::ShaderProgram;
using graphics::Size;
using types::ImageWrapper;
using types::Point;

ShaderEffects::ShaderEffects(PandaDocument* doc, int nbPasses)
	: OGLObject(doc)
	, m_nbPasses(nbPasses)
	, m_input(initData("input", "The original image"))
	, m_output(initData("output", "Image created by the operation"))
{
	addInput(m_input);
	addOutput(m_output);

	m_texCoords[0*2+0] = 1; m_texCoords[0*2+1] = 1;
	m_texCoords[1*2+0] = 0; m_texCoords[1*2+1] = 1;
	m_texCoords[3*2+0] = 0; m_texCoords[3*2+1] = 0;
	m_texCoords[2*2+0] = 1; m_texCoords[2*2+1] = 0;
}

void ShaderEffects::update()
{
	const auto& inputVal = m_input.getValue();
	GLuint inputTexId = inputVal.getTextureId();
	if(inputTexId)
	{
		auto inputSize = inputVal.size();
		auto outputAcc = m_output.getAccessor();
		Framebuffer* outputFbo = outputAcc->getFbo();
		Framebuffer* intermediaryFbo = &m_intermediaryFbo;
		if(!outputFbo || outputFbo->size() != inputSize)
		{
			auto newFbo = Framebuffer(inputSize);
			outputAcc->setFbo(newFbo);
			outputFbo = &newFbo;

			if(m_nbPasses > 1)
			{
				m_intermediaryFbo = Framebuffer(inputSize);
				intermediaryFbo = &m_intermediaryFbo;
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
			Framebuffer* destFbo = nullptr;
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

			auto& program = preparePass(i);
			if(!program.isLinked())
				return;
			destFbo->bind();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindTexture(GL_TEXTURE_2D, texId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			program.setUniformValueMat4("MVP", mvp.constData());
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

panda::ModuleHandle shadersEffectsModule = REGISTER_MODULE
		.setDescription("Object to manipulate images using shaders.")
		.setLicense("GPL")
		.setVersion("1.0");

//****************************************************************************//

bool resizeFBO(Framebuffer& fbo, Size size, const FramebufferFormat& format)
{
	if(!fbo || fbo.size() != size)
	{
		fbo = Framebuffer(size, format);
		return true;
	}

	return false;
}

bool resizeFBO(types::ImageWrapper& img, Size size, const FramebufferFormat& format)
{
	auto fbo = img.getFbo();
	if(!fbo || img.size() != size)
	{
		auto newFbo = Framebuffer(size, format);
		img.setFbo(newFbo);
		return true;
	}

	return false;
}

void renderImage(Framebuffer& fbo, ShaderProgram& program)
{
	fbo.bind();

	auto size = fbo.size();
	glViewport(0, 0, size.width(), size.height());

	QMatrix4x4 mvp;
	mvp.ortho(0, size.width(), size.height(), 0, -10, 10);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat verts[8];
	verts[0*2+0] = size.width(); verts[0*2+1] = 0;
	verts[1*2+0] = 0;  verts[1*2+1] = 0;
	verts[3*2+0] = 0;  verts[3*2+1] = size.height();
	verts[2*2+0] = size.width(); verts[2*2+1] = size.height();

	const GLfloat texCoords[8] = {1, 1, 0, 1, 1, 0, 0, 0};

	program.bind();
	program.setUniformValueMat4("MVP", mvp.constData());

	program.enableAttributeArray("vertex");
	program.setAttributeArray("vertex", verts, 2);

	program.enableAttributeArray("texCoord");
	program.setAttributeArray("texCoord", texCoords, 2);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	program.disableAttributeArray("vertex");
	program.disableAttributeArray("texCoord");
	program.release();

	fbo.release();
}

bool bindTextures(ShaderProgram& program, const std::vector<GLuint>& texIds)
{
	program.bind();

	int nb = static_cast<int>(texIds.size());
	nb = std::min(nb, 32);
	for(int i = 0; i < nb; ++i)
	{
		std::string name = "tex" + std::to_string(i);
		int loc = program.uniformLocation(name.c_str());
		if(loc == -1)
		{
			std::cerr << "Shader program does not have a uniform named " << name << std::endl;
			return false;
		}

		program.setUniformValue(loc, i);
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texIds[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	glActiveTexture(GL_TEXTURE0);

	return true;
}

} // namespace Panda
