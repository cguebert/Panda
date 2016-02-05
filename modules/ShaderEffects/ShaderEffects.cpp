#include "ShaderEffects.h"

#include <panda/object/ObjectFactory.h>
#include <panda/graphics/Mat4x4.h>
#include <panda/graphics/ShaderProgram.h>

#include <algorithm>
#include <iostream>

namespace panda {

using graphics::Framebuffer;
using graphics::FramebufferFormat;
using graphics::ShaderProgram;
using graphics::Size;
using types::ImageWrapper;
using types::Point;

ShaderEffects::ShaderEffects(PandaDocument* doc)
	: OGLObject(doc)
{ }

void ShaderEffects::initializeGL()
{
	m_VAO.create();
	m_VAO.bind();

	m_verticesVBO.create();
	m_verticesVBO.bind();
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(0);

	m_texCoordsVBO.create();
	m_texCoordsVBO.bind();
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(1);

	std::vector<GLfloat> texCoords = { 0, 1, 1, 1, 0, 0, 1, 0 };
	m_texCoordsVBO.write(texCoords);

	m_VAO.release();
}

void ShaderEffects::renderImage(Framebuffer& fbo, ShaderProgram& program)
{
	fbo.bind();

	auto size = fbo.size();
	GLfloat w = static_cast<float>(size.width()), h = static_cast<float>(size.height());
	std::vector<GLfloat> verts = { 0, 0, w, 0, 0, h, w, h };
	m_verticesVBO.bind();
	m_verticesVBO.write(verts);

	glViewport(0, 0, size.width(), size.height());
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	graphics::Mat4x4 mvp;
	mvp.ortho(0, w, h, 0, -10.f, 10.f);

	program.bind();
	program.setUniformValueMat4("MVP", mvp.data());

	m_VAO.bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_VAO.release();

	program.release();
	fbo.release();
}

//****************************************************************************//

ShaderEffectsMultiPass::ShaderEffectsMultiPass(PandaDocument* doc, int nbPasses)
	: ShaderEffects(doc)
	, m_nbPasses(nbPasses)
	, m_input(initData("input", "The original image"))
	, m_output(initData("output", "Image created by the operation"))
{
	addInput(m_input);
	addOutput(m_output);
}

void ShaderEffectsMultiPass::initializeGL()
{
	ShaderEffects::initializeGL();
}

void ShaderEffectsMultiPass::update()
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

		glViewport(0, 0, inputSize.width(), inputSize.height());
		glClearColor(0, 0, 0, 0);

		GLfloat w = static_cast<float>(inputSize.width()), h = static_cast<float>(inputSize.height());
		std::vector<GLfloat> verts = { 0, 0, w, 0, 0, h, w, h };
		m_verticesVBO.bind();
		m_verticesVBO.write(verts);

		graphics::Mat4x4 mvp;
		mvp.ortho(0, w, h, 0, -10.f, 10.f);

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

			program.setUniformValueMat4("MVP", mvp.data());
			program.setUniformValue("tex0", 0);

			m_VAO.bind();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			m_VAO.release();
			program.release();
			destFbo->release();
		}
	}
}

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
	}

	glActiveTexture(GL_TEXTURE0);

	return true;
}

//****************************************************************************//

panda::ModuleHandle shadersEffectsModule = REGISTER_MODULE
		.setDescription("Object to manipulate images using shaders.")
		.setLicense("GPL")
		.setVersion("1.0");

} // namespace Panda
