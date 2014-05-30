#include <panda/Layer.h>
#include <panda/Renderer.h>
#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include <QOpenGLFramebufferObject>
#include <QCoreApplication>

#include <ui/GraphView.h>

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

	QSize renderSize = doc->getRenderSize();
	if(!renderFrameBuffer || renderFrameBuffer->size() != renderSize)
	{
		QOpenGLFramebufferObjectFormat fmt;
		fmt.setSamples(16);
		renderFrameBuffer.reset(new QOpenGLFramebufferObject(renderSize, fmt));
		displayFrameBuffer.reset(new QOpenGLFramebufferObject(renderSize));

		// Setting the image Data to the display Fbo
		getImage()->getAccessor()->setFbo(displayFrameBuffer);
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
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef PANDA_LOG_EVENTS
	}
#endif

	iterateRenderers();

#ifdef PANDA_LOG_EVENTS
	helper::ScopedEvent log2("blitFramebuffer");
#endif

	glDisable(GL_BLEND);

	renderFrameBuffer->release();

	QOpenGLFramebufferObject::blitFramebuffer(displayFrameBuffer.data(), renderFrameBuffer.data());
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
	return displayFrameBuffer->texture();
}

//*************************************************************************//

Layer::Layer(PandaDocument *parent)
	: DockObject(parent)
	, layerName(initData(&layerName, "name", "Name of this layer"))
	, image(initData(&image, "image", "Image created by the renderers connected to this layer"))
	, compositionMode(initData(&compositionMode, 0, "composition mode", "Defines how this layer is merged on top of the previous ones (see help for list of modes)"))
	, opacity(initData(&opacity, (PReal)1.0, "opacity", "Set the opacity of the layer"))
{
	addInput(&layerName);
	addInput(&opacity);
	addInput(&compositionMode);

	compositionMode.setWidget("enum");
	compositionMode.setWidgetData("SourceOver;DestinationOver;Clear;Source;Destination"
								  ";SourceIn;DestinationIn;DestinationOut;SourceAtop;DestinationAtop"
								  ";Xor;Plus;Multiply;Screen;Overlay;Darken;Lighten"
								  ";ColorDodge;ColorBurn;HardLight;SoftLight;Difference;Exclusion"
								  ";SourceOrDestination;SourceAndDestination;SourceXorDestination"
								  ";NotSourceAndNotDestination;NotSourceOrNotDestination;NotSourceXorDestination"
								  ";NoSource;NoSourceAndDestination;SourceAndNotDestination");

	opacity.setWidget("slider");
	opacity.setWidgetData("0 1 0.01");

	addOutput((DataNode*)parent);
	addOutput(&image);
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
	for(auto dockable : getDockables())
	{
		Renderer* renderer = dynamic_cast<Renderer*>(dockable);
		if(renderer)
			renderers.push_back(renderer);
	}

	return renderers;
}

QString Layer::getLayerName() const
{
	return layerName.getValue();
}

void Layer::setLayerName(QString name)
{
	if(layerName.getValue() != name)
		layerName.setValue(name);
}

int Layer::getCompositionMode() const
{
	return compositionMode.getValue();
}

void Layer::setCompositionMode(int mode)
{
	if(compositionMode.getValue() != mode)
		compositionMode.setValue(mode);
}

PReal Layer::getOpacity() const
{
	return opacity.getValue();
}

void Layer::setOpacity(PReal opa)
{
	PReal tmp = qBound<PReal>(0.0, opa, 1.0);
	if(opacity.getValue() != opa)
		opacity.setValue(tmp);
}

Data<types::ImageWrapper>* Layer::getImage()
{
	return &image;
}

QMatrix4x4& Layer::getMVPMatrix()
{
	return mvpMatrix;
}

void Layer::postCreate()
{
	int i = 1;
	for(auto obj : m_parentDocument->getObjects())
	{
		if(dynamic_cast<Layer*>(obj.data()) && obj!=this)
			++i;
	}

	QString text = QCoreApplication::translate("Layer", "Layer #%1");
	layerName.setValue(text.arg(i));
}

int LayerClass = RegisterObject<Layer>("Layer").setDescription("Organize renderers and change opacity and the composition mode");

} // namespace panda
