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

	void createShape() override;
	int dataStartY() override;

protected:
	panda::Group* m_group;
};

#endif
