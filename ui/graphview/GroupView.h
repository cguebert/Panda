#pragma once

#include <ui/graphview/GraphView.h>

namespace panda
{
class Group;
}

namespace graphview
{

class GroupView : public GraphView
{
	Q_OBJECT

public:
	explicit GroupView(panda::Group* group, panda::PandaDocument* doc, panda::ObjectsList& objectsList, QWidget* parent = nullptr);

	bool isTemporaryView() const override;

	bool getDataRect(const panda::BaseData* data, panda::types::Rect& rect) override;

protected:
	void paintGL() override;
	void contextMenuEvent(QContextMenuEvent* event) override;

	std::pair<panda::BaseData*, panda::types::Rect> getDataAtPos(const panda::types::Point& pt) override;
	std::pair<Rects, PointsPairs> getConnectedDatas(panda::BaseData* data) override;
	bool isCompatible(const panda::BaseData* data1, const panda::BaseData* data2) override;
	void computeCompatibleDatas(panda::BaseData* data) override;

	void updateLinks() override;
	void updateObjectsRect() override;
	void updateViewRect() override;

	bool createLink(panda::BaseData* data1, panda::BaseData* data2) override;

private:
	void updateGroupDataRects();

	void createInputGroupData();
	void createOutputGroupData();
	void removeGroupData(panda::BaseData* data);

	void modifiedObject(panda::PandaObject* object);

	static const int dataRectSize = 10;
	static const int dataMarginW = 100;
	static const int dataMarginH = 20;
	static const int tagW = 18;
	static const int tagH = 13;
	static const int tagMargin = 10;

	panda::Group* m_group;
	panda::types::Rect m_onlyObjectsRect; // Without the group datas

	using DataRect = std::pair<panda::BaseData*, panda::types::Rect>;
	std::vector<DataRect> m_groupDataRects;
};

} // namespace graphview
