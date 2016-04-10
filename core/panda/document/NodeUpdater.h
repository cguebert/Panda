#pragma once

#include <panda/messaging.h>

#include <map>
#include <set>
#include <vector>

namespace panda
{

class PandaDocument;
class PandaObject;
class DataNode;
class BaseData;

class PANDA_CORE_API NodeUpdater
{
public:
	NodeUpdater(PandaDocument& document);

	void updateObject(PandaObject& object);

private:
	using ObjectsList = std::vector<PandaObject*>;
	using ObjectsSet = std::set<PandaObject*>;
	using UpdateMap = std::map<PandaObject*, ObjectsList>;

	void clear(PandaObject*);
	const ObjectsList& getUpdateList(PandaObject& object);

	PandaDocument& m_document;
	UpdateMap m_updateMap;
	msg::Observer m_observer;
};

} // namespace panda

