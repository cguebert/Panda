#pragma once

#include <panda/core.h>

#include <functional>
#include <vector>

namespace panda {

class PandaObject;

namespace graph
{

	// Extract objects inside the groups
	std::vector<PandaObject*> PANDA_CORE_API expandObjectsList(std::vector<PandaObject*> objects);

	// Get the complete graph connect to these objects (not sorted)
	std::vector<PandaObject*> PANDA_CORE_API computeConnectedObjects(const std::vector<PandaObject*>& objects);

	// Execute a function for each PandaObject directly connected to the inputs or outputs of the given object
	using ObjectFunctor = std::function<void(PandaObject*)>;
	void PANDA_CORE_API forEachObjectOutput(PandaObject* object, ObjectFunctor func);
	void PANDA_CORE_API forEachObjectInput(PandaObject* object, ObjectFunctor func);

}

} // namespace panda
