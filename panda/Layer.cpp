#include <panda/Layer.h>
#include <panda/Renderer.h>
#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include <QPainter>

#include <ui/GraphView.h>
#include <ui/SimpleDataWidget.h>

namespace panda
{

void BaseLayer::updateLayer(PandaDocument* doc)
{
	auto editImage = this->getImage()->getAccessor();
	editImage = QImage(doc->getRenderSize(), QImage::Format_ARGB32);
	editImage->fill(QColor(0,0,0,0));
	QPainter painter(&*editImage);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::TextAntialiasing, true);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

	QList<Renderer*> renderers = this->getRenderers();
	QListIterator<Renderer*> iter = QListIterator<Renderer*>(renderers);
	iter.toBack();
	while(iter.hasPrevious())
	{
		Renderer* renderer = iter.previous();
		renderer->render(&painter);
		renderer->cleanDirty();
	}
}

void BaseLayer::mergeLayer(QPainter* docPainter)
{
	double valOpacity = qBound(0.0, this->getOpacity(), 1.0);
	int valCompoMode = qBound(0, this->getCompositionMode(), 32);

	docPainter->save();
	docPainter->setOpacity(valOpacity);
	docPainter->setCompositionMode((QPainter::CompositionMode)valCompoMode);
	docPainter->drawImage(0, 0, this->getImage()->getValue());
	docPainter->restore();
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

	compositionMode.setWidget("enum_LayerComposition");

	addOutput((DataNode*)parent);
	addOutput(&image);
}

void Layer::update()
{
	this->updateLayer(parentDocument);
	this->cleanDirty();
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

Data<QImage>* Layer::getImage()
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

//*************************************************************************//

const char* Layer::compositionModes[] = { "SourceOver", "DestinationOver", "Clear", "Source", "Destination",
										  "SourceIn", "DestinationIn", "DestinationOut", "SourceAtop", "DestinationAtop",
										  "Xor", "Plus", "Multiply", "Screen", "Overlay", "Darken", "Lighten",
										  "ColorDodge", "ColorBurn", "HardLight", "SoftLight", "Difference", "Exclusion",
										  "SourceOrDestination", "SourceAndDestination", "SourceXorDestination",
										  "NotSourceAndNotDestination", "NotSourceOrNotDestination", "NotSourceXorDestination",
										  "NoSource", "NoSourceAndDestination", "SourceAndNotDestination" };

Creator<DataWidgetFactory, SimpleDataWidget<int, EnumDataWidget<32, Layer::compositionModes> > > DWClass_enum_layer_composition("enum_LayerComposition",true);

} // namespace panda
