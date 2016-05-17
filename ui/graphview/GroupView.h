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

	void moveObjects(std::vector<panda::PandaObject*> objects, panda::types::Point delta) override;

	bool isTemporaryView() const override;

protected:
	void paintGL() override;

	std::pair<panda::BaseData*, panda::types::Rect> getDataAtPos(const panda::types::Point& pt) override;
	bool getDataRect(const panda::BaseData* data, panda::types::Rect& rect) override;
	std::pair<Rects, PointsPairs> getConnectedDatas(panda::BaseData* data) override;

private:
	panda::Group* m_group;

	std::vector<std::pair<panda::BaseData*, panda::types::Rect>> m_groupDataRects;
};

#endif // GROUPVIEW_H
