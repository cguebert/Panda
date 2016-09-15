#include <panda/object/GenericObject.h>
#include <panda/command/GenericDataCommand.h>
#include <panda/document/PandaDocument.h>

namespace panda
{

ConnectGenericDataCommand::ConnectGenericDataCommand(GenericObject* object, BaseData* parent)
	: m_object(object)
	, m_parent(parent)
{
	setText("connect to generic data");
}

void ConnectGenericDataCommand::redo()
{
	int type = m_object->getGenericData()->getCompatibleType(m_parent);
	m_inputData = m_object->createDatas(type);

	if(m_inputData)
		m_inputData->setParent(m_parent);

	m_object->parentDocument()->onModifiedObject(m_object);
}

void ConnectGenericDataCommand::undo()
{
	if (m_inputData)
		m_inputData->setParent(nullptr);

	m_object->disconnectData(m_inputData);

	m_inputData = nullptr;
	m_object->parentDocument()->onModifiedObject(m_object);
}

//****************************************************************************//

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
