#include <panda/Layer.h>
#include <panda/Renderer.h>
#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include <QPainter>

#include <ui/GraphView.h>

namespace panda
{

void BaseLayer::updateLayer(PandaDocument* doc)
{
	Data<QImage>* dataImage = this->getImage();
	QImage* editImage = dataImage->beginEdit();
	*editImage = QImage(doc->getRenderSize(), QImage::Format_ARGB32);
	editImage->fill(QColor(0,0,0,0));
	QPainter painter(editImage);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::TextAntialiasing, true);

	QList<Renderer*> renderers = this->getRenderers();
	QListIterator<Renderer*> iter = QListIterator<Renderer*>(renderers);
	iter.toBack();
	while(iter.hasPrevious())
	{
		Renderer* renderer = iter.previous();
		renderer->render(&painter);
		renderer->cleanDirty();
	}

	dataImage->endEdit();
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
	: DockObject((QObject*)parent)
	, layerName(initData(&layerName, "name", "Name of this layer"))
	, image(initData(&image, "image", "Image created by the renderers connected to this layer"))
	, compositionMode(initData(&compositionMode, 0, "composition mode", "Defines how this layer is merged on top of the previous ones (see help for lsit of modes)"))
	, opacity(initData(&opacity, 1.0, "opacity", "Set the opacity of the layer"))
{
    addInput(&opacity);
    addInput(&compositionMode);

    addOutput((DataNode*)parent);
    addOutput(&image);

    image.setDisplayed(false);
}

void Layer::update()
{
	this->updateLayer(parentDocument);
    this->cleanDirty();
}

bool Layer::accepts(DockableObject* dockable) const
{
	return dynamic_cast<Renderer*>(dockable) != NULL;
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
	layerName.setValue(name);
}

int Layer::getCompositionMode() const
{
	return compositionMode.getValue();
}

void Layer::setCompositionMode(int mode)
{
	compositionMode.setValue(mode);
}

double Layer::getOpacity() const
{
	return opacity.getValue();
}

void Layer::setOpacity(double opa)
{
	opacity.setValue(qBound(0.0, opa, 1.0));
}

Data<QImage>* Layer::getImage()
{
	return &image;
}

void Layer::postCreate()
{
	int i = 1;
	PandaDocument::ObjectsIterator iter = parentDocument->getObjectsIterator();
	while(iter.hasNext())
	{
		PandaObject* obj = iter.next();
		if(dynamic_cast<Layer*>(obj) && obj!=this)
			++i;
	}

	layerName.setValue(tr("Layer #%1").arg(i));
}

int LayerClass = RegisterObject("Layer").setClass<Layer>().setDescription("Organize renderers and change opacity and the composition mode");

} // namespace panda
