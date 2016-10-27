#pragma once

#include <panda/types/Rect.h>

#include <memory>
#include <vector>

class QOpenGLBuffer;
class QOpenGLShaderProgram;
class QOpenGLTexture;
class QOpenGLVertexArrayObject;

namespace graphview
{

namespace graphics 
{
	class DrawList;
	class FontAtlas;
	class Font;
}

class ViewRenderer
{
public:
	using DrawListSPtr = std::shared_ptr<graphics::DrawList>;

	ViewRenderer();
	~ViewRenderer();

	void initialize();
	void resize(int w, int h);

	void setView(const panda::types::Rect& bounds);
	void newFrame();
	void addDrawList(const DrawListSPtr& dl);
	void render();

	bool initialized();
	unsigned int defaultTextureId();
	graphics::Font* currentFont();

private:
	std::unique_ptr<QOpenGLTexture> m_fontTexture;
	std::unique_ptr<QOpenGLShaderProgram> m_shader;
	std::unique_ptr<QOpenGLVertexArrayObject> m_VAO;
	std::unique_ptr<QOpenGLBuffer> m_VBO, m_EBO;
	
	int m_locationTex = -1, m_locationProjMtx = -1;
	int m_width = 0, m_height = 0;
	panda::types::Rect m_viewBounds;

	std::vector<DrawListSPtr> m_drawLists;
	std::unique_ptr<graphics::FontAtlas> m_atlas;
};

inline void ViewRenderer::addDrawList(const DrawListSPtr& dl)
{ if(dl) m_drawLists.push_back(dl); }

} // namespace graphview

