#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include "Model.h"

#define BACKGROUND_COLOR 1.0
#define MSAA_COUNT 8

float time = 0.0;
double lastFrameTime = 0.0;
double currentFrameTime = 0.0;
double deltaTime = 0.0;

#include "user/sfGame.h"
#include "Input.inl"

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_SAMPLES, MSAA_COUNT);

	window = glfwCreateWindow(1280, 720, "sf", NULL, NULL);
	//window = glfwCreateWindow(1920, 1080, "sf", glfwGetPrimaryMonitor(), NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	/* INPUT BINDINGS */
	glfwSetCursorPosCallback(window, Input::OnMouseMoved);
	glfwSetScrollCallback(window, Input::OnScroll);
	glfwSetKeyCallback(window, Input::OnKey);
	//glfwSetKeyCallback(window, keyCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context (GLAD)" << std::endl;
		return -1;
	}

	// Get GPU info and supported OpenGL version
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "OpenGL version supported " << glGetString(GL_VERSION) << std::endl;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glClearColor(BACKGROUND_COLOR, BACKGROUND_COLOR, BACKGROUND_COLOR, 1);

	//-------------------//
	Game::Initialize();
	//-------------------//

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		currentFrameTime = glfwGetTime();
		deltaTime = currentFrameTime - lastFrameTime;

		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//-------------------//
		Game::OnUpdate(deltaTime, time);
		//-------------------//

		time += deltaTime;
		Model::DrawAll();
		//ModelReference::DrawAll();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		lastFrameTime = currentFrameTime;
	}

	glfwTerminate();
	return 0;
}