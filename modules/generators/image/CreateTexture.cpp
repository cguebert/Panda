#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Dockable.h>
#include <panda/Renderer.h>
#include <panda/Layer.h>

#include <panda/types/Point.h>
#include <panda/types/ImageWrapper.h>

#include <QOpenGLFramebufferObject>

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
		, width(initData(&width, 64, "width", "Width of the texture"))
		, height(initData(&height, 64, "height", "Height of the texture"))
		, image(initData(&image, "texture", "The image rendered"))
	{
		addInput(&width);
		addInput(&height);

		addOutput(&image);
	}

	bool accepts(DockableObject* dockable) const
	{
		return dynamic_cast<Renderer*>(dockable) != nullptr;
	}

	QMatrix4x4& getMVPMatrix()
	{
		return mvpMatrix;
	}

	QSize getLayerSize() const
	{
		return QSize(std::max(1, width.getValue()), std::max(1, height.getValue()));
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

		QSize renderSize = getLayerSize();

		if(!renderFrameBuffer || renderFrameBuffer->size() != renderSize)
		{
			QOpenGLFramebufferObjectFormat fmt;
			fmt.setSamples(16);
			renderFrameBuffer.reset(new QOpenGLFramebufferObject(renderSize, fmt));
			displayFrameBuffer.reset(new QOpenGLFramebufferObject(renderSize));

			// Setting the image Data to the display Fbo
			image.getAccessor()->setFbo(displayFrameBuffer);
		}

		renderFrameBuffer->bind();

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

		renderFrameBuffer->release();

		QOpenGLFramebufferObject::blitFramebuffer(displayFrameBuffer.data(), renderFrameBuffer.data());

		cleanDirty();
	}

protected:
	Data<int> width, height;
	Data<ImageWrapper> image;
	QSharedPointer<QOpenGLFramebufferObject> renderFrameBuffer, displayFrameBuffer;
	QMatrix4x4 mvpMatrix;
};

int GeneratorImage_CreateTextureClass = RegisterObject<GeneratorImage_CreateTexture>("Generator/Image/Create texture").setDescription("Create a texture and render on it");

} // namespace Panda
