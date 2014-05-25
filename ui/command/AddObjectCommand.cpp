#include <QCoreApplication>

#include <panda/PandaDocument.h>
#include <ui/GraphView.h>
#include <ui/drawstruct/ObjectDrawStruct.h>
#include <ui/command/AddObjectCommand.h>

AddObjectCommand::AddObjectCommand(panda::PandaDocument* document,
								   GraphView* view,
								   QSharedPointer<panda::PandaObject> object,
								   QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_document(document)
	, m_view(view)
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
{
	setText(QCoreApplication::translate("AddObjectCommand", "add objects"));
}

void AddObjectCommand::redo()
{
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
