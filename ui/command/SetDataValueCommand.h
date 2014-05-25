#ifndef SETDATAVALUECOMMAND_H
#define SETDATAVALUECOMMAND_H

#include <QUndoCommand>
#include <QCoreApplication>

#include <panda/Data.h>

template <class T>
class SetDataValueCommand : public QUndoCommand
{
public:
	typedef panda::Data<T> TData;
	SetDataValueCommand(TData* data, T oldValue, T newValue, QUndoCommand* parent = nullptr)
		: QUndoCommand(parent)
		, m_data(data)
		, m_oldValue(oldValue)
		, m_newValue(newValue)
	{
		setText(QCoreApplication::translate("SetDataValueCommand", "modify data value"));
	}

	virtual void redo()
	{
		m_data->setValue(m_newValue);
	}

	virtual void undo()
	{
		m_data->setValue(m_oldValue);
	}

protected:
	TData* m_data;
	T m_oldValue, m_newValue;
};

#endif
