#ifndef GROUPOBJECTDRAWSTRUCT_H
#define GROUPOBJECTDRAWSTRUCT_H

#include <ui/drawstruct/ObjectDrawStruct.h>

namespace panda
{
class Group;
}

class GraphView;

class GroupDrawStruct : public ObjectDrawStruct
{
public:
	GroupDrawStruct(GraphView* view, panda::Group* object);

	void drawShape(DrawList& list, DrawColors& colors) override;
	std::string getLabel() const override;

	void moveVisual(const QPointF& delta) override;
	void update() override;
	bool contains(const QPointF& point) override;
	int dataStartY() override;

protected:
	DrawPath m_shapePath;
	panda::Group* m_group;
};

#endif
