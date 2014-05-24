#include <QUndoCommand>
#include <QCoreApplication>

#include <panda/PandaDocument.h>
#include <ui/command/CommandId.h>
#include <ui/command/CreateObjectCommand.h>

AddObjectCommand::AddObjectCommand(panda::PandaDocument* document,
										 QSharedPointer<panda::PandaObject> object,
										 QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_document(document)
{
	m_objects.push_back(object);
	setText(QCoreApplication::translate("CreateObjectCommand", "create objects"));
}

int AddObjectCommand::id() const
{
	return getCommandId<AddObjectCommand>();
}

void AddObjectCommand::redo()
{
	for(auto object : m_objects)
		m_document->addObject(object);
}

void AddObjectCommand::undo()
{
	for(auto object : m_objects)
		m_document->removeObject(object.data());
}

bool AddObjectCommand::mergeWith(const QUndoCommand *other)
{
	// Only merge if creating a macro of multiple commands (not in case of multiple users actions)
	if(!m_document->isInCommandMacro())
		return false;

	const AddObjectCommand* command = dynamic_cast<const AddObjectCommand*>(other);
	if(!command)
		return false;
	if(m_document == command->m_document)
	{
		m_objects += command->m_objects;
		return true;
	}

	return false;
}

