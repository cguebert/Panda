#include <GL/glew.h>

#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Dockable.h>
#include <panda/object/Renderer.h>
#include <panda/object/Layer.h>

#include <panda/types/Rect.h>
#include <panda/types/ImageWrapper.h>

#include <panda/graphics/Framebuffer.h>

#ifdef PANDA_LOG_EVENTS
#include <panda/helper/UpdateLogger.h>
#endif

namespace panda {

using types::Point;
using types::ImageWrapper;

class GeneratorImage_CreateTexture : public DockObject, public BaseDrawTarget
{
public:
	PANDA_CLASS(GeneratorImage_CreateTexture, DockObject)

	GeneratorImage_CreateTexture(PandaDocument *doc)
		: DockObject(doc)
		, m_size(initData(Point(64, 64), "size", "Dimensions of the texture"))
		, m_image(initData("texture", "The image rendered"))
	{
		addInput(m_size);

		addOutput(m_image);
	}

	bool accepts(DockableObject* dockable) const
	{
		return dynamic_cast<Renderer*>(dockable) != nullptr;
	}

	QMatrix4x4& getMVPMatrix()
	{
		return m_mvpMatrix;
	}

	graphics::Size getLayerSize() const
	{
		Point size = m_size.getValue();
		return graphics::Size(std::max(1, (int)size.x), std::max(1, (int)size.y));
	}

	typedef std::vector<Renderer*> RenderersList;
	RenderersList getRenderers()
	{
		RenderersList renderers;
		for(auto dockable : getDockedObjects())
		{
			Renderer* renderer = dynamic_cast<Renderer*>(dockable);
			if(renderer)
				renderers.push_back(renderer);
		}

		return renderers;
	}

	void update()
	{
		const auto& renderers = getRenderers();
		for(const auto& renderer : renderers)
		{
			for(const auto* input : renderer->getInputDatas())
				input->updateIfDirty();
			renderer->updateIfDirty();
		}

		auto renderSize = getLayerSize();

		if(!m_renderFrameBuffer || m_renderFrameBuffer.size() != renderSize)
		{
			graphics::FramebufferFormat format;
			format.samples = 16;
			m_renderFrameBuffer = graphics::Framebuffer(renderSize, format);
			m_displayFrameBuffer = graphics::Framebuffer(renderSize);

			// Setting the image Data to the display Fbo
			m_image.getAccessor()->setFbo(m_displayFrameBuffer);
		}

		m_renderFrameBuffer.bind();

		glViewport(0, 0, renderSize.width(), renderSize.height());

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, renderSize.width(), renderSize.height(), 0, -10, 10);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		QMatrix4x4& mvp = getMVPMatrix();
		mvp = QMatrix4x4();
		mvp.ortho(0, renderSize.width(), renderSize.height(), 0, -10, 10);

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for(auto iter = renderers.rbegin(); iter != renderers.rend(); ++iter)
		{
			auto renderer = *iter;
#ifdef PANDA_LOG_EVENTS
			helper::ScopedEvent log(helper::event_render, renderer);
#endif
			renderer->render();
			renderer->cleanDirty();
		}

		glDisable(GL_BLEND);

		m_renderFrameBuffer.release();

		auto acc = m_image.getAccessor();
		panda::types::Rect area(0, 0, renderSize.width(), renderSize.height());
		graphics::Framebuffer::blitFramebuffer(*acc->getFbo(), area, m_renderFrameBuffer, area);

		cleanDirty();
	}

protected:
	Data<Point> m_size;
	Data<ImageWrapper> m_image;
	graphics::Framebuffer m_renderFrameBuffer, m_displayFrameBuffer;
	QMatrix4x4 m_mvpMatrix;
};

int GeneratorImage_CreateTextureClass = RegisterObject<GeneratorImage_CreateTexture>("Generator/Image/Create texture").setDescription("Create a texture and render on it");

} // namespace Panda
