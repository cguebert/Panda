#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <panda/PandaDocument.h>
#include <panda/PluginsManager.h>
#include <panda/document/DocumentRenderer.h>
#include <panda/helper/system/FileRepository.h>

#include "SimpleGUIImpl.h"

#include <iostream>
#include <boost/filesystem.hpp>

GLFWwindow* theWindow = nullptr;
std::shared_ptr<SimpleGUIImpl> gui;
std::shared_ptr<panda::PandaDocument> document;
int currentWidth = 800, currentHeight = 600;
panda::types::Point mousePos;

void error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error: " << description << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void mouse_pos_callback(GLFWwindow* window, double x, double y)
{
	mousePos = panda::types::Point(static_cast<float>(x), static_cast<float>(y));
	document->setMousePosition(mousePos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if(action == GLFW_PRESS)
			document->setMouseClick(true, mousePos);
		else if(action == GLFW_RELEASE)
			document->setMouseClick(false, mousePos);
	}
}

void drop_callback(GLFWwindow* window, int count, const char** paths)
{
	if (!count)
		return;

	document->resetDocument();
	document->readFile(paths[0]);
	document->clearCommands();
	document->selectNone();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	document->getRenderer()->resizeGL(width, height);
	currentWidth = width;
	currentHeight = height;
}

bool init()
{
	const GLuint width = 800, height = 600;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	theWindow = glfwCreateWindow(width, height, "Panda Viewer", nullptr, nullptr);
	if (!theWindow)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(theWindow);
	glfwSetKeyCallback(theWindow, key_callback);
	glfwSetCursorPosCallback(theWindow, mouse_pos_callback);
	glfwSetMouseButtonCallback(theWindow, mouse_button_callback);
	glfwSetDropCallback(theWindow, drop_callback);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return false;
	}

	auto& dataRepository = panda::helper::system::DataRepository;
	dataRepository.addPath(boost::filesystem::current_path().string());

#ifdef WIN32
	dataRepository.addPath("C:/Windows/Fonts");
#endif

	panda::PluginsManager::getInstance()->loadPlugins();

	gui = std::make_shared<SimpleGUIImpl>();
	document = std::make_shared<panda::PandaDocument>(*gui);
	auto renderer = document->getRenderer();
	renderer->initializeGL();
	renderer->resizeGL(width, height);
	renderer->setRenderingMainView(true);

	return true;
}

int main(int argc, char** argv)
{
	if (!init())
		return -1;

	while (!glfwWindowShouldClose(theWindow))
	{
		glfwPollEvents();
		gui->executeFunctions();

		document->step();
		document->updateIfDirty();
		auto fbo = document->getFBO();

		panda::graphics::RectInt rect(0, 0, currentWidth, currentHeight);
		panda::graphics::Framebuffer::blitFramebuffer(0, rect, fbo.id(), rect);

		glfwSwapBuffers(theWindow);
	}

	document.reset();
	glfwTerminate();
	return 0;
}