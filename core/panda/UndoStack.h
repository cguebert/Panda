#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include <panda/messaging.h>

#include <deque>
#include <memory>
#include <string>
#include <vector>

namespace panda
{

class PANDA_CORE_API UndoCommand
{
public:
	using SPtr = std::shared_ptr<UndoCommand>;

	virtual void redo(); /// Applies the change. By default, calls redo() on all childs
	virtual void undo(); /// Reverts the change. By default, calls undo() on all childs in reverse order

	std::string text() const;
	void setText(const std::string& text);

	virtual int id() const; /// If the command supports compression, this must return an integer unique to the command's class. Else return -1;
	virtual bool mergeWith(const UndoCommand* other); /// Attemps to merge this command with another of the same id.

	void push(UndoCommand::SPtr command); /// Append the command to the childs

private:
	friend class UndoStack;

	std::string m_commandText;
	std::vector<UndoCommand::SPtr> m_commandChilds;
};

class ScopedMacro;

class PANDA_CORE_API UndoStack
{
public:
	void push(const UndoCommand::SPtr& command);
	void clear();

	void setEnabled(bool enabled); // While set to false, no new commands can be added

	void undo();
	void redo();

	bool canUndo() const;
	bool canRedo() const;

	std::string undoText() const;
	std::string redoText() const;
	std::string text(int index) const;

	void setClean(); /// Marks the stack (and the current state) as clean.
	bool isClean() const; /// Returns true if the last command is the clean state.

	void setUndoLimit(int limit); /// When the number of commands on the stack is higher than the limit, commands are remooved from the bottom.
	int undoLimit() const;

	std::shared_ptr<ScopedMacro> beginMacro(const std::string& text); /// Any following commands until a call to endMacro() will be treated as one single command 

	bool isInCommandMacro() const;
	UndoCommand* getCurrentCommand() const; /// The command we are currently adding (if we want to connect another to this one)

	int count() const;
	int index() const;
	int cleanIndex() const;
	void setIndex(int index);

// Signals
	msg::Signal<void(bool)> m_canUndoChangedSignal;
	msg::Signal<void(bool)> m_canRedoChangedSignal;
	msg::Signal<void(int)> m_indexChangedSignal;
	msg::Signal<void(bool)> m_cleanChangedSignal;
	msg::Signal<void(const std::string&)> m_undoTextChangedSignal;
	msg::Signal<void(const std::string&)> m_redoTextChangedSignal;

private:
	friend class ScopedMacro;

	void endMacro();

	void checkStackLimit();
	void changeIndex(int index, bool clean = false);

	int m_index = 0, m_cleanIndex = 0, m_undoLimit = 0;
	int m_inCommandMacro = 0;
	bool m_enabled = true;
	std::deque<UndoCommand::SPtr> m_commands, m_macros;
	std::shared_ptr<UndoCommand> m_currentCommand;
};

class PANDA_CORE_API ScopedMacro
{
public:
	ScopedMacro(UndoStack* stack) : m_stack(stack) {}
	~ScopedMacro() { m_stack->endMacro(); }

protected:
	UndoStack* m_stack;
};

//****************************************************************************//

inline std::string UndoCommand::text() const
{ return m_commandText; }

inline void UndoCommand::setText(const std::string& text)
{ m_commandText = text; }

inline void UndoCommand::push(UndoCommand::SPtr command)
{ m_commandChilds.push_back(command); }

inline int UndoStack::undoLimit() const
{ return m_undoLimit; }

inline void UndoStack::setEnabled(bool enabled)
{ m_enabled = enabled; }

inline int UndoStack::count() const
{ return m_commands.size(); }

inline int UndoStack::index() const
{ return m_index; }

inline int UndoStack::cleanIndex() const
{ return m_cleanIndex; }

inline bool UndoStack::isInCommandMacro() const
{ return m_inCommandMacro > 0; }

inline UndoCommand* UndoStack::getCurrentCommand() const
{ return m_currentCommand.get(); }

} // namespace panda

#endif // UNDOSTACK_H
