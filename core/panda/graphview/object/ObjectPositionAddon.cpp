#include <panda/graphview/object/ObjectPositionAddon.h>
#include <panda/object/PandaObject.h>

namespace panda
{

namespace graphview
{

ObjectPositionAddon::ObjectPositionAddon(PandaObject& object) 
	: BaseObjectAddon(object) 
{ }

void ObjectPositionAddon::setDefinition(ObjectAddonNodeDefinition& nodeDefinition)
{
	nodeDefinition.addAttribute("x");
	nodeDefinition.addAttribute("y");
}

void ObjectPositionAddon::save(ObjectAddonNode& node)
{ 
	node.setAttribute("x", m_position.x); 
	node.setAttribute("y", m_position.y); 
}

void ObjectPositionAddon::load(const ObjectAddonNode& node)
{ 
	setPosition({ node.attribute("x").toFloat(), node.attribute("y").toFloat() }); 
}

bool ObjectPositionAddon::isSet() const
{ 
	return m_isSet; 
}

types::Point ObjectPositionAddon::getPosition() const
{ 
	return m_position; 
}

void ObjectPositionAddon::setPosition(types::Point pos)
{
	m_position = pos;
	m_isSet = true;
	positionChanged.run(pos);
}

void ObjectPositionAddon::move(types::Point delta)
{
	setPosition(m_position + delta);
}

//****************************************************************************//

types::Point ObjectPositionAddon::getPosition(PandaObject* object)
{
	return object->addons().edit<ObjectPositionAddon>().getPosition();
}

void ObjectPositionAddon::setPosition(PandaObject* object, const types::Point& pos)
{
	object->addons().edit<ObjectPositionAddon>().setPosition(pos);
}

//****************************************************************************//

int ObjectPositionAddon_Reg = RegisterObjectAddon<ObjectPositionAddon>();

} // namespace graphview

} // namespace panda
