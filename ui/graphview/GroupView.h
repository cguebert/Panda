#ifndef GROUPVIEW_H
#define GROUPVIEW_H

#include <ui/graphview/GraphView.h>

namespace panda
{
class Group;
}

class GroupView : public GraphView
{
	Q_OBJECT

public:
	explicit GroupView(panda::Group* group, panda::PandaDocument* doc, panda::ObjectsList& objectsList, QWidget* parent = nullptr);

	bool isTemporaryView() const override;

protected:
	void paintGL() override;

	std::pair<panda::BaseData*, panda::types::Rect> getDataAtPos(const panda::types::Point& pt) override;
	bool getDataRect(const panda::BaseData* data, panda::types::Rect& rect) override;
	std::pair<Rects, PointsPairs> getConnectedDatas(panda::BaseData* data) override;

	void updateLinks() override;
	void updateObjectsRect() override;
	void updateViewRect() override;

private:
	void updateGroupDataRects();

	static const int dataRectSize = 10;
	static const int dataMarginW = 100;
	static const int dataMarginH = 20;
	static const int tagW = 18;
	static const int tagH = 13;
	static const int tagMargin = 10;

	panda::Group* m_group;
	panda::types::Rect m_onlyObjectsRect; // Without the group datas
	std::vector<std::pair<panda::BaseData*, panda::types::Rect>> m_groupDataRects;
};

#endif // GROUPVIEW_H
