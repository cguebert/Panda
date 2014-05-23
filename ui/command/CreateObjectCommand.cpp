#include <QUndoCommand>
#include <QCoreApplication>

#include <panda/PandaDocument.h>
#include <ui/command/CreateObjectCommand.h>

CreateObjectCommand::CreateObjectCommand(panda::PandaDocument* document,
										 QSharedPointer<panda::PandaObject> object,
										 QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_document(document)
{
	m_objects.push_back(object);
	setText(QCoreApplication::translate("CreateObjectCommand", "create objects"));
}

void CreateObjectCommand::redo()
{
	for(auto object : m_objects)
		m_document->addObject(object);
}

void CreateObjectCommand::undo()
{
	for(auto object : m_objects)
		m_document->removeObject(object.data());
}
/*
bool CreateObjectCommand::mergeWith(const QUndoCommand *other)
{
	const CreateObjectCommand* command = dynamic_cast<const CreateObjectCommand*>(other);
	if(!command)
		return false;
	if(m_document == command->m_document)
	{
		m_objects += command->m_objects;
		return true;
	}

	return false;
}
*/
