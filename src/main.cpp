#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include "Camera.h"
#include "Model.h"
#include "Skybox.h"
#include "Input.h"

#define BACKGROUND_COLOR 1.0
#define MSAA_COUNT 8

float gameTime = 0.0;
double lastFrameTime = 0.0;
double currentFrameTime = 0.0;
double deltaTime = 0.0;
bool deltaTimeLock = true;

#include "../user/Game.h"
//#include "Input.inl"


void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	Input::UpdateMousePosition(xpos, ypos);
}

void mouse_button_callback(GLFWwindow*, int button, int action, int mods)
{
	Input::UpdateMouseButtons(button, action);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Input::UpdateKeyboard(key, action);
}
void character_callback(GLFWwindow* window, unsigned int codepoint)
{
	Input::UpdateCharacter(codepoint);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Input::UpdateMouseScroll(xoffset, yoffset);
}


unsigned int windowWidth;
unsigned int windowHeight;

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_SAMPLES, MSAA_COUNT);

	//windowWidth = 1920;
	//windowHeight = 1080;
	windowWidth = 1280;
	windowHeight = 720;
	window = glfwCreateWindow(windowWidth, windowHeight, "sf", NULL, NULL);
	//window = glfwCreateWindow(windowWidth, windowHeight, "sf", glfwGetPrimaryMonitor(), NULL);

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

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glClearColor(BACKGROUND_COLOR, BACKGROUND_COLOR, BACKGROUND_COLOR, 0.0);

	//-------------------//
	User::Game::Initialize();
	//-------------------//
	glViewport(0, 0, windowWidth, windowHeight);

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
		User::Game::OnUpdate(deltaTime, gameTime);
		//-------------------//

		Camera::boundCamera->ComputeMatrices();

		gameTime += deltaTime;
		
		Model::DrawAll();
		Skybox::Draw();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		Input::FrameEnd();
		/* Poll for and process events */
		glfwPollEvents();

		lastFrameTime = currentFrameTime;
	}

	//-------------------//
	User::Game::Terminate();
	//-------------------//

	glfwTerminate();
	return 0;
}