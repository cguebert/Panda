#include <QCoreApplication>

#include <panda/PandaDocument.h>
#include <ui/GraphView.h>
#include <ui/command/CommandId.h>
#include <ui/command/DeleteObjectCommand.h>
#include <ui/command/LinkDatasCommand.h>

DeleteObjectCommand::DeleteObjectCommand(panda::PandaDocument* document,
										 GraphView* view,
										 QList<panda::PandaObject*> objects,
										 QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_document(document)
	, m_view(view)
{
	for(auto object : objects)
	{
		auto objectPtr = document->getSharedPointer(object);
		auto ods = view->getSharedObjectDrawStruct(object);
		if(objectPtr && ods)
			m_objects.push_back(qMakePair(objectPtr, ods));

		// Create (and apply) unlink commands
		// We start from output datas
		for(auto data : objectPtr->getOutputDatas())
		{
			for(auto output : data->getOutputs())
			{
				auto data2 = dynamic_cast<panda::BaseData*>(output);
				if(data2 && data2->getOwner())
					m_document->addCommand(new LinkDatasCommand(data2, nullptr));
			}
		}

		// And then the inputs (getInputDatas gives a copy,
		//  so no problem when some objects remove datas during this operation)
		for(auto data : objectPtr->getInputDatas())
		{
			if(data->getParent())
				m_document->addCommand(new LinkDatasCommand(data, nullptr));
		}
	}
	setText(QCoreApplication::translate("DeleteObjectCommand", "delete objects"));
}

int DeleteObjectCommand::id() const
{
	return getCommandId<DeleteObjectCommand>();
}

void DeleteObjectCommand::redo()
{
	m_document->selectNone();

	for(auto object : m_objects)
		m_document->removeObject(object.first.data());
}

void DeleteObjectCommand::undo()
{
	for(auto object : m_objects)
	{
		m_view->setObjectDrawStruct(object.first.data(), object.second);
		m_document->addObject(object.first);
	}
}

bool DeleteObjectCommand::mergeWith(const QUndoCommand *other)
{
	// Only merge if creating a macro of multiple commands (not in case of multiple users actions)
	if(!m_document->isInCommandMacro())
		return false;

	const DeleteObjectCommand* command = dynamic_cast<const DeleteObjectCommand*>(other);
	if(!command)
		return false;
	if(m_document == command->m_document)
	{
		m_objects += command->m_objects;
		return true;
	}

	return false;
}

