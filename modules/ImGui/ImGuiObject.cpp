#include <panda/PandaDocument.h>
#include <panda/SimpleGUI.h>
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

class ImGui_Object : public DockObject
{
public:
	PANDA_CLASS(ImGui_Object, DockObject)

	ImGui_Object(PandaDocument* doc)
		: DockObject(doc)
	{
		m_observer.get(doc->getSignals().postRender).connect<ImGui_Object, &ImGui_Object::render>(this);
		m_observer.get(doc->getSignals().mouseMoveEvent).connect<ImGui_Object, &ImGui_Object::onMouseMove>(this);
		m_observer.get(doc->getSignals().mouseButtonEvent).connect<ImGui_Object, &ImGui_Object::onMouseButton>(this);

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
	}

	~ImGui_Object()
	{
		ImGui::Shutdown();
	}

	bool accepts(DockableObject* dockable) const override
	{
		return false;
	}

	void onMouseMove(panda::types::Point pt)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = { pt.x, pt.y };
	}

	void onMouseButton(int button, bool isPressed, panda::types::Point pt)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = { pt.x, pt.y };
		if(button >= 0 && button < 3)
			io.MouseDown[button] = isPressed;
	}

	void reset() override
	{
	}

	void createFontsTexture()
	{
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		m_fontTexture = graphics::Texture(graphics::Size(width, height), pixels);

		io.Fonts->TexID = reinterpret_cast<void*>(static_cast<intptr_t>(m_fontTexture.id()));
	}

	void initGui()
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
	}

	void renderGui(ImDrawData* draw_data)
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

	void testGui()
	{
		static float f = 0.0f;
		static ImVec4 clear_color = ImColor(114, 144, 154);
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("clear color", (float*)&clear_color);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	void render(int width, int height, int dstFbo)
	{
		if (!m_VAO)
			initGui();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));

		auto now = high_resolution_clock::now();
		duration<double> delta = m_lastTime - now;
		m_lastTime = now;
		auto dt = delta.count();
		io.DeltaTime = dt > 0 ? static_cast<float>(dt) : 1 / 60.f;

		ImGui::NewFrame();

		testGui();

		ImGui::Render();

		renderGui(ImGui::GetDrawData());

		parentDocument()->getGUI().updateView();
	}

protected:
	msg::Observer m_observer;

	high_resolution_clock::time_point m_lastTime;

	graphics::Texture m_fontTexture;
	graphics::ShaderProgram m_shader;
	graphics::VertexArrayObject m_VAO;
	graphics::Buffer m_VBO, m_EBO;

	int m_locationTex = -1, m_locationProjMtx = -1;
};

int ImGui_ObjectClass = RegisterObject<ImGui_Object>("ImGui/ImGui window").setDescription("Create an ImGui window");

panda::ModuleHandle imGuiModule = REGISTER_MODULE
		.setDescription("Components to create and show dear imgui windows")
		.setLicense("GPL")
		.setVersion("1.0");


} // namespace Panda
