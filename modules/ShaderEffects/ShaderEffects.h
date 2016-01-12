#include <panda/OGLObject.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Point.h>

#include <panda/graphics/Framebuffer.h>
#include <panda/graphics/ShaderProgram.h>

#include <GL/glew.h>

namespace panda {

namespace graphics
{
class ShaderProgram;
}

class ShaderEffects : public OGLObject
{
public:
	PANDA_CLASS(ShaderEffects, OGLObject)

	ShaderEffects(PandaDocument* doc, int nbPasses = 1);

	/// Before we do any pass (get the Data values)
	virtual void prepareUpdate(graphics::Size size) = 0;

	/// Called before doing a new pass, set the program that is to be used for this pass
	virtual graphics::ShaderProgram& preparePass(int passId) = 0;

	void update();

protected:
	const int m_nbPasses;
	Data< types::ImageWrapper > m_input, m_output;

	graphics::Framebuffer m_intermediaryFbo;
	float m_texCoords[8];
};

// Returns true if it created a new fbo
bool resizeFBO(graphics::Framebuffer& fbo, graphics::Size size, const graphics::FramebufferFormat& format = graphics::FramebufferFormat());
bool resizeFBO(types::ImageWrapper& img, graphics::Size size, const graphics::FramebufferFormat& format = graphics::FramebufferFormat());
void renderImage(graphics::Framebuffer& fbo, graphics::ShaderProgram& program);

bool bindTextures(graphics::ShaderProgram& program, const std::vector<GLuint>& texIds);

template <typename... Args>
void renderImage(graphics::Framebuffer& fbo, graphics::ShaderProgram& program, Args... args)
{
	std::vector<GLuint> texIds { static_cast<GLuint>(args)... };
	if(bindTextures(program, texIds))
		renderImage(fbo, program);
}

} // namespace Panda
