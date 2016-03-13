#pragma once

#include <panda/messaging.h>
#include <panda/types/Point.h>

#include <panda/graphics/Buffer.h>
#include <panda/graphics/Framebuffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/graphics/Texture.h>
#include <panda/graphics/VertexArrayObject.h>

#include <chrono>
#include <memory>

namespace panda 
{

class PandaDocument;

class ImGui_Wrapper
{
public:
	static std::shared_ptr<ImGui_Wrapper> instance(PandaDocument* doc);
	
	ImGui_Wrapper(PandaDocument* doc);
	~ImGui_Wrapper();

private:
	void createFontsTexture();
	void initGui();
	void renderGui();
	void update(int width, int height, int dstFbo);
	void updateObjects();

	void onMouseMove(panda::types::Point pt);
	void onMouseButton(int button, bool isPressed, panda::types::Point pt);
	void onKeyEvent(int key, bool isPressed);
	void onTextEvent(const std::string& text);

	PandaDocument* m_document;
	msg::Observer m_observer;

	bool m_mouseButtons[3];
	std::chrono::high_resolution_clock::time_point m_lastTime;

	graphics::Texture m_fontTexture;
	graphics::ShaderProgram m_shader;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_VBO, m_EBO;

	int m_locationTex = -1, m_locationProjMtx = -1;
};

} // namespace Panda
