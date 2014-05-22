#include <QUndoCommand>
#include <QCoreApplication>

#include <panda/PandaDocument.h>
#include <ui/GraphView.h>
#include <ui/command/DeleteObjectCommand.h>

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
	}
	setText(QCoreApplication::translate("DeleteObjectCommand", "delete objects"));
}

void DeleteObjectCommand::redo()
{
	m_document->selectNone();

	// TODO: Disconnect the links in another command
	for(auto object : m_objects)
	{
		for(auto data : object.first->getOutputDatas())
		{
			for(auto output : data->getOutputs())
			{
				auto data2 = dynamic_cast<panda::BaseData*>(output);
				if(data2 && data2->getOwner())
					data2->getOwner()->dataSetParent(data2, nullptr);
			}
		}

		for(auto data : object.first->getInputDatas())
		{
			if(data->getParent())
				object.first->dataSetParent(data, nullptr);
		}
	}

	for(auto object : m_objects)
		m_document->doRemoveObject(object.first.data());
}

void DeleteObjectCommand::undo()
{
	for(auto object : m_objects)
	{
		m_view->setObjectDrawStruct(object.first.data(), object.second);
		m_document->doAddObject(object.first);
	}
}
/*
bool DeleteObjectCommand::mergeWith(const QUndoCommand *other)
{
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
*/
