#include <panda/PandaDocument.h>
#include <panda/command/CommandId.h>
#include <panda/document/ObjectsList.h>
#include <panda/helper/algorithm.h>
#include <ui/graphview/GraphView.h>
#include <ui/drawstruct/ObjectDrawStruct.h>
#include <ui/command/AddObjectCommand.h>

AddObjectCommand::AddObjectCommand(panda::PandaDocument* document,
								   panda::ObjectsList& objectsList,
								   GraphView* view,
								   std::shared_ptr<panda::PandaObject> object,
								   bool newObject)
	: m_document(document)
	, m_objectsList(objectsList)
	, m_view(view)
	, m_ignoreRedo(false)
	, m_addToDocument(newObject)
{
	m_objects.push_back(object);
	setText("add object");
}

AddObjectCommand::AddObjectCommand(panda::PandaDocument* document,
								   panda::ObjectsList& objectsList,
								   GraphView* view,
								   std::vector<std::shared_ptr<panda::PandaObject>> objects,
								   bool newObject)
	: m_document(document)
	, m_objectsList(objectsList)
	, m_view(view)
	, m_objects(objects)
	, m_ignoreRedo(false)
	, m_addToDocument(newObject)
{
	setText("add objects");
}

AddObjectCommand::AddObjectCommand(panda::PandaDocument* document,
								   panda::ObjectsList& objectsList,
								   GraphView* view,
								   std::vector<panda::PandaObject*> objects,
								   bool newObject)
	: m_document(document)
	, m_objectsList(objectsList)
	, m_view(view)
	, m_ignoreRedo(true) // This version is used when importing a document: when the command is created, objects are already added
	, m_addToDocument(newObject)
{
	for(auto object : objects)
	{
		auto objectPtr = m_objectsList.getShared(object);
		if(objectPtr)
			m_objects.push_back(objectPtr);
	}

	setText("add objects");
}

int AddObjectCommand::id() const
{
	return panda::getCommandId<AddObjectCommand>();
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
		m_objectsList.addObject(object, m_addToDocument);
}

void AddObjectCommand::undo()
{
	if(m_view && m_drawStructs.empty())
	{
		for(auto object : m_objects)
		{
			auto ods = m_view->getSharedObjectDrawStruct(object.get());
			if(ods)
				m_drawStructs.push_back(ods);
		}
	}

	for(auto object : m_objects)
		m_objectsList.removeObject(object.get(), m_addToDocument);
}

bool AddObjectCommand::mergeWith(const UndoCommand *other)
{
	// Only merge if creating a macro of multiple commands (not in case of multiple users actions)
	if(!m_document->getUndoStack().isInCommandMacro())
		return false;

	const AddObjectCommand* command = dynamic_cast<const AddObjectCommand*>(other);
	if(!command)
		return false;

	if(m_document == command->m_document)
	{
		panda::helper::concatenate(m_objects, command->m_objects);
		panda::helper::concatenate(m_drawStructs, command->m_drawStructs);
		return true;
	}

	return false;
}
