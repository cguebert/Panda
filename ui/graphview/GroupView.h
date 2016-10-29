#pragma once

#include <ui/graphview/GraphView.h>

#include <panda/types/Rect.h>

namespace panda {
	class Group;
}

namespace graphview
{

class GroupViewport;

class GroupView : public GraphView
{
	Q_OBJECT

public:
	using DataRect = std::pair<panda::BaseData*, panda::types::Rect>;
	using DataRects = std::vector<DataRect>;

	static std::unique_ptr<GroupView> createGroupView(panda::Group* group, panda::PandaDocument* doc, panda::ObjectsList& objectsList, MainWindow* mainWindow);

	panda::Group* group() const;

	static const int dataRectSize = 10;
	static const int dataMarginW = 100;
	static const int dataMarginH = 20;
	static const int tagW = 18;
	static const int tagH = 13;
	static const int tagMargin = 10;

	void updateGroupDataRects();
	const DataRects& groupDataRects() const;

	void createInputGroupData();
	void createOutputGroupData();
	void removeGroupData(panda::BaseData* data);

protected:
	void initializeRenderer(ViewRenderer& viewRenderer) override;
	void drawGraphView(ViewRenderer& viewRenderer, graphics::DrawColors drawColors) override;

private:
	GroupView(panda::Group* group, panda::PandaDocument* doc, panda::ObjectsList& objectsList, MainWindow* mainWindow);

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
