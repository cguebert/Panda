#pragma once

#include <panda/document/PandaDocument.h>
#include <panda/messaging.h>

#include <panda/types/Color.h>
#include <panda/types/Point.h>

#include <panda/graphics/Size.h>

namespace panda 
{

class DocumentRenderer;
class Layer;

namespace graphics {
	class Framebuffer;
}

class PANDA_CORE_API RenderedDocument : public PandaDocument
{
public:
	PANDA_CLASS(RenderedDocument, PandaObject)

	explicit RenderedDocument(gui::BaseGUI& gui);
	~RenderedDocument();

	types::Color getBackgroundColor() const;
	void setBackgroundColor(types::Color color);

	graphics::Size getRenderSize() const;
	void setRenderSize(graphics::Size size);

	void update() override;

	graphics::Framebuffer& getFBO();

	Layer* getDefaultLayer() const;

	DocumentRenderer& getRenderer() const; // What takes care of rendering the document in OpenGL

protected:
	void objectsReordered();

	std::shared_ptr<Layer> m_defaultLayer;

	Data<types::Point> m_renderSize;
	Data<types::Color> m_backgroundColor;
	
	std::unique_ptr<DocumentRenderer> m_renderer;
	msg::Observer m_observer;
};

//****************************************************************************//

inline types::Color RenderedDocument::getBackgroundColor() const
{ return m_backgroundColor.getValue(); }

inline void RenderedDocument::setBackgroundColor(types::Color color)
{ m_backgroundColor.setValue(color); }

inline Layer* RenderedDocument::getDefaultLayer() const
{ return m_defaultLayer.get(); }

inline DocumentRenderer& RenderedDocument::getRenderer() const
{ return *m_renderer; }

} // namespace panda
