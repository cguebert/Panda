#include "ImGuiWrapper.h"
#include "ImGuiBase.h"

#include <panda/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/TimedFunctions.h>
#include <panda/document/DocumentSignals.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Dockable.h>

#include <panda/graphics/Buffer.h>
#include <panda/graphics/Framebuffer.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/graphics/Texture.h>
#include <panda/graphics/VertexArrayObject.h>

#include "imgui/imgui.h"

#include <chrono>
#include <memory>
#include <GL/glew.h>

using namespace std::chrono;

namespace panda 
{

std::shared_ptr<ImGui_Wrapper> ImGui_Wrapper::instance(PandaDocument* doc)
{
	static std::weak_ptr<ImGui_Wrapper> ptr;
	if (ptr.expired())
	{
		auto wrapper = std::shared_ptr<ImGui_Wrapper>(new ImGui_Wrapper(doc));
		ptr = wrapper;
		return wrapper;
	}
	else
		return ptr.lock();
}

ImGui_Wrapper::ImGui_Wrapper(PandaDocument* doc)
	: m_document(doc)
{
	auto& signals = doc->getSignals();
	m_observer.get(signals.postRender).connect<ImGui_Wrapper, &ImGui_Wrapper::update>(this);
	m_observer.get(signals.mouseMoveEvent).connect<ImGui_Wrapper, &ImGui_Wrapper::onMouseMove>(this);
	m_observer.get(signals.mouseButtonEvent).connect<ImGui_Wrapper, &ImGui_Wrapper::onMouseButton>(this);
	m_observer.get(signals.keyEvent).connect<ImGui_Wrapper, &ImGui_Wrapper::onKeyEvent>(this);
	m_observer.get(signals.textEvent).connect<ImGui_Wrapper, &ImGui_Wrapper::onTextEvent>(this);

	m_mouseButtons[0] = m_mouseButtons[1] = m_mouseButtons[2] = 0;

	ImGuiIO& io = ImGui::GetIO();
	io.KeyMap[ImGuiKey_Tab] = 258;
	io.KeyMap[ImGuiKey_LeftArrow] = 263;
	io.KeyMap[ImGuiKey_RightArrow] = 262;
	io.KeyMap[ImGuiKey_UpArrow] = 265;
	io.KeyMap[ImGuiKey_DownArrow] = 264;
	io.KeyMap[ImGuiKey_PageUp] = 266;
	io.KeyMap[ImGuiKey_PageDown] = 267;
	io.KeyMap[ImGuiKey_Home] = 268;
	io.KeyMap[ImGuiKey_End] = 269;
	io.KeyMap[ImGuiKey_Delete] = 261;
	io.KeyMap[ImGuiKey_Backspace] = 259;
	io.KeyMap[ImGuiKey_Enter] = 257;
	io.KeyMap[ImGuiKey_Escape] = 256;
	io.KeyMap[ImGuiKey_A] = 0x41;
	io.KeyMap[ImGuiKey_C] = 0x43;
	io.KeyMap[ImGuiKey_V] = 0x56;
	io.KeyMap[ImGuiKey_X] = 0x58;
	io.KeyMap[ImGuiKey_Y] = 0x59;
	io.KeyMap[ImGuiKey_Z] = 0x5a;

	io.IniFilename = nullptr;
}

ImGui_Wrapper::~ImGui_Wrapper()
{
	ImGui::Shutdown();
}

void ImGui_Wrapper::onMouseMove(panda::types::Point /*localPt*/, panda::types::Point globalPt)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = { globalPt.x, globalPt.y };
}

void ImGui_Wrapper::onMouseButton(int button, bool isPressed, panda::types::Point /*pt*/)
{
	ImGuiIO& io = ImGui::GetIO();
		
	if (button >= 0 && button < 3)
	{
		if (isPressed)
			io.MouseDown[button] = true;
		m_mouseButtons[button] = isPressed;
	}
}

void ImGui_Wrapper::onKeyEvent(int key, bool isPressed)
{
	ImGuiIO& io = ImGui::GetIO();
	if (key >= 0 && key < 512)
		io.KeysDown[key] = isPressed;

	io.KeyShift = io.KeysDown[340] || io.KeysDown[344];
	io.KeyCtrl = io.KeysDown[341] || io.KeysDown[345];
	io.KeyAlt = io.KeysDown[342] || io.KeysDown[346];
}

void ImGui_Wrapper::onTextEvent(const std::string& text)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharactersUTF8(text.c_str());
}

void ImGui_Wrapper::createFontsTexture()
{
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	m_fontTexture = graphics::Texture(graphics::Size(width, height), pixels);

	io.Fonts->TexID = reinterpret_cast<void*>(static_cast<intptr_t>(m_fontTexture.id()));
}

