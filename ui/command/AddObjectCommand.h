#ifndef ADDOBJECTCOMMAND_H
#define ADDOBJECTCOMMAND_H

#include <panda/UndoStack.h>

#include <memory>
#include <vector>

namespace panda
{
class PandaDocument;
class PandaObject;
}

class GraphView;
class ObjectDrawStruct;

class AddObjectCommand : public panda::UndoCommand
{
public:
	AddObjectCommand(panda::PandaDocument* document, GraphView* view, std::shared_ptr<panda::PandaObject> object);
	AddObjectCommand(panda::PandaDocument* document, GraphView* view, std::vector<std::shared_ptr<panda::PandaObject>> objects);
	AddObjectCommand(panda::PandaDocument* document, GraphView* view, std::vector<panda::PandaObject*> objects);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const UndoCommand *other);

protected:
	panda::PandaDocument* m_document;
	GraphView* m_view;
	bool m_ignoreRedo;
	std::vector<std::shared_ptr<panda::PandaObject>> m_objects;
	std::vector<std::shared_ptr<ObjectDrawStruct>> m_drawStructs;
};

#endif
