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

class PANDA_CORE_API RemoveObjectCommand : public UndoCommand
{
public:
	enum class LinkOperation { Keep, Unlink };
	enum class ObjectOperation { None, RemoveFromDocument };

	RemoveObjectCommand(PandaDocument* document, 
						ObjectsList& objectsList, 
						const std::vector<PandaObject*>& objects, 
						LinkOperation linkOp = LinkOperation::Unlink, 
						ObjectOperation objectOp = ObjectOperation::RemoveFromDocument);

	RemoveObjectCommand(PandaDocument* document, 
						ObjectsList& objectsList, 
						PandaObject* object, 
						LinkOperation linkOp = LinkOperation::Unlink, 
						ObjectOperation objectOp = ObjectOperation::RemoveFromDocument);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const UndoCommand *other);

protected:
	PandaDocument* m_document;
	ObjectsList& m_objectsList;
	std::vector<std::shared_ptr<PandaObject>> m_objects;
	bool m_removeFromDocument; // If true, signal the objects they are removed from the document (false if moving to a group)

	void prepareCommand(const std::vector<PandaObject*>& objects, bool unlinkDatas);
};

} // namespace panda

#endif