void ImGui_Wrapper::initGui()
{
	createFontsTexture();

	const GLchar *vertex_shader =
		"#version 330\n"
		"uniform mat4 ProjMtx;\n"
		"layout (location = 0) in vec2 Position;\n"
		"layout (location = 1) in vec2 UV;\n"
		"layout (location = 2) in vec4 Color;\n"
		"out vec2 Frag_UV;\n"
		"out vec4 Frag_Color;\n"
		"void main()\n"
		"{\n"
		"	Frag_UV = UV;\n"
		"	Frag_Color = Color;\n"
		"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
		"}\n";

	const GLchar* fragment_shader =
		"#version 330\n"
		"uniform sampler2D Texture;\n"
		"in vec2 Frag_UV;\n"
		"in vec4 Frag_Color;\n"
		"out vec4 Out_Color;\n"
		"void main()\n"
		"{\n"
		"	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
		"}\n";

	m_shader.addShaderFromMemory(graphics::ShaderType::Vertex, vertex_shader);
	m_shader.addShaderFromMemory(graphics::ShaderType::Fragment, fragment_shader);
	m_shader.link();

	m_locationTex = m_shader.uniformLocation("Texture");
	m_locationProjMtx = m_shader.uniformLocation("ProjMtx");

	m_VAO.create();
	m_VAO.bind();

	m_VBO.create(graphics::BufferType::ArrayBuffer, graphics::BufferUsage::StreamDraw);
	m_VBO.bind();
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, pos));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, uv));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, col));
	glEnableVertexAttribArray(2);

	m_EBO.create(graphics::BufferType::ElementArrayBuffer, graphics::BufferUsage::StreamDraw);
	m_EBO.bind();

	m_VAO.release();

	m_lastTime = high_resolution_clock::now();
}

void ImGui_Wrapper::renderGui()
{
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glActiveTexture(GL_TEXTURE0);

	ImGuiIO& io = ImGui::GetIO();
	int fb_width = static_cast<GLsizei>(io.DisplaySize.x);
	int fb_height = static_cast<GLsizei>(io.DisplaySize.y);
	glViewport(0, 0, fb_width, fb_height);
	const float ortho_projection[4][4] =
	{
		{ 2.0f/io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
		{ 0.0f,                  2.0f/-io.DisplaySize.y, 0.0f, 0.0f },
		{ 0.0f,                  0.0f,                  -1.0f, 0.0f },
		{-1.0f,                  1.0f,                   0.0f, 1.0f },
	};

	m_shader.bind();
	m_shader.setUniformValue(m_locationTex, 0);
	m_shader.setUniformValueMat4(m_locationProjMtx, &ortho_projection[0][0]);

	m_VAO.bind();

	const auto draw_data = ImGui::GetDrawData();
	for (int n = 0; n < draw_data->CmdListsCount; ++n)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawIdx* idx_buffer_offset = nullptr;

		m_VBO.bind();
		m_VBO.write(0, &cmd_list->VtxBuffer.front(), cmd_list->VtxBuffer.size() * sizeof(ImDrawVert));

		m_EBO.bind();
		m_EBO.write(0, &cmd_list->IdxBuffer.front(), cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx));

		for (const ImDrawCmd& pcmd : cmd_list->CmdBuffer)
		{
			if (pcmd.UserCallback)
				pcmd.UserCallback(cmd_list, &pcmd);
			else
			{
				glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(reinterpret_cast<intptr_t>(pcmd.TextureId)));
				glScissor(static_cast<int>(pcmd.ClipRect.x), fb_height - static_cast<int>(pcmd.ClipRect.w), 
					static_cast<int>(pcmd.ClipRect.z - pcmd.ClipRect.x), static_cast<int>(pcmd.ClipRect.w - pcmd.ClipRect.y));
				glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(pcmd.ElemCount), sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
			}
			idx_buffer_offset += pcmd.ElemCount;
		}
	}

	m_shader.release();
	m_VAO.release();

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
}

void ImGui_Wrapper::updateObjects()
{
	for (auto object : m_document->getObjects())
	{
		auto dockable = dynamic_cast<BaseImGuiContainer*>(object.get());
		if (dockable)
			dockable->fillGui();
	}
}

void ImGui_Wrapper::update(int width, int height, int dstFbo)
{
	if (!m_VAO)
		initGui();

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));

	auto now = high_resolution_clock::now();
	duration<double> delta = now - m_lastTime;
	m_lastTime = now;
	auto dt = delta.count();
	io.DeltaTime = dt > 0 ? static_cast<float>(dt) : 1 / 60.f;

	ImGui::NewFrame();

	updateObjects();

	ImGui::Render();

	renderGui();

	if (!m_document->animationIsPlaying())
	{
		auto& gui = m_document->getGUI();
		TimedFunctions::instance().delayRun(1 / 60.0, [&gui]() {
			gui.updateView();
		});
	}

	for (int i = 0; i < 3; ++i)
		io.MouseDown[i] = m_mouseButtons[i];
}

panda::ModuleHandle imGuiModule = REGISTER_MODULE
	.setDescription("Components to create and show dear imgui windows")
	.setLicense("GPL")
	.setVersion("1.0");

} // namespace Panda
