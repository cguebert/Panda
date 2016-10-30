#ifndef ADDOBJECTCOMMAND_H
#define ADDOBJECTCOMMAND_H

#include <panda/UndoStack.h>

#include <memory>
#include <vector>

namespace panda
{

class ObjectsList;
class PandaDocument;
class PandaObject;

class PANDA_CORE_API AddObjectCommand : public UndoCommand
{
public:
	AddObjectCommand(PandaDocument* document, 
					 ObjectsList& objectsList, 
					 std::shared_ptr<PandaObject> object,
					 bool newObject = true);
	AddObjectCommand(PandaDocument* document, 
					 ObjectsList& objectsList, 
					 std::vector<std::shared_ptr<PandaObject>> objects,
					 bool newObject = true);
	AddObjectCommand(PandaDocument* document, 
					 ObjectsList& objectsList, 
					 std::vector<PandaObject*> objects,
					 bool newObject = true);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const UndoCommand *other);

protected:
	PandaDocument* m_document;
	ObjectsList& m_objectsList;
	bool m_ignoreRedo;
	bool m_addToDocument; // If true, signal to the objects they are being added to the document (false if moving to a group)
	std::vector<std::shared_ptr<PandaObject>> m_objects;
};

} // namespace panda

#endif
