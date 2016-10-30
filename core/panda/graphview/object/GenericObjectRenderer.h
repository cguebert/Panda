#pragma once

#include <panda/graphview/object/ObjectRenderer.h>

namespace panda
{
class GenericObject;
}

namespace graphview
{

namespace object
{

class GenericObjectRenderer : public ObjectRenderer
{
public:
	GenericObjectRenderer(GraphView* view, panda::GenericObject* m_object);

	void update() override;
	void drawDatas(graphics::DrawList& list, graphics::DrawColors& colors) override;
	panda::types::Point getObjectSize() override;

	static const int createdDataRectMargin = 3;

protected:
	panda::GenericObject* m_genericObject;
	int m_nbDefInputs, m_nbDefOutputs;
};

} // namespace object

} // namespace graphview
