#pragma once

#include <panda/types/Rect.h>

#include <memory>

class QOpenGLBuffer;
class QOpenGLShaderProgram;
class QOpenGLTexture;
class QOpenGLVertexArrayObject;

class DrawList;
class FontAtlas;
class Font;

class ViewRenderer
{
public:
	ViewRenderer();
	~ViewRenderer();

	void initialize();
	void resize(int w, int h);

	void setView(const panda::types::Rect& bounds);
	void newFrame();
	void addDrawList(DrawList* dl);
	void render();

	static bool initialized();
	static unsigned int defaultTextureId();
	static Font* currentFont();

private:
	std::unique_ptr<QOpenGLTexture> m_fontTexture;
	std::unique_ptr<QOpenGLShaderProgram> m_shader;
	std::unique_ptr<QOpenGLVertexArrayObject> m_VAO;
	std::unique_ptr<QOpenGLBuffer> m_VBO, m_EBO;
	
	int m_locationTex = -1, m_locationProjMtx = -1;
	int m_width = 0, m_height = 0;
	panda::types::Rect m_viewBounds;

	std::vector<DrawList*> m_drawLists;
	std::unique_ptr<FontAtlas> m_atlas;
};

inline void ViewRenderer::addDrawList(DrawList* dl)
{ m_drawLists.push_back(dl); }
