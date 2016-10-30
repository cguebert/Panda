#include <panda/graphview/object/GroupRenderer.h>
#include <panda/graphview/GraphView.h>

#include <panda/document/PandaDocument.h>
#include <panda/object/Group.h>

namespace graphview
{

namespace object
{

GroupRenderer::GroupRenderer(GraphView* view, panda::Group* object)
	: ObjectRenderer(view, object)
	, m_group(object)
{
}

void GroupRenderer::createShape()
{
	const int w = 9, h = 9;

	m_outline.clear();
	m_outline.moveTo(pPoint(m_visualArea.left() + w,  m_visualArea.top()));
	m_outline.lineTo(pPoint(m_visualArea.right() - w, m_visualArea.top()));
	m_outline.lineTo(pPoint(m_visualArea.right(),     m_visualArea.top() + h));
	m_outline.lineTo(pPoint(m_visualArea.right(),     m_visualArea.bottom() - h));
	m_outline.lineTo(pPoint(m_visualArea.right() - w, m_visualArea.bottom()));
	m_outline.lineTo(pPoint(m_visualArea.left() + w,  m_visualArea.bottom()));
	m_outline.lineTo(pPoint(m_visualArea.left(),      m_visualArea.bottom() - h));
	m_outline.lineTo(pPoint(m_visualArea.left(),      m_visualArea.top() + h));
	m_outline.lineTo(pPoint(m_visualArea.left() + w,  m_visualArea.top()));

	m_fillShape = m_outline.triangulate();
}

int GroupRenderer::dataStartY()
{
	return 10;
}

int GroupDrawClass = RegisterDrawObject<panda::Group, GroupRenderer>();

} // namespace object

} // namespace graphview
