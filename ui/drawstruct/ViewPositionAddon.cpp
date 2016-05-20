#include <ui/drawstruct/ViewPositionAddon.h>
#include <panda/XmlDocument.h>

ViewPositionAddon::ViewPositionAddon(panda::PandaObject& object) 
	: panda::BaseObjectAddon(object) 
{ }

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

void ViewPositionAddon::setPosition(panda::types::Point pt)
{
	m_position = pt;
	m_isSet = true;
	positionChanged.run(pt);
}

//****************************************************************************//

int ViewPositionAddon_Reg = panda::RegisterObjectAddon<ViewPositionAddon>();