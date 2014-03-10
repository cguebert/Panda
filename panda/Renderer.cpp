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

} // namespace panda
