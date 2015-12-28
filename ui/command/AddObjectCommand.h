#ifndef ADDOBJECTCOMMAND_H
#define ADDOBJECTCOMMAND_H

#include <QUndoCommand>

#include <memory>
#include <vector>

namespace panda
{
class PandaDocument;
class PandaObject;
}

class GraphView;
class ObjectDrawStruct;

class AddObjectCommand : public QUndoCommand
{
public:
	AddObjectCommand(panda::PandaDocument* document, GraphView* view, std::shared_ptr<panda::PandaObject> object, QUndoCommand* parent = nullptr);
	AddObjectCommand(panda::PandaDocument* document, GraphView* view, std::vector<std::shared_ptr<panda::PandaObject>> objects, QUndoCommand* parent = nullptr);
	AddObjectCommand(panda::PandaDocument* document, GraphView* view, std::vector<panda::PandaObject*> objects, QUndoCommand* parent = nullptr);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const QUndoCommand *other);

protected:
	panda::PandaDocument* m_document;
	GraphView* m_view;
	bool m_ignoreRedo;
	std::vector<std::shared_ptr<panda::PandaObject>> m_objects;
	std::vector<std::shared_ptr<ObjectDrawStruct>> m_drawStructs;
};

#endif
