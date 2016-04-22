#ifndef REMOVEGENERICDATACOMMAND_H
#define REMOVEGENERICDATACOMMAND_H

#include <panda/core.h>
#include <panda/UndoStack.h>

namespace panda
{

class GenericObject;

class PANDA_CORE_API ConnectGenericDataCommand : public UndoCommand
{
public:
	ConnectGenericDataCommand(GenericObject* object, BaseData* parent);

	virtual void redo();
	virtual void undo();

protected:
	GenericObject* m_object;
	BaseData *m_parent, *m_inputData = nullptr;
	int m_type, m_index;
};

//****************************************************************************//

class PANDA_CORE_API RemoveGenericDataCommand : public UndoCommand
{
public:
	RemoveGenericDataCommand(GenericObject* object, int type, int index);

	virtual void redo();
	virtual void undo();

protected:
	GenericObject* m_object;
	int m_type, m_index;
};

} // namespace panda

#endif
