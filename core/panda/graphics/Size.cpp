#include <panda/graphics/Size.h>

namespace panda
{

namespace graphics
{

Size Size::scaledInside(const Size& s) const
{
	if (empty())
		return s;

	int rWidth = s.m_height * m_width / m_height;
	if (rWidth <= s.m_width)
		return Size(rWidth, s.m_height);
	else
	{
		int rHeight = s.m_height * m_height / m_width;
		return Size(s.m_width, rHeight);
	}
}

Size Size::scaledOutside(const Size& s) const
{
	if (empty())
		return s;

	int rWidth = s.m_height * m_width / m_height;
	if (rWidth >= s.m_width)
		return Size(rWidth, s.m_height);
	else
	{
		int rHeight = s.m_height * m_height / m_width;
		return Size(s.m_width, rHeight);
	}
}

} // namespace graphics

} // namespace panda

