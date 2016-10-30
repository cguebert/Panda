#pragma once

#include <panda/graphview/object/ObjectRenderer.h>

namespace panda
{

class Group;

namespace graphview
{
	
class GraphView;

namespace object
{

class GroupRenderer : public ObjectRenderer
{
public:
	GroupRenderer(GraphView* view, Group* object);

	void createShape() override;
	int dataStartY() override;

protected:
	Group* m_group;
};

} // namespace object

} // namespace graphview

} // namespace panda
