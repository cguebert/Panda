#include <panda/Layer.h>
#include <panda/Renderer.h>
#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include <QOpenGLFramebufferObject>

#include <ui/GraphView.h>

#ifdef PANDA_LOG_EVENTS
#include <panda/helper/UpdateLogger.h>
#endif

namespace panda
{

void BaseLayer::updateLayer(PandaDocument* doc)
{
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

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	QList<Renderer*> renderers = getRenderers();
	QListIterator<Renderer*> iter = QListIterator<Renderer*>(renderers);
	iter.toBack();
	while(iter.hasPrevious())
	{
		Renderer* renderer = iter.previous();
#ifdef PANDA_LOG_EVENTS
		helper::ScopedEvent log(helper::event_render, renderer);
#endif
		renderer->render();
		renderer->cleanDirty();
	}
	renderFrameBuffer->release();

	QOpenGLFramebufferObject::blitFramebuffer(displayFrameBuffer.data(), renderFrameBuffer.data());
}

void BaseLayer::mergeLayer()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, displayFrameBuffer->texture());
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	GLfloat w = displayFrameBuffer->width(), h = displayFrameBuffer->height();
	GLfloat verts[8], texCoords[8];

	verts[0*2+0] = w; verts[0*2+1] = h;
	verts[1*2+0] = 0; verts[1*2+1] = h;
	verts[2*2+0] = w; verts[2*2+1] = 0;
	verts[3*2+0] = 0; verts[3*2+1] = 0;

	texCoords[0*2+0] = 1; texCoords[0*2+1] = 0;
	texCoords[1*2+0] = 0; texCoords[1*2+1] = 0;
	texCoords[2*2+0] = 1; texCoords[2*2+1] = 1;
	texCoords[3*2+0] = 0; texCoords[3*2+1] = 1;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, 0, verts );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glTexCoordPointer( 2, GL_FLOAT, 0, texCoords );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

//*************************************************************************//

Layer::Layer(PandaDocument *parent)
	: DockObject(parent)
	, layerName(initData(&layerName, "name", "Name of this layer"))
	, image(initData(&image, "image", "Image created by the renderers connected to this layer"))
	, compositionMode(initData(&compositionMode, 0, "composition mode", "Defines how this layer is merged on top of the previous ones (see help for list of modes)"))
	, opacity(initData(&opacity, 1.0, "opacity", "Set the opacity of the layer"))
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
	updateLayer(parentDocument);
	cleanDirty();
}

bool Layer::accepts(DockableObject* dockable) const
{
	return dynamic_cast<Renderer*>(dockable) != nullptr;
}

QList<Renderer*> Layer::getRenderers()
{
	QList<Renderer*> renderers;
	DockablesIterator iter = getDockablesIterator();
	while(iter.hasNext())
	{
		Renderer* renderer = dynamic_cast<Renderer*>(iter.next());
		if(renderer)
			renderers.append(renderer);
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

double Layer::getOpacity() const
{
	return opacity.getValue();
}

void Layer::setOpacity(double opa)
{
	double tmp = qBound(0.0, opa, 1.0);
	if(opacity.getValue() != opa)
		opacity.setValue(tmp);
}

Data<types::ImageWrapper>* Layer::getImage()
{
	return &image;
}

void Layer::postCreate()
{
	int i = 1;
	for(auto obj : parentDocument->getObjects())
	{
		if(dynamic_cast<Layer*>(obj) && obj!=this)
			++i;
	}

	layerName.setValue(tr("Layer #%1").arg(i));
}

int LayerClass = RegisterObject<Layer>("Layer").setDescription("Organize renderers and change opacity and the composition mode");

} // namespace panda
