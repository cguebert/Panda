#pragma once

#include <panda/messaging.h>

#include <map>
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
	using UpdateMap = std::map<PandaObject*, ObjectsList>;

	void clear(PandaObject*);
	ObjectsList buildUpdateList(PandaObject& object);

	PandaDocument& m_document;
	UpdateMap m_updateMap;
	msg::Observer m_observer;
};

} // namespace panda

