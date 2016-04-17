#include <ui/drawstruct/UserValueDrawStruct.h>
#include <ui/graphview/GraphView.h>

#include <panda/PandaDocument.h>
#include <panda/object/Group.h>

#include <QPainter>

UserValueDrawStruct::UserValueDrawStruct(GraphView* view, panda::BaseGeneratorUser* object)
	: ObjectDrawStruct(view, object)
	, m_userValueObject(object)
{
	update();
}

std::string UserValueDrawStruct::getLabel() const
{
	if (m_userValueObject)
	{
		const auto& name = m_userValueObject->getCaption();
		if (!name.empty())
			return name + "\n(" + m_userValueObject->getName() + ")";
	}
	
	return ObjectDrawStruct::getLabel();
}

int UserValueDrawClass = RegisterDrawObject<panda::BaseGeneratorUser, UserValueDrawStruct>();
