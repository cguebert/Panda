#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/Layer.h>

#include <QPainter>

#ifdef PANDA_LOG_EVENTS
#include <panda/helper/UpdateLogger.h>
#endif

namespace panda {

class Replicator : public Layer
{
public:
	PANDA_CLASS(Replicator, Layer)

	Replicator(PandaDocument *doc)
		: Layer(doc)
		, index(initData(&index, 0, "index", "0-based index of the current iteration"))
		, nbIterations(initData(&nbIterations, 1, "# iterations", "Number of times the objects have to be rendered"))
	{
		addInput(&nbIterations);

		index.setOutput(true); // But not really connecting
		index.setReadOnly(true);

		m_laterUpdate = true;
	}

	void iterateRenderers()
	{
		int nb = nbIterations.getValue();
		for(int i=0; i<nb; ++i)
		{
			m_parentDocument->setDataDirty(&index); // Preset the outputs as dirty
			index.setValue(i);
			m_parentDocument->setDataReady(&index); // Launch the computation
			m_parentDocument->waitForOtherTasksToFinish(); // Wait for the end of the computation

			auto dockables = getDockables();
			for(int i=dockables.size()-1; i>=0; --i)
			{
				Renderer* renderer = dynamic_cast<Renderer*>(dockables[i]);
				if(renderer)
				{
#ifdef PANDA_LOG_EVENTS
					helper::ScopedEvent log(helper::event_render, renderer);
#endif
					renderer->updateIfDirty();
					renderer->render();
					renderer->cleanDirty();
				}
			}
		}
	}

protected:
	Data<int> index, nbIterations;
};

int ReplicatorClass = RegisterObject<Replicator>("Replicator").setDescription("Draw multiple times the objects");

} // namespace Panda

