#include <panda/object/GenericObject.h>
#include <panda/command/RemoveGenericDataCommand.h>

namespace panda
{

RemoveGenericDataCommand::RemoveGenericDataCommand(GenericObject* object,
												   int type,
												   int index)
	: m_object(object)
	, m_type(type)
	, m_index(index)
{
	setText("remove generic data");
}

void RemoveGenericDataCommand::redo()
{

}

void RemoveGenericDataCommand::undo()
{
	m_object->createDatas(m_type, m_index);
}

} // namespace panda
