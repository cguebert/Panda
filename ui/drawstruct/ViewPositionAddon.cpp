#include <ui/drawstruct/ViewPositionAddon.h>
#include <panda/XmlDocument.h>
#include <panda/object/PandaObject.h>

ViewPositionAddon::ViewPositionAddon(panda::PandaObject& object) 
	: panda::BaseObjectAddon(object) 
{ }

void ViewPositionAddon::setDefinition(panda::ObjectAddonNodeDefinition& nodeDefinition)
{
	nodeDefinition.addAttribute("x");
	nodeDefinition.addAttribute("y");
}

void ViewPositionAddon::save(panda::XmlElement& elem)
{ 
	elem.setAttribute("x", m_position.x); 
	elem.setAttribute("y", m_position.y); 
}

void ViewPositionAddon::load(panda::XmlElement& elem)
{ 
	setPosition({ elem.attribute("x").toFloat(), elem.attribute("y").toFloat() }); 
}

bool ViewPositionAddon::isSet() const
{ 
	return m_isSet; 
}

panda::types::Point ViewPositionAddon::getPosition() const
{ 
	return m_position; 
}

void ViewPositionAddon::setPosition(panda::types::Point pos)
{
	m_position = pos;
	m_isSet = true;
	positionChanged.run(pos);
}

void ViewPositionAddon::move(panda::types::Point delta)
{
	setPosition(m_position + delta);
}

//****************************************************************************//

panda::types::Point getPosition(panda::PandaObject* object)
{
	return object->addons().edit<ViewPositionAddon>().getPosition();
}

void setPosition(panda::PandaObject* object, const panda::types::Point& pos)
{
	object->addons().edit<ViewPositionAddon>().setPosition(pos);
}

//****************************************************************************//

int ViewPositionAddon_Reg = panda::RegisterObjectAddon<ViewPositionAddon>();