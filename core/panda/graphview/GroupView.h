#pragma once

#include <panda/graphview/GraphView.h>

#include <panda/types/Rect.h>

namespace panda 
{
	
class BaseData;
class Group;

namespace graphview
{

class GroupViewport;

class PANDA_CORE_API GroupView : public GraphView
{
public:
	using DataRect = std::pair<BaseData*, types::Rect>;
	using DataRects = std::vector<DataRect>;

	static std::unique_ptr<GroupView> createGroupView(Group* group, PandaDocument* doc, ObjectsList& objectsList);

	Group* group() const;

	void updateGroupDataRects();
	const DataRects& groupDataRects() const;

protected:
	void initializeRenderer(ViewRenderer& viewRenderer) override;
	void drawGraphView(ViewRenderer& viewRenderer, graphics::DrawColors drawColors) override;

private:
	GroupView(Group* group, PandaDocument* doc, ObjectsList& objectsList);

	void modifiedObject(PandaObject* object);

	Group* m_group;

	DataRects m_groupDataRects;

	std::shared_ptr<graphics::DrawList> m_groupDrawList;
};

//****************************************************************************//

inline Group* GroupView::group() const
{ return m_group; }

inline const GroupView::DataRects& GroupView::groupDataRects() const
{ return m_groupDataRects; }

} // namespace graphview

} // namespace panda
