#ifndef GENERICOBJECTDRAWSTRUCT_H
#define GENERICOBJECTDRAWSTRUCT_H

#include <ui/graphview/object/ObjectRenderer.h>

namespace panda
{
class GenericObject;
}

class GenericObjectRenderer : public ObjectRenderer
{
public:
	GenericObjectRenderer(GraphView* view, panda::GenericObject* m_object);

	void update() override;
	void drawDatas(DrawList& list, DrawColors& colors) override;
	panda::types::Point getObjectSize() override;

	static const int createdDataRectMargin = 3;

protected:
	panda::GenericObject* m_genericObject;
	int m_nbDefInputs, m_nbDefOutputs;
};

#endif
