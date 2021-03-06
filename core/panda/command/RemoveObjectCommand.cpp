#include <panda/command/RemoveObjectCommand.h>

#include <panda/document/PandaDocument.h>
#include <panda/command/CommandId.h>
#include <panda/command/LinkDatasCommand.h>
#include <panda/document/ObjectsList.h>
#include <panda/helper/algorithm.h>

namespace panda
{

RemoveObjectCommand::RemoveObjectCommand(PandaDocument* document,
										 ObjectsList& objectsList,
										 const std::vector<PandaObject*>& objects,
										 LinkOperation linkOp, 
										 ObjectOperation objectOp)
	: m_document(document)
	, m_objectsList(objectsList)
	, m_removeFromDocument(objectOp == ObjectOperation::RemoveFromDocument)
{
	prepareCommand(objects, linkOp == LinkOperation::Unlink);
	setText("delete objects");
}

RemoveObjectCommand::RemoveObjectCommand(PandaDocument* document,
										 ObjectsList& objectsList,
										 PandaObject* object,
										 LinkOperation linkOp, 
										 ObjectOperation objectOp)
	: m_document(document)
	, m_objectsList(objectsList)
	, m_removeFromDocument(objectOp == ObjectOperation::RemoveFromDocument)
{
	std::vector<PandaObject*> objects;
	objects.push_back(object);
	prepareCommand(objects, linkOp == LinkOperation::Unlink);
	setText("delete objects");
}

void RemoveObjectCommand::prepareCommand(const std::vector<PandaObject*>& objects, bool unlinkDatas)
{
	for(auto object : objects)
	{
		auto objectPtr = m_objectsList.getShared(object);
		if(objectPtr)
			m_objects.emplace_back(objectPtr);

		if(unlinkDatas)
		{
			// Create (and apply) unlink commands
			// We start from output datas
			const auto outputsDatas = object->getOutputDatas();
			for(auto data : outputsDatas)
			{
				const auto outputs = data->getOutputs();
				for(auto output : outputs)
				{
					auto data2 = dynamic_cast<BaseData*>(output);
					if(data2 && data2->getOwner())
						m_document->getUndoStack().push(std::make_shared<LinkDatasCommand>(data2, nullptr));
				}
			}

			// And then the inputs
			// We have to reverse the order as the generic objects can remove following datas when unlinking one
			auto inputDatas = object->getInputDatas();
			std::reverse(inputDatas.begin(), inputDatas.end());
			for(auto data : inputDatas)
			{
				if(data->getParent())
					m_document->getUndoStack().push(std::make_shared<LinkDatasCommand>(data, nullptr));
			}
		}
	}
}

int RemoveObjectCommand::id() const
{
	return getCommandId<RemoveObjectCommand>();
}

void RemoveObjectCommand::redo()
{
	for(auto& object : m_objects)
		m_objectsList.removeObject(object.get(), m_removeFromDocument);
}

void RemoveObjectCommand::undo()
{
	for(auto& object : m_objects)
		m_objectsList.addObject(object, m_removeFromDocument);
}

bool RemoveObjectCommand::mergeWith(const UndoCommand *other)
{
	// Only merge if creating a macro of multiple commands (not in case of multiple users actions)
	if(!m_document->getUndoStack().isInCommandMacro())
		return false;

	const RemoveObjectCommand* command = dynamic_cast<const RemoveObjectCommand*>(other);
	if(!command)
		return false;
	if(m_document == command->m_document)
	{
		helper::concatenate(m_objects, command->m_objects);
		return true;
	}

	return false;
}

} // namespace panda
