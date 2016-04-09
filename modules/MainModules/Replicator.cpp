#include <panda/PandaDocument.h>
#include <panda/helper/UpdateLogger.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/object/Layer.h>

namespace panda {

class Replicator : public Layer
{
public:
	PANDA_CLASS(Replicator, Layer)

	Replicator(PandaDocument *doc)
		: Layer(doc)
		, index(initData(0, "index", "0-based index of the current iteration"))
		, nbIterations(initData(1, "# iterations", "Number of times the objects have to be rendered"))
	{
		addInput(nbIterations);

		index.setOutput(true); // But not really connecting
		index.setReadOnly(true);

		setLaterUpdate(true);
	}

	void iterateRenderers()
	{
		int nb = nbIterations.getValue();
		for(int i=0; i<nb; ++i)
		{
			parentDocument()->setDataDirty(&index); // Preset the outputs as dirty
			index.setValue(i);
			parentDocument()->setDataReady(&index); // Launch the computation
			parentDocument()->waitForOtherTasksToFinish(); // Wait for the end of the computation

			auto& dockables = getDockedObjects();
			for(auto it=dockables.rbegin(); it!=dockables.rend(); ++it)
			{
				Renderer* renderer = dynamic_cast<Renderer*>(*it);
				if(renderer)
				{
					helper::ScopedEvent log(helper::event_render, renderer);
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

