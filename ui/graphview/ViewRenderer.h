#pragma once

#include <panda/types/Point.h>

#include <memory>

class QOpenGLBuffer;
class QOpenGLShaderProgram;
class QOpenGLTexture;
class QOpenGLVertexArrayObject;

class DrawList;

class ViewRenderer
{
public:
	ViewRenderer();
	~ViewRenderer();

	void initialize();
	void resize(int w, int h);

	void setView(float left, float top, float right, float bottom);
	void newFrame();
	void addDrawList(DrawList* dl);
	void render();

	static unsigned int defaultTextureId();

private:
	std::unique_ptr<QOpenGLTexture> m_fontTexture;
	std::unique_ptr<QOpenGLShaderProgram> m_shader;
	std::unique_ptr<QOpenGLVertexArrayObject> m_VAO;
	std::unique_ptr<QOpenGLBuffer> m_VBO, m_EBO;

	int m_locationTex = -1, m_locationProjMtx = -1;
	int m_width = 0, m_height = 0;
	float m_viewBounds[4];

	std::vector<DrawList*> m_drawLists;
};

inline void ViewRenderer::addDrawList(DrawList* dl)
{ m_drawLists.push_back(dl); }
