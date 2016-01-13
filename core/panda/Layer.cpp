#include <GL/glew.h>

#include <panda/Layer.h>
#include <panda/Renderer.h>
#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>
#include <panda/helper/algorithm.h>

#include <panda/graphics/Framebuffer.h>

#include <panda/command/MoveLayerCommand.h>

#ifdef PANDA_LOG_EVENTS
#include <panda/helper/UpdateLogger.h>
#endif

namespace panda
{

void BaseLayer::updateLayer(PandaDocument* doc)
{
	// Bugfix : we update the input Datas of the renderers before setting the opengl context
	//  as getting the image from an ImageWrapper can screw it up
	for(const auto& renderer : getRenderers())
	{
		for(const auto* input : renderer->getInputDatas())
			input->updateIfDirty();
		renderer->updateIfDirty();
	}

#ifdef PANDA_LOG_EVENTS
	{
		helper::ScopedEvent log1("prepareLayer");
#endif

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

	m_renderFrameBuffer->bind();

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
	glClear(GL_COLOR_BUFFER_BIT);

#ifdef PANDA_LOG_EVENTS
	}
#endif

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	iterateRenderers();

	glDisable(GL_BLEND);

#ifdef PANDA_LOG_EVENTS
	helper::ScopedEvent log2("blitFramebuffer");
#endif

	m_renderFrameBuffer->release();

	graphics::Framebuffer::blitFramebuffer(*m_displayFrameBuffer, *m_renderFrameBuffer);
}

void BaseLayer::iterateRenderers()
{
	const auto& renderers = getRenderers();
	for(auto iter = renderers.rbegin(); iter != renderers.rend(); ++iter)
	{
		auto renderer = *iter;
#ifdef PANDA_LOG_EVENTS
		helper::ScopedEvent log(helper::event_render, renderer);
#endif
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
	m_name = "Default layer"; // Any Layer created by the Factory will have its name changed

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
}

void Layer::update()
{
	updateLayer(m_parentDocument);
	cleanDirty();
}

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
	for(auto obj : m_parentDocument->getObjects())
	{
		if(dynamic_cast<Layer*>(obj.get()) && obj.get() != this)
			++i;
	}

	m_layerName.setValue("Layer #" + std::to_string(i));
}

void Layer::removedFromDocument()
{
	DockObject::removedFromDocument();

	if(m_parentDocument->isInCommandMacro())
		m_parentDocument->addCommand(std::make_shared<MoveLayerCommand>(m_parentDocument, this, 0));
}

graphics::Size Layer::getLayerSize() const
{
	return m_parentDocument->getRenderSize();
}

int LayerClass = RegisterObject<Layer>("Layer").setDescription("Organize renderers and change opacity and the composition mode");

} // namespace panda
