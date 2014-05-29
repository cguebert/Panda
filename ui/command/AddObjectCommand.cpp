#include <QCoreApplication>

#include <panda/PandaDocument.h>
#include <ui/GraphView.h>
#include <ui/drawstruct/ObjectDrawStruct.h>
#include <ui/command/CommandId.h>
#include <ui/command/AddObjectCommand.h>

AddObjectCommand::AddObjectCommand(panda::PandaDocument* document,
								   GraphView* view,
								   QSharedPointer<panda::PandaObject> object,
								   QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_document(document)
	, m_view(view)
	, m_ignoreRedo(false)
{
	m_objects.push_back(object);
	setText(QCoreApplication::translate("AddObjectCommand", "add object"));
}

AddObjectCommand::AddObjectCommand(panda::PandaDocument* document,
								   GraphView* view,
								   QVector<QSharedPointer<panda::PandaObject>> objects,
								   QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_document(document)
	, m_view(view)
	, m_objects(objects)
	, m_ignoreRedo(false)
{
	setText(QCoreApplication::translate("AddObjectCommand", "add objects"));
}

AddObjectCommand::AddObjectCommand(panda::PandaDocument* document,
								   GraphView* view,
								   QList<panda::PandaObject*> objects,
								   QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_document(document)
	, m_view(view)
	, m_ignoreRedo(true) // This version is used when importing a document: when the command is created, objects are already added
{
	for(auto object : objects)
	{
		auto objectPtr = document->getSharedPointer(object);
		if(objectPtr)
			m_objects.push_back(objectPtr);
	}

	setText(QCoreApplication::translate("AddObjectCommand", "add objects"));
}

int AddObjectCommand::id() const
{
	return getCommandId<AddObjectCommand>();
}

void AddObjectCommand::redo()
{
	if(m_ignoreRedo)
	{
		m_ignoreRedo = false;
		return;
	}

	for(auto ods : m_drawStructs)
		m_view->setObjectDrawStruct(ods->getObject(), ods);

	for(auto object : m_objects)
		m_document->addObject(object);
}

void AddObjectCommand::undo()
{
	if(m_drawStructs.empty())
	{
		for(auto object : m_objects)
		{
			auto ods = m_view->getSharedObjectDrawStruct(object.data());
			m_drawStructs.push_back(ods);
		}
	}

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
		m_drawStructs += command->m_drawStructs;
		return true;
	}

	return false;
}
