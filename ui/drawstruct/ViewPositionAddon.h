#pragma once

#include <panda/object/ObjectAddons.h>
#include <panda/types/Point.h>
#include <panda/messaging.h>

namespace panda
{
	class XmlElement;
}

class ViewPositionAddon : public panda::BaseObjectAddon
{
public:
	ViewPositionAddon(panda::PandaObject& object);

	void save(panda::XmlElement& elem) override;
	void load(panda::XmlElement& elem) override;

	bool isSet() const;

	panda::types::Point getPosition() const;
	void setPosition(panda::types::Point pt);

	panda::msg::Signal<void(panda::types::Point newPos)> positionChanged;

private:
	bool m_isSet = false;
	panda::types::Point m_position;
};