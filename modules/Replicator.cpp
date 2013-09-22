#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/Layer.h>

#include <QImage>
#include <QPainter>

namespace panda {

class Replicator : public Layer
{
public:
	PANDA_CLASS(Replicator, Layer)

    Replicator(PandaDocument *doc)
        : Layer(doc)
        , iterating(false)
        , index(initData(&index, 0, "index", "0-based index of the current iteration"))
        , nbIterations(initData(&nbIterations, 1, "# iterations", "Number of times the objects have to be rendered"))
    {
        addInput(&nbIterations);

        index.setOutput(true); // But not really connecting
        index.setReadOnly(true);
    }

    void update()
    {
		PandaDocument* doc = dynamic_cast<PandaDocument*>(parent());
        if(doc)
        {
			auto editImage = image.getAccessor();
			editImage = QImage(doc->getRenderSize(), QImage::Format_ARGB32);
            editImage->fill(QColor(0,0,0,0));
			QPainter painter(&*editImage);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::TextAntialiasing, true);

			DockablesIterator iter = getDockablesIterator();
			int nb = nbIterations.getValue();
			for(int i=0; i<nb; ++i)
			{
                index.setValue(i);

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
            }
        }
        this->cleanDirty();
    }

protected:
    bool iterating;
    Data<int> index, nbIterations;
};

int ReplicatorClass = RegisterObject("Replicator").setClass<Replicator>().setName("Replicator").setDescription("Draw multiple times the objects");

} // namespace Panda

