#ifndef DELETEOBJECTCOMMAND_H
#define DELETEOBJECTCOMMAND_H

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

class RemoveObjectCommand : public panda::UndoCommand
{
public:
	RemoveObjectCommand(panda::PandaDocument* document, GraphView* view,
						const std::vector<panda::PandaObject*>& objects, bool unlinkDatas = true);
	RemoveObjectCommand(panda::PandaDocument* document, GraphView* view,
						panda::PandaObject* object, bool unlinkDatas = true);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const panda::UndoCommand *other);

protected:
	panda::PandaDocument* m_document;
	GraphView* m_view;
	std::vector< std::pair< std::shared_ptr<panda::PandaObject>, std::shared_ptr<ObjectDrawStruct> > > m_objects;

	void prepareCommand(const std::vector<panda::PandaObject*>& objects, bool unlinkDatas);
};

#endif
