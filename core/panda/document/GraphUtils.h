#pragma once

#include <panda/core.h>

#include <functional>
#include <vector>

namespace panda {

class BaseData;
class DataNode;
class PandaObject;

namespace graph
{

	// Extract objects inside the groups
	std::vector<PandaObject*> PANDA_CORE_API expandObjectsList(std::vector<PandaObject*> objects);

	// Get the complete graph connected to these objects (not sorted)
	std::vector<PandaObject*> PANDA_CORE_API computeConnectedObjects(const std::vector<PandaObject*>& objects);

	// Execute a function for each PandaObject directly connected to the inputs or outputs of the given object
	using ObjectFunctor = std::function<void(PandaObject*)>;
	void PANDA_CORE_API forEachObjectOutput(PandaObject* object, ObjectFunctor func);
	void PANDA_CORE_API forEachObjectInput(PandaObject* object, ObjectFunctor func);

	// Get all connected input or ouput Datas. KeepRecursive to false removes the created datas of ListBuffer and ListLoop, for example.
	std::vector<DataNode*> PANDA_CORE_API computeConnectedInputNodes(DataNode* originNode, bool keepRecursive = true);
	std::vector<DataNode*> PANDA_CORE_API computeConnectedOutputNodes(DataNode* originNode, bool keepRecursive = true);

	std::vector<BaseData*> PANDA_CORE_API extractDatas(const std::vector<DataNode*>& nodes);
	std::vector<PandaObject*> PANDA_CORE_API extractObjects(const std::vector<DataNode*>& nodes);
}

} // namespace panda
