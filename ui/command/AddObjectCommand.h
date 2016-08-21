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
}

class GraphView;
class ObjectDrawStruct;

class AddObjectCommand : public panda::UndoCommand
{
public:
	AddObjectCommand(panda::PandaDocument* document, 
					 panda::ObjectsList& objectsList, 
					 GraphView* view, 
					 std::shared_ptr<panda::PandaObject> object,
					 bool newObject = true);
	AddObjectCommand(panda::PandaDocument* document, 
					 panda::ObjectsList& objectsList, 
					 GraphView* view, 
					 std::vector<std::shared_ptr<panda::PandaObject>> objects,
					 bool newObject = true);
	AddObjectCommand(panda::PandaDocument* document, 
					 panda::ObjectsList& objectsList, 
					 GraphView* view, 
					 std::vector<panda::PandaObject*> objects,
					 bool newObject = true);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const UndoCommand *other);

protected:
	panda::PandaDocument* m_document;
	panda::ObjectsList& m_objectsList;
	GraphView* m_view;
	bool m_ignoreRedo;
	bool m_addToDocument; // If true, signal to the objects they are being added to the document (false if moving to a group)
	std::vector<std::shared_ptr<panda::PandaObject>> m_objects;
	std::vector<std::shared_ptr<ObjectDrawStruct>> m_drawStructs;
};

#endif
