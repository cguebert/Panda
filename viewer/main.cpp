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
bool verticalSync = true;

void error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error: " << description << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_F:
			std::cout << "FPS: " << document->getFPS() << std::endl;
			break;
		case GLFW_KEY_V:
			verticalSync = !verticalSync;
			glfwSwapInterval(verticalSync ? 1 : 0);
			break;
		case GLFW_KEY_F5:
			document->play(!document->animationIsPlaying());
			break;
		case GLFW_KEY_F6:
			if(!document->animationIsPlaying())
				document->step();
			break;
		case GLFW_KEY_F7:
			document->rewind();
			break;
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		}
	}
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
	document->setRenderSize({ currentWidth, currentHeight });
	document->play(true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	document->setRenderSize({ width, height });
	document->getRenderer()->resizeGL(width, height);
	currentWidth = width;
	currentHeight = height;
}

bool init(const std::string& filePath = "")
{
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	currentWidth = 800;
	currentHeight = 600;
	GLFWmonitor* monitor = nullptr;
	if (!filePath.empty())
	{
		monitor = glfwGetPrimaryMonitor();
		if (monitor)
		{
			const auto videoMode = glfwGetVideoMode(monitor);
			glfwWindowHint(GLFW_RED_BITS, videoMode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, videoMode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, videoMode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);
			currentWidth = videoMode->width;
			currentHeight = videoMode->height;
		}
	}

	theWindow = glfwCreateWindow(currentWidth, currentHeight, "Panda Viewer", monitor, nullptr);
	if (!theWindow)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(theWindow);
	glfwSetKeyCallback(theWindow, key_callback);
	glfwSetCursorPosCallback(theWindow, mouse_pos_callback);
	glfwSetMouseButtonCallback(theWindow, mouse_button_callback);
	glfwSetDropCallback(theWindow, drop_callback);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
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
	
	if (!filePath.empty())
	{
		document->readFile(filePath);
		document->play(true);
	}

	document->setRenderSize({ currentWidth, currentHeight }); // Loading the file has changed the render size
	renderer->resizeGL(currentWidth, currentHeight);
	renderer->setRenderingMainView(true);

	return true;
}

int main(int argc, char** argv)
{
	std::string filePath;
	if (argc > 1)
		filePath = argv[1];

	if (!init(filePath))
		return -1;

	while (!glfwWindowShouldClose(theWindow))
	{
		glfwPollEvents();
		gui->executeFunctions();

		document->updateIfDirty();
		auto fbo = document->getFBO();

		panda::graphics::RectInt rect(0, 0, currentWidth, currentHeight);
		panda::graphics::Framebuffer::blitFramebuffer(0, rect, fbo.id(), rect);

		glfwSwapBuffers(theWindow);
	}

	document.reset();
	glfwDestroyWindow(theWindow);
	glfwTerminate();
	return 0;
}