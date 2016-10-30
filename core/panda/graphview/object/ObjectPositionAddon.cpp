#include <ui/graphview/object/ObjectPositionAddon.h>
#include <panda/XmlDocument.h>
#include <panda/object/PandaObject.h>

namespace graphview
{

ObjectPositionAddon::ObjectPositionAddon(panda::PandaObject& object) 
	: panda::BaseObjectAddon(object) 
{ }

void ObjectPositionAddon::setDefinition(panda::ObjectAddonNodeDefinition& nodeDefinition)
{
	nodeDefinition.addAttribute("x");
	nodeDefinition.addAttribute("y");
}

void ObjectPositionAddon::save(panda::ObjectAddonNode& node)
{ 
	node.setAttribute("x", m_position.x); 
	node.setAttribute("y", m_position.y); 
}

void ObjectPositionAddon::load(const panda::ObjectAddonNode& node)
{ 
	setPosition({ node.attribute("x").toFloat(), node.attribute("y").toFloat() }); 
}

bool ObjectPositionAddon::isSet() const
{ 
	return m_isSet; 
}

panda::types::Point ObjectPositionAddon::getPosition() const
{ 
	return m_position; 
}

void ObjectPositionAddon::setPosition(panda::types::Point pos)
{
	m_position = pos;
	m_isSet = true;
	positionChanged.run(pos);
}

void ObjectPositionAddon::move(panda::types::Point delta)
{
	setPosition(m_position + delta);
}

//****************************************************************************//

panda::types::Point ObjectPositionAddon::getPosition(panda::PandaObject* object)
{
	return object->addons().edit<ObjectPositionAddon>().getPosition();
}

void ObjectPositionAddon::setPosition(panda::PandaObject* object, const panda::types::Point& pos)
{
	object->addons().edit<ObjectPositionAddon>().setPosition(pos);
}

//****************************************************************************//

int ObjectPositionAddon_Reg = panda::RegisterObjectAddon<ObjectPositionAddon>();

} // namespace graphview
