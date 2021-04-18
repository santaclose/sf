#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include <Camera.h>
#include <Mesh.h>
#include <Skybox.h>
#include <Input.h>
#include <Config.h>
#include <Game.h>
#include <Defaults.h>

#define BACKGROUND_COLOR 0.1

float gameTime = 0.0;
double lastFrameTime = 0.0;
double currentFrameTime = 0.0;
double deltaTime = 0.0;
bool deltaTimeLock = true;

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	sf::Input::UpdateMousePosition(xpos, ypos);
}
void mouse_button_callback(GLFWwindow*, int button, int action, int mods)
{
	sf::Input::UpdateMouseButtons(button, action);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	sf::Input::UpdateKeyboard(key, action);
}
void character_callback(GLFWwindow* window, unsigned int codepoint)
{
	sf::Input::UpdateCharacter(codepoint);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	sf::Input::UpdateMouseScroll(xoffset, yoffset);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	sf::Config::windowWidth = width;
	sf::Config::windowHeight = height;
	sf::Camera::aspectRatio = (float)width / (float)height;
}

int main(int argc, char** argv)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_SAMPLES, sf::Config::msaaCount);

	window = glfwCreateWindow(sf::Config::windowWidth, sf::Config::windowHeight, sf::Config::name.c_str(), sf::Config::fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	/* INPUT BINDINGS */
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCharCallback(window, character_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context (GLAD)" << std::endl;
		return -1;
	}

	// Get GPU info and supported OpenGL version
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "OpenGL version supported " << glGetString(GL_VERSION) << std::endl;

	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glClearColor(sf::Config::clearColor[0], sf::Config::clearColor[1], sf::Config::clearColor[2], 0.0);

	sf::Defaults::Initialize();
	//-------------------//
	sf::Game::Initialize(argc, argv);
	//-------------------//
	glViewport(0, 0, sf::Config::windowWidth, sf::Config::windowHeight);
	sf::Camera::aspectRatio = (float)sf::Config::windowWidth / (float)sf::Config::windowHeight;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		if (deltaTimeLock)
		{
			currentFrameTime = lastFrameTime = glfwGetTime();
			deltaTimeLock = false;
		}
		else
			currentFrameTime = glfwGetTime();
		deltaTime = currentFrameTime - lastFrameTime;

		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//-------------------//
		sf::Game::OnUpdate(deltaTime, gameTime);
		//-------------------//

		sf::Camera::boundCamera->ComputeMatrices();

		gameTime += deltaTime;

		sf::Skybox::Draw();
		sf::Mesh::DrawAll();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		sf::Input::FrameEnd();
		/* Poll for and process events */
		glfwPollEvents();

		lastFrameTime = currentFrameTime;
	}

	//-------------------//
	sf::Game::Terminate();
	//-------------------//

	glfwTerminate();
	return 0;
}