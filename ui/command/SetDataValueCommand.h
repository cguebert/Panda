#ifndef SETDATAVALUECOMMAND_H
#define SETDATAVALUECOMMAND_H

#include <QUndoCommand>
#include <QCoreApplication>

#include <panda/Data.h>
#include <panda/command/CommandId.h>

template <class T>
class SetDataValueCommand : public QUndoCommand
{
public:
	typedef panda::Data<T> TData;
	SetDataValueCommand(TData* data, T oldValue, T newValue,
						panda::PandaObject* owner = nullptr, /// Refresh another object when the value changes (used with GroupWithLayer)
						QUndoCommand* parent = nullptr)
		: QUndoCommand(parent)
		, m_data(data)
		, m_oldValue(oldValue)
		, m_newValue(newValue)
		, m_owner(nullptr)
	{
		if(m_data->getOwner() != owner)
			m_owner = owner;
		setText(QCoreApplication::translate("SetDataValueCommand", "modify data value"));
	}

	virtual int id() const
	{
		return getCommandId<SetDataValueCommand<T>>();
	}

	virtual void redo()
	{
		m_data->setValue(m_newValue);
		m_data->getOwner()->emitDirty();
		if(m_owner)
		{
			m_owner->setDirtyValue(m_data);
			m_owner->emitDirty();
		}
	}

	virtual void undo()
	{
		m_data->setValue(m_oldValue);
		m_data->getOwner()->emitDirty();
		if(m_owner)
		{
			m_owner->setDirtyValue(m_data);
			m_owner->emitDirty();
		}
	}

	virtual bool mergeWith(const QUndoCommand *other)
	{
		const SetDataValueCommand<T>* command = dynamic_cast<const SetDataValueCommand<T>*>(other);
		if(!command)
			return false;
		if(m_data == command->m_data && m_owner == command->m_owner)
		{
			m_newValue = command->m_newValue;
			return true;
		}

		return false;
	}

protected:
	TData* m_data;
	T m_oldValue, m_newValue;
	panda::PandaObject* m_owner;
};

#endif
