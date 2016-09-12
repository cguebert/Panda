#pragma once

#include <panda/object/ObjectAddons.h>
#include <panda/types/Point.h>
#include <panda/messaging.h>

class ViewPositionAddon : public panda::BaseObjectAddon
{
public:
	ViewPositionAddon(panda::PandaObject& object);

	static void setDefinition(panda::ObjectAddonNodeDefinition& nodeDefinition);

	void save(panda::ObjectAddonNode& node) override;
	void load(const panda::ObjectAddonNode& node) override;

	bool isSet() const;

	panda::types::Point getPosition() const;
	void setPosition(panda::types::Point pos);
	void move(panda::types::Point delta);

	static panda::types::Point getPosition(panda::PandaObject* object);
	static void setPosition(panda::PandaObject* object, const panda::types::Point& pos);

	panda::msg::Signal<void(panda::types::Point newPos)> positionChanged;

private:
	bool m_isSet = false;
	panda::types::Point m_position;
};