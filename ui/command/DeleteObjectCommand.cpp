#include <QCoreApplication>

#include <panda/PandaDocument.h>
#include <ui/GraphView.h>
#include <ui/command/CommandId.h>
#include <ui/command/DeleteObjectCommand.h>
#include <ui/command/LinkDatasCommand.h>

DeleteObjectCommand::DeleteObjectCommand(panda::PandaDocument* document,
										 GraphView* view,
										 const QList<panda::PandaObject*>& objects,
										 bool unlinkDatas,
										 QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_document(document)
	, m_view(view)
{
	prepareCommand(objects, unlinkDatas);
	setText(QCoreApplication::translate("DeleteObjectCommand", "delete objects"));
}

DeleteObjectCommand::DeleteObjectCommand(panda::PandaDocument* document,
										 GraphView* view,
										 panda::PandaObject* object,
										 bool unlinkDatas,
										 QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_document(document)
	, m_view(view)
{
	QList<panda::PandaObject*> objects;
	objects.push_back(object);
	prepareCommand(objects, unlinkDatas);
	setText(QCoreApplication::translate("DeleteObjectCommand", "delete objects"));
}

void DeleteObjectCommand::prepareCommand(const QList<panda::PandaObject*>& objects, bool unlinkDatas)
{
	for(auto object : objects)
	{
		auto objectPtr = m_document->getSharedPointer(object);
		auto ods = m_view->getSharedObjectDrawStruct(object);
		if(objectPtr && ods)
			m_objects.push_back(qMakePair(objectPtr, ods));

		if(unlinkDatas)
		{
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
	}
}

int DeleteObjectCommand::id() const
{
	return getCommandId<DeleteObjectCommand>();
}

void DeleteObjectCommand::redo()
{
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

