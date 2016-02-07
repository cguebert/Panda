#include <panda/object/OGLObject.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Point.h>

#include <panda/graphics/Buffer.h>
#include <panda/graphics/Framebuffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/graphics/VertexArrayObject.h>

#include <GL/glew.h>

namespace panda {

class ShaderEffects : public OGLObject
{
public:
	PANDA_CLASS(ShaderEffects, OGLObject)

	ShaderEffects(PandaDocument* doc);

	void initializeGL() override;

protected:
	void renderImage(graphics::Framebuffer& destFbo, graphics::ShaderProgram& program);

	template <typename... Args>
	void renderImage(graphics::Framebuffer& fbo, graphics::ShaderProgram& program, Args... args)
	{
		std::vector<GLuint> texIds { static_cast<GLuint>(args)... };
		if(bindTextures(program, texIds))
			renderImage(fbo, program);
	}

	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_verticesVBO, m_texCoordsVBO;
};

//****************************************************************************//

class ShaderEffectsMultiPass : public ShaderEffects
{
public:
	PANDA_CLASS(ShaderEffectsMultiPass, ShaderEffects)

	ShaderEffectsMultiPass(PandaDocument* doc, int nbPasses = 1);

	void initializeGL() override;

	/// Before we do any pass (get the Data values)
	virtual void prepareUpdate(graphics::Size size) = 0;

	/// Called before doing a new pass, set the program that is to be used for this pass
	virtual graphics::ShaderProgram& preparePass(int passId) = 0;

	void update() override;

protected:
	const int m_nbPasses;
	Data< types::ImageWrapper > m_input, m_output;

	graphics::Framebuffer m_intermediaryFbo;
};

// Returns true if it created a new fbo
bool resizeFBO(graphics::Framebuffer& fbo, graphics::Size size, const graphics::FramebufferFormat& format = graphics::FramebufferFormat());
bool resizeFBO(types::ImageWrapper& img, graphics::Size size, const graphics::FramebufferFormat& format = graphics::FramebufferFormat());

bool bindTextures(graphics::ShaderProgram& program, const std::vector<GLuint>& texIds);

} // namespace Panda
