#pragma once

#include <panda/graphview/GraphView.h>

#include <panda/types/Rect.h>

namespace panda {
	class Group;
}

namespace graphview
{

class GroupViewport;

class PANDA_CORE_API GroupView : public GraphView
{
public:
	using DataRect = std::pair<panda::BaseData*, panda::types::Rect>;
	using DataRects = std::vector<DataRect>;

	static std::unique_ptr<GroupView> createGroupView(panda::Group* group, panda::PandaDocument* doc, panda::ObjectsList& objectsList);

	panda::Group* group() const;

	void updateGroupDataRects();
	const DataRects& groupDataRects() const;

	void createInputGroupData();
	void createOutputGroupData();
	void removeGroupData(panda::BaseData* data);

protected:
	void initializeRenderer(ViewRenderer& viewRenderer) override;
	void drawGraphView(ViewRenderer& viewRenderer, graphics::DrawColors drawColors) override;

private:
	GroupView(panda::Group* group, panda::PandaDocument* doc, panda::ObjectsList& objectsList);

	void modifiedObject(panda::PandaObject* object);

	panda::Group* m_group;

	DataRects m_groupDataRects;

	std::shared_ptr<graphics::DrawList> m_groupDrawList;
};

//****************************************************************************//

inline panda::Group* GroupView::group() const
{ return m_group; }

inline const GroupView::DataRects& GroupView::groupDataRects() const
{ return m_groupDataRects; }

} // namespace graphview
