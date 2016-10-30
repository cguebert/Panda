#pragma once

#include <panda/object/ObjectAddons.h>
#include <panda/types/Point.h>
#include <panda/messaging.h>

namespace panda
{

namespace graphview
{

class PANDA_CORE_API ObjectPositionAddon : public BaseObjectAddon
{
public:
	ObjectPositionAddon(PandaObject& object);

	static void setDefinition(ObjectAddonNodeDefinition& nodeDefinition);

	void save(ObjectAddonNode& node) override;
	void load(const ObjectAddonNode& node) override;

	bool isSet() const;

	types::Point getPosition() const;
	void setPosition(types::Point pos);
	void move(types::Point delta);

	static types::Point getPosition(PandaObject* object);
	static void setPosition(PandaObject* object, const types::Point& pos);

	msg::Signal<void(types::Point newPos)> positionChanged;

private:
	bool m_isSet = false;
	types::Point m_position;
};

} // namespace graphview

} // namespace panda
