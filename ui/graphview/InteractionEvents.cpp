#include <ui/graphview/InteractionEvents.h>

namespace graphview
{

	InputEvent::InputEvent(EventModifier modifiers)
		: m_modifiers(modifiers) 
	{}

	MouseEvent::MouseEvent(const panda::types::Point& pos, 
						   MouseButton button, 
						   EventModifier modifiers)
		: InputEvent(modifiers)
		, m_pos(pos)
		, m_button(button)
	{}

	WheelEvent::WheelEvent(const panda::types::Point& pos, const panda::types::Point& angleDelta, EventModifier modifiers)
		: InputEvent(modifiers)
		, m_pos(pos)
		, m_angleDelta(angleDelta)
	{}

	KeyEvent::KeyEvent(int key, EventModifier modifiers)
		: InputEvent(modifiers)
		, m_key(key)
	{}

	ContextMenuEvent::ContextMenuEvent(const panda::types::Point& pos, EventModifier modifiers)
		: InputEvent(modifiers)
		, m_pos(pos)
	{}

} // namespace graphview
