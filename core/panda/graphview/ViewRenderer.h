#pragma once

#include <panda/types/Rect.h>

#include <memory>

namespace panda
{

namespace graphview
{

namespace graphics 
{
	class DrawList;
	class Font;
}

class PANDA_CORE_API ViewRenderer
{
public:
	using DrawListSPtr = std::shared_ptr<graphics::DrawList>;

	virtual ~ViewRenderer();

	virtual void initialize() = 0;
	virtual void resize(int w, int h) = 0;

	virtual void setView(const panda::types::Rect& bounds) = 0;
	virtual void newFrame() = 0;
	virtual void addDrawList(const DrawListSPtr& dl) = 0;
	virtual void render() = 0;

	virtual void setClearColor(float r, float g, float b) = 0;

	virtual bool initialized() const = 0;
	virtual unsigned int defaultTextureId() const = 0;
	virtual graphics::Font* currentFont() const = 0;
};

} // namespace graphview

} // namespace panda
