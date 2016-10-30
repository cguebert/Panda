#include <panda/graphview/InteractionEvents.h>

namespace panda
{

namespace graphview
{

	InputEvent::InputEvent(EventModifiers modifiers)
		: m_modifiers(modifiers) 
	{}

	MouseEvent::MouseEvent(const panda::types::Point& pos, 
						   MouseButton button, 
						   EventModifiers modifiers)
		: InputEvent(modifiers)
		, m_pos(pos)
		, m_button(button)
	{}

	WheelEvent::WheelEvent(const panda::types::Point& pos, const panda::types::Point& angleDelta, EventModifiers modifiers)
		: InputEvent(modifiers)
		, m_pos(pos)
		, m_angleDelta(angleDelta)
	{}

	KeyEvent::KeyEvent(Key key, EventModifiers modifiers)
		: InputEvent(modifiers)
		, m_key(key)
	{}

	ContextMenuEvent::ContextMenuEvent(const panda::types::Point& pos, EventModifiers modifiers)
		: InputEvent(modifiers)
		, m_pos(pos)
	{}

} // namespace graphview

} // namespace panda
