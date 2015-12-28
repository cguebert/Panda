#include <panda/OGLObject.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Point.h>

#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>

namespace panda {

class ShaderEffects : public OGLObject
{
public:
	PANDA_CLASS(ShaderEffects, OGLObject)

	ShaderEffects(PandaDocument* doc, int nbPasses = 1);

	/// Before we do any pass (get the Data values)
	virtual void prepareUpdate(QSize size) = 0;

	/// Called before doing a new pass, set the program that is to be used for this pass
	virtual QOpenGLShaderProgram& preparePass(int passId) = 0;

	void update();

protected:
	const int m_nbPasses;
	Data< types::ImageWrapper > m_input, m_output;

	std::shared_ptr<QOpenGLFramebufferObject> m_intermediaryFbo;
	std::vector< std::shared_ptr<QOpenGLShaderProgram> > m_shaderPrograms;
	GLfloat m_texCoords[8];
};

// Returns true if it created a new fbo
bool resizeFBO(std::shared_ptr<QOpenGLFramebufferObject>& fbo, QSize size, const QOpenGLFramebufferObjectFormat& format = QOpenGLFramebufferObjectFormat());
bool resizeFBO(types::ImageWrapper& img, QSize size, const QOpenGLFramebufferObjectFormat& format = QOpenGLFramebufferObjectFormat());
void renderImage(QOpenGLFramebufferObject& fbo, QOpenGLShaderProgram& program);

bool bindTextures(QOpenGLShaderProgram& program, const std::vector<GLuint>& texIds);

template <typename... Args>
void renderImage(QOpenGLFramebufferObject& fbo, QOpenGLShaderProgram& program, Args... args)
{
	std::vector<GLuint> texIds { static_cast<GLuint>(args)... };
	if(bindTextures(program, texIds))
		renderImage(fbo, program);
}

} // namespace Panda
