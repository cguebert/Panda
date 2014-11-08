#include <panda/PandaObject.h>
#include <panda/types/ImageWrapper.h>

#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>

namespace panda {

using types::ImageWrapper;

class ShaderEffects : public PandaObject
{
public:
	PANDA_CLASS(ShaderEffects, PandaObject)

	ShaderEffects(PandaDocument* doc, int nbPasses = 1);

	/// Before we do any pass (get the Data values)
	virtual void prepareUpdate(QSize size) = 0;

	/// Called before doing a new pass, set the program that is to be used for this pass
	virtual QOpenGLShaderProgram& preparePass(int passId) = 0;

	void update();

protected:
	const int m_nbPasses;
	Data< ImageWrapper > m_input, m_output;

	QSharedPointer<QOpenGLFramebufferObject> m_intermediaryFbo;
	QVector< QSharedPointer<QOpenGLShaderProgram> > m_shaderPrograms;
	GLfloat m_texCoords[8];
};

// Returns true if it created a new fbo
bool resizeFBO(QSharedPointer<QOpenGLFramebufferObject>& fbo, QSize size);
void renderImage(QSharedPointer<QOpenGLFramebufferObject>& fbo, QOpenGLShaderProgram& program, GLuint texId);

} // namespace Panda
