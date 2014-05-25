#include <QCoreApplication>

#include <panda/GenericObject.h>
#include <ui/command/RemoveGenericDataCommand.h>

RemoveGenericDataCommand::RemoveGenericDataCommand(panda::GenericObject* object,
												   int type,
												   int index,
												   QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_object(object)
	, m_type(type)
	, m_index(index)
{

	setText(QCoreApplication::translate("RemoveGenericDataCommand", "remove generic data"));
}

void RemoveGenericDataCommand::redo()
{

}

void RemoveGenericDataCommand::undo()
{
	m_object->createDatas(m_type, m_index);
}
