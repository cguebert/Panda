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

DockObject* Renderer::getDefaultDock()
{
	return parentDocument->getDefaultLayer();
}

QMatrix4x4& Renderer::getMVPMatrix()
{
	BaseDrawTarget* drawTarget = dynamic_cast<BaseDrawTarget*>(getParentDock());
	return drawTarget->getMVPMatrix();
}

} // namespace panda
