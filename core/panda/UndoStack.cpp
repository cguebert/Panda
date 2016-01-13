#include <panda/UndoStack.h>

namespace panda
{

void UndoCommand::redo()
{
	for (auto& child : m_commandChilds)
		child->redo();
}

void UndoCommand::undo()
{
	// Calling undo on childs, in reverse order
	for (auto it = m_commandChilds.rbegin(), itEnd = m_commandChilds.rend(); it != itEnd; ++it)
		(*it)->undo();
}

//****************************************************************************//

void UndoStack::push(UndoCommand::SPtr command)
{
	command->redo();

	bool inMacro = !m_macros.empty();

	UndoCommand::SPtr current;
	if (inMacro)
	{
		auto macro = m_macros.back();
		if (!macro->m_commandChilds.empty())
			current = macro->m_commandChilds.back();
	}
	else
	{
		if (m_index > 0)
			current = m_commands[m_index - 1];
		// Clear all undone commands after the current index
		m_commands.erase(m_commands.begin() + m_index, m_commands.end());
		if (m_cleanIndex > m_index) // Clean state has been removed
			m_cleanIndex = -1;
	}

	bool tryToMerge = (current != nullptr)
		&& (current->id() != -1)
		&& (current->id() == command->id())
		&& (inMacro || m_index != m_cleanIndex);

	if (tryToMerge && current->mergeWith(command.get()))
	{
		if (!inMacro)
		{
			m_indexChangedSignal.run(m_index);
			m_canUndoChangedSignal.run(canUndo());
			m_undoTextChangedSignal.run(undoText());
			m_canRedoChangedSignal.run(canRedo());
			m_redoTextChangedSignal.run(redoText());
		}
	}
	else
	{
		if (inMacro)
			m_macros.back()->m_commandChilds.push_back(command);
		else
		{
			m_commands.push_back(command);
			checkStackLimit();
			changeIndex(m_index + 1);
		}
	}
}

void UndoStack::clear()
{
	if (m_commands.empty())
		return;

	bool wasClean = isClean();

	m_macros.clear();
	m_commands.clear();
	m_index = 0;
	m_cleanIndex = 0;

	m_indexChangedSignal.run(0);
	m_canUndoChangedSignal.run(false);
	m_undoTextChangedSignal.run("");
	m_canRedoChangedSignal.run(false);
	m_redoTextChangedSignal.run("");

	if (!wasClean)
		m_cleanChangedSignal.run(true);
}

void UndoStack::undo()
{
	if (!m_index)
		return;

	if (!m_macros.empty())
		return;

	int index = m_index - 1;
	m_commands[index]->undo();
	changeIndex(index);
}

void UndoStack::redo()
{
	if (m_index == m_commands.size())
		return;

	if (!m_macros.empty())
		return;

	m_commands[m_index]->redo();
	changeIndex(m_index + 1);
}

bool UndoStack::canUndo() const
{
	if (!m_macros.empty())
		return false;
	return m_index > 0;
}

bool UndoStack::canRedo() const
{
	if (!m_macros.empty())
		return false;
	return m_index < static_cast<int>(m_commands.size());
}

std::string UndoStack::undoText() const
{
	if (!m_macros.empty())
		return "";
	if (m_index > 0)
		return m_commands[m_index - 1]->text();
	return "";
}

std::string UndoStack::redoText() const
{
	if (!m_macros.empty())
		return "";
	if (m_index < static_cast<int>(m_commands.size()))
		return m_commands[m_index]->text();
	return "";
}

std::string UndoStack::text(int index) const
{
	if (index < 0 || index >= static_cast<int>(m_commands.size()))
		return "";
	return m_commands[index]->text();
}

void UndoStack::setClean()
{
	changeIndex(m_index, true);
}

bool UndoStack::isClean() const
{
	if (!m_macros.empty())
		return false;
	return m_cleanIndex == m_index;
}

void UndoStack::setUndoLimit(int limit)
{ 
	if (m_undoLimit == limit)
		return;

	m_undoLimit = limit; 
	checkStackLimit();
}

void UndoStack::beginMacro(const std::string& text)
{
	auto cmd = std::make_shared<UndoCommand>();
	cmd->setText(text);

	if (m_macros.empty())
	{
		// Clear all undone commands after the current index
		m_commands.erase(m_commands.begin() + m_index, m_commands.end());
		if (m_cleanIndex > m_index)
			m_cleanIndex = -1;
		m_commands.push_back(cmd);
	}
	else
		m_macros.back()->m_commandChilds.push_back(cmd);

	m_macros.push_back(cmd);

	if (m_macros.size() == 1)
	{
		m_canUndoChangedSignal.run(false);
		m_undoTextChangedSignal.run("");
		m_canRedoChangedSignal.run(false);
		m_redoTextChangedSignal.run("");
	}
}

void UndoStack::endMacro()
{
	m_macros.pop_back();
	if (m_macros.empty())
	{
		checkStackLimit();
		changeIndex(m_index + 1);
	}
}

void UndoStack::setIndex(int index)
{
	if (index < 0)
		index = 0;
	int nbCommands = static_cast<int>(m_commands.size());
	if (index > nbCommands)
		index = nbCommands;

	int i = m_index;
	while (i < index)
		m_commands[i++]->redo();
	while (i > index)
		m_commands[--i]->undo();

	changeIndex(index);
}

void UndoStack::checkStackLimit()
{
	if (m_undoLimit <= 0 || !m_macros.empty() || m_undoLimit >= static_cast<int>(m_commands.size()))
		return;

	int delCount = m_commands.size() - m_undoLimit;
	m_commands.erase(m_commands.begin(), m_commands.begin() + delCount);

	m_index -= delCount;
	if (m_cleanIndex != -1)
	{
		if (m_cleanIndex < delCount)
			m_cleanIndex = -1; // Clean state has been removed
		else
			m_cleanIndex -= delCount;
	}
}

void UndoStack::changeIndex(int index, bool clean)
{
	bool wasClean = (m_index == m_cleanIndex);
	if (index != m_index)
	{
		m_index = index;
		m_indexChangedSignal.run(index);
		m_canUndoChangedSignal.run(canUndo());
		m_undoTextChangedSignal.run(undoText());
		m_canRedoChangedSignal.run(canRedo());
		m_redoTextChangedSignal.run(redoText());
	}

	if (clean)
		m_cleanIndex = m_index;

	bool isClean = (m_index == m_cleanIndex);
	if (isClean != wasClean)
		m_cleanChangedSignal.run(isClean);
}

} // namespace panda
