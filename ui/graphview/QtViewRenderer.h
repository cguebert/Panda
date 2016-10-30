#pragma once

#include <panda/graphview/ViewRenderer.h>

#include <vector>

class QOpenGLBuffer;
class QOpenGLShaderProgram;
class QOpenGLTexture;
class QOpenGLVertexArrayObject;

namespace panda {
	namespace graphview {
		namespace graphics
		{
			class DrawList;
			class FontAtlas;
			class Font;
		}
	}
}

namespace graphview
{

class QtViewRenderer : public panda::graphview::ViewRenderer
{
public:
	QtViewRenderer();
	~QtViewRenderer();

	void initialize() override;
	void resize(int w, int h) override;

	void setView(const panda::types::Rect& bounds) override;
	void newFrame() override;
	void addDrawList(const DrawListSPtr& dl) override;
	void render() override;

	void setClearColor(float r, float g, float b) override;

	bool initialized() const override;
	unsigned int defaultTextureId() const override;
	panda::graphview::graphics::Font* currentFont() const override;

private:
	std::unique_ptr<QOpenGLTexture> m_fontTexture;
	std::unique_ptr<QOpenGLShaderProgram> m_shader;
	std::unique_ptr<QOpenGLVertexArrayObject> m_VAO;
	std::unique_ptr<QOpenGLBuffer> m_VBO, m_EBO;
	
	int m_locationTex = -1, m_locationProjMtx = -1;
	int m_width = 0, m_height = 0;
	panda::types::Rect m_viewBounds;

	std::vector<DrawListSPtr> m_drawLists;
	std::unique_ptr<panda::graphview::graphics::FontAtlas> m_atlas;

	float m_clearColor[4];
};

inline void QtViewRenderer::addDrawList(const DrawListSPtr& dl)
{ if(dl) m_drawLists.push_back(dl); }

} // namespace graphview

