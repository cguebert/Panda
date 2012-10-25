#include <panda/Layer.h>
#include <panda/Renderer.h>
#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include <QPainter>

#include <ui/GraphView.h>

namespace panda
{

Layer::Layer(PandaDocument *parent)
	: DockObject((QObject*)parent)
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
	PandaDocument* doc = dynamic_cast<PandaDocument*>(parent());
    if(doc)
    {
        QImage* editImage = image.beginEdit();
        *editImage = QImage(doc->getRenderSize(), QImage::Format_ARGB32);
        editImage->fill(QColor(0,0,0,0));
        QPainter painter(editImage);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);

		DockablesIterator iter = getDockablesIterator();
		iter.toBack();
		while(iter.hasPrevious())
		{
			Renderer* renderer = dynamic_cast<Renderer*>(iter.previous());
			if(renderer)
			{
				renderer->render(&painter);
				renderer->cleanDirty();
			}
		}

        image.endEdit();
    }
    this->cleanDirty();
}

void Layer::mergeLayer(QPainter* docPainter)
{
    this->updateIfDirty();

    double valOpacity = qBound(0.0, opacity.getValue(), 1.0);
    int valCompoMode = qBound(0, compositionMode.getValue(), 32);

    docPainter->save();
    docPainter->setOpacity(valOpacity);
    docPainter->setCompositionMode((QPainter::CompositionMode)valCompoMode);
    docPainter->drawImage(0, 0, image.getValue());
    docPainter->restore();
}

bool Layer::accepts(DockableObject* dockable) const
{
	return dynamic_cast<Renderer*>(dockable) != NULL;
}

int LayerClass = RegisterObject("Layer").setClass<Layer>().setDescription("Organize renderers and change opacity and the composition mode");

} // namespace panda
