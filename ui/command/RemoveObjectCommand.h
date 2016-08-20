#ifndef DELETEOBJECTCOMMAND_H
#define DELETEOBJECTCOMMAND_H

#include <panda/UndoStack.h>

#include <memory>
#include <vector>

namespace panda
{
class ObjectsList;
class PandaDocument;
class PandaObject;
}

class GraphView;
class ObjectDrawStruct;

class RemoveObjectCommand : public panda::UndoCommand
{
public:
	enum class LinkOperation { Keep, Unlink };
	enum class ObjectOperation { None, RemoveFromDocument };

	RemoveObjectCommand(panda::PandaDocument* document, 
						panda::ObjectsList& objectsList, 
						GraphView* view,
						const std::vector<panda::PandaObject*>& objects, 
						LinkOperation linkOp = LinkOperation::Unlink, 
						ObjectOperation objectOp = ObjectOperation::RemoveFromDocument);

	RemoveObjectCommand(panda::PandaDocument* document, 
						panda::ObjectsList& objectsList, 
						GraphView* view,
						panda::PandaObject* object, 
						LinkOperation linkOp = LinkOperation::Unlink, 
						ObjectOperation objectOp = ObjectOperation::RemoveFromDocument);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const panda::UndoCommand *other);

protected:
	panda::PandaDocument* m_document;
	panda::ObjectsList& m_objectsList;
	GraphView* m_view;
	std::vector< std::pair< std::shared_ptr<panda::PandaObject>, std::shared_ptr<ObjectDrawStruct> > > m_objects;
	bool m_removeFromDocument; // If true, signal the objects they are removed from the document (false if moving to a group)

	void prepareCommand(const std::vector<panda::PandaObject*>& objects, bool unlinkDatas);
};

#endif
