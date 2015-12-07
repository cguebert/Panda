#include <panda/ObjectFactory.h>

#include <QOpenGLFunctions>

#include "ShaderEffects.h"

namespace panda {

using types::ImageWrapper;
using types::Point;

class TexturesOuterProduct : public OGLObject
{
public:
	PANDA_CLASS(TexturesOuterProduct, OGLObject)

	TexturesOuterProduct(PandaDocument* doc)
		: OGLObject(doc)
		, m_inputH(initData("horizontal", "Texture to be used horizontally"))
		, m_inputV(initData("vertical", "Texture to be used vertically"))
		, m_output(initData("output", "Texture created by the operation"))
		, m_size(initData(Point(64, 64), "size", "Size of the generated texture"))
	{
		addInput(m_inputH);
		addInput(m_inputV);
		addInput(m_size);

		addOutput(m_output);
	}

	void initializeGL()
	{
		m_shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/PT_noColor_Tex.v.glsl");
		m_shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/TextureOuterProduct.f.glsl");
		m_shaderProgram.link();
		m_shaderProgram.bind();

		m_shaderProgram.setUniformValue("tex0", 0);
		m_shaderProgram.setUniformValue("tex1", 1);

		m_shaderProgram.release();
	}

	void update()
	{
		unsigned int texH = m_inputH.getValue().getTextureId(), texV = m_inputV.getValue().getTextureId();
		if(!texH || !texV)
		{
			m_output.getAccessor()->clear();
			return;
		}

		auto outputAcc = m_output.getAccessor();
		Point psize = m_size.getValue();
		QSize size = QSize(psize.x, psize.y);
		QOpenGLFramebufferObject* outputFbo = outputAcc->getFbo();
		if(!outputFbo || outputAcc->getFbo()->size() != size)
		{
			auto newFbo = QSharedPointer<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(size));
			outputAcc->setFbo(newFbo);
			outputFbo = newFbo.data();
		}

		QOpenGLFunctions glFunctions(QOpenGLContext::currentContext());

		glFunctions.glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texH);
		glFunctions.glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texV);
		glFunctions.glActiveTexture(GL_TEXTURE0);

		renderImage(*outputFbo, m_shaderProgram);

		cleanDirty();
	}

protected:
	Data<ImageWrapper> m_inputH, m_inputV, m_output;
	Data<Point> m_size;

	QOpenGLShaderProgram m_shaderProgram;
	QSharedPointer<QOpenGLFramebufferObject> m_FBOs;
};

int TexturesOuterProductClass = RegisterObject<TexturesOuterProduct>("Modifier/Image/Textures outer product")
		.setDescription("Compute the outer product of two 1d textures (result is 2d)");

//****************************************************************************//

} // namespace Panda
