#pragma once

#include <panda/graphview/object/ObjectRenderer.h>

namespace panda
{
class Group;
}

namespace graphview
{
	
class GraphView;

namespace object
{

class GroupRenderer : public ObjectRenderer
{
public:
	GroupRenderer(GraphView* view, panda::Group* object);

	void createShape() override;
	int dataStartY() override;

protected:
	panda::Group* m_group;
};

} // namespace object

} // namespace graphview
