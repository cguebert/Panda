#ifndef LAYERDRAWSTRUCT_H
#define LAYERDRAWSTRUCT_H

#include <ui/drawstruct/DockableDrawStruct.h>

namespace panda
{
class Layer;
}

class GraphView;

class LayerDrawStruct : public DockObjectDrawStruct
{
public:
	LayerDrawStruct(GraphView* view, panda::Layer* object);

	virtual void drawText(QPainter* painter);

protected:
	panda::Layer* m_layer;
};

#endif
