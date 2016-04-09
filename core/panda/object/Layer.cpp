#include <GL/glew.h>

#include <panda/PandaDocument.h>
#include <panda/command/MoveLayerCommand.h>
#include <panda/document/NodeUpdater.h>
#include <panda/graphics/Framebuffer.h>
#include <panda/helper/algorithm.h>
#include <panda/helper/UpdateLogger.h>
#include <panda/object/Layer.h>
#include <panda/object/Renderer.h>
#include <panda/object/ObjectFactory.h>

namespace panda
{

void BaseLayer::updateLayer(PandaDocument* doc)
{
	// Bugfix : we update the input Datas of the renderers before setting the viewport
	//  as getting the image from an ImageWrapper can screw it up
	auto& nodeUpdater = doc->getNodeUpdater();
	for(const auto& renderer : getRenderers())
		nodeUpdater.updateObject(*renderer);

	{
		helper::ScopedEvent log1("prepareLayer");

		auto renderSize = doc->getRenderSize();
		auto output = getImage()->getAccessor(); // At the end of the function, the accessor destructor calls cleanDirty & setDirtyOutputs
		if(!m_renderFrameBuffer || m_renderFrameBuffer->size() != renderSize)
		{
			graphics::FramebufferFormat format;
			format.samples = 16;
			m_renderFrameBuffer = std::make_shared<graphics::Framebuffer>(renderSize, format);
			m_displayFrameBuffer = std::make_shared<graphics::Framebuffer>(renderSize);

			// Setting the image Data to the display Fbo
			output->setFbo(*m_displayFrameBuffer);
		}

		glViewport(0, 0, renderSize.width(), renderSize.height());
		m_renderFrameBuffer->bind();
	
		auto& mvp = getMVPMatrix();
		mvp.ortho(0, static_cast<float>(renderSize.width()), static_cast<float>(renderSize.height()), 0, -10.f, 10.f);

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	iterateRenderers();

	glDisable(GL_BLEND);

	m_renderFrameBuffer->release();

	helper::ScopedEvent log2("blitFramebuffer");
	graphics::Framebuffer::blitFramebuffer(*m_displayFrameBuffer, *m_renderFrameBuffer);
}

void BaseLayer::iterateRenderers()
{
	const auto& renderers = getRenderers();
	for(auto iter = renderers.rbegin(); iter != renderers.rend(); ++iter)
	{
		auto renderer = *iter;
		helper::ScopedEvent log(helper::event_render, renderer);
		renderer->render();
		renderer->cleanDirty();
	}
}

unsigned int BaseLayer::getTextureId() const
{
	return m_displayFrameBuffer->texture();
}

//****************************************************************************//

Layer::Layer(PandaDocument* parent)
	: DockObject(parent)
	, m_layerName(initData("name", "Name of this layer"))
	, m_image(initData("image", "Image created by the renderers connected to this layer"))
	, m_compositionMode(initData(0, "composition mode", "Defines how this layer is merged on top of the previous ones (see help for list of modes)"))
	, m_opacity(initData(1.0, "opacity", "Set the opacity of the layer"))
{
	addInput(m_layerName);
	addInput(m_opacity);
	addInput(m_compositionMode);

	// 24 possible modes
	m_compositionMode.setWidget("enum");
	m_compositionMode.setWidgetData("SourceOver;DestinationOver;Clear;Source;Destination;"
								  "SourceIn;DestinationIn;SourceOut;DestinationOut;SourceAtop;DestinationAtop;"
								  "Xor;Plus;Multiply;Screen;Overlay;Darken;Lighten;"
								  "ColorDodge;ColorBurn;HardLight;SoftLight;Difference;Exclusion");

	m_opacity.setWidget("slider");
	m_opacity.setWidgetData("0 1 0.01");

	addOutput(*parent);
	addOutput(m_image);

	setUpdateOnMainThread(true);
}

void Layer::update()
{
	updateLayer(parentDocument());}

bool Layer::accepts(DockableObject* dockable) const
{
	return dynamic_cast<Renderer*>(dockable) != nullptr;
}

BaseLayer::RenderersList Layer::getRenderers()
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

void Layer::postCreate()
{
	PandaObject::postCreate();
	int i = 1;
	for(auto& obj : parentDocument()->getObjects())
	{
		if(dynamic_cast<Layer*>(obj.get()) && obj.get() != this)
			++i;
	}

	m_layerName.setValue("Layer #" + std::to_string(i));
}

void Layer::removedFromDocument()
{
	// Bugfix: deconnecting the ImageViewport if there is one
	auto outputs = m_image.getOutputs();
	for (const auto output : outputs)
		m_image.removeOutput(*output);

	DockObject::removedFromDocument();

	auto& undoStack = parentDocument()->getUndoStack();
	if(undoStack.isInCommandMacro())
		undoStack.push(std::make_shared<MoveLayerCommand>(parentDocument(), this, 0));
}

graphics::Size Layer::getLayerSize() const
{
	return parentDocument()->getRenderSize();
}

int LayerClass = RegisterObject<Layer>("Layer").setDescription("Organize renderers and change opacity and the composition mode");

} // namespace panda
