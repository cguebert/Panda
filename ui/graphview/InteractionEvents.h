#pragma once

#include <panda/helper/bitmask_operators.hpp>

#include <panda/types/Point.h>

namespace graphview
{

	enum MouseButton
	{
		NoButton     = 0,
		LeftButton   = 1,
		RightButton  = 2,
		MiddleButton = 4
	};

	// Can use MouseButton as a flag
	template<> struct enable_bitmask_operators<MouseButton>{
		static constexpr bool enable = true;
	};

	enum EventModifier
	{
		NoModifier      = 0,
		ShiftModifier   = 1,
		ControlModifier = 2,
		AltModifier     = 4
	};

	// Can use EventModifier as a flag
	template<> struct enable_bitmask_operators<EventModifier>{
		static constexpr bool enable = true;
	};

	class InputEvent
	{
	public:
		InputEvent(EventModifier modifiers);

		EventModifier modifiers() const;

	private:
		EventModifier m_modifiers;
	};

	class MouseEvent : public InputEvent
	{
	public:
		MouseEvent(const panda::types::Point& pos, MouseButton button, EventModifier modifiers);

		panda::types::Point pos() const;
		MouseButton button() const;

	private:
		panda::types::Point m_pos;
		MouseButton m_button;
	};

	class WheelEvent : public InputEvent
	{
	public:
		WheelEvent(const panda::types::Point& pos, const panda::types::Point& angleDelta, EventModifier modifiers);

		panda::types::Point pos() const;
		panda::types::Point angleDelta() const;

	private:
		panda::types::Point m_pos, m_angleDelta;
	};

	class KeyEvent : public InputEvent
	{
	public:
		KeyEvent(int key, EventModifier modifiers);

		int key() const;

	private:
		int m_key;
	};

	class ContextMenuEvent : public InputEvent
	{
	public:
		ContextMenuEvent(const panda::types::Point& pos, EventModifier modifiers);

		panda::types::Point pos() const;

	private:
		panda::types::Point m_pos;
	};

//****************************************************************************//

	inline EventModifier InputEvent::modifiers() const
	{ return m_modifiers; }

	inline panda::types::Point MouseEvent::pos() const
	{ return m_pos; }

	inline MouseButton MouseEvent::button() const
	{ return m_button; }

	inline panda::types::Point WheelEvent::pos() const
	{ return m_pos; }

	inline panda::types::Point WheelEvent::angleDelta() const
	{ return m_angleDelta; }

	inline int KeyEvent::key() const
	{ return m_key; }

	inline panda::types::Point ContextMenuEvent::pos() const
	{ return m_pos; }

} // namespace graphview

