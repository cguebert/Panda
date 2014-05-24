#ifndef REMOVEGENERICDATACOMMAND_H
#define REMOVEGENERICDATACOMMAND_H

#include <QUndoCommand>
#include <QSharedPointer>
#include <QVector>
#include <QPair>

namespace panda
{
class GenericObject;
}

class RemoveGenericDataCommand : public QUndoCommand
{
public:
	RemoveGenericDataCommand(panda::GenericObject* object, int type, int index, QUndoCommand* parent = nullptr);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

protected:
	panda::GenericObject* m_object;
	int m_type, m_index;
};

#endif
