#include <panda/Renderer.h>
#include <panda/PandaDocument.h>
#include <panda/Layer.h>

namespace panda
{

Renderer::Renderer(PandaDocument* doc)
	: DockableObject(doc)
{
	BaseData* data = doc->getData("render size");
	if(data) addInput(data);
}

void Renderer::setDirtyValue()
{
	DataNode::setDirtyValue();
	if(!isInStep)
		emit dirty(this);
}

DockObject* Renderer::getDefaultDock()
{
	return parentDocument->getDefaultLayer();
}

QMatrix4x4& Renderer::getMVPMatrix()
{
	BaseLayer* layer = dynamic_cast<BaseLayer*>(getParentDock());
	return layer->getMVPMatrix();
}

} // namespace panda
