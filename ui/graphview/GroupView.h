#pragma once

#include <ui/graphview/GraphView.h>

namespace panda
{
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

	explicit GroupView(panda::Group* group, panda::PandaDocument* doc, panda::ObjectsList& objectsList, QWidget* parent = nullptr);

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
