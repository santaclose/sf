#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include "Vertex.h"
#include "Model.h"
#include "Shader.h"
#include "Camera.h"

#define BACKGROUND_COLOR 1.0
#define MSAA_COUNT 4

double lastFrameTime = 0.0;
double currentFrameTime = 0.0;
double deltaTime = 0.0;

float shipSpeed = 5.0;
glm::fquat targetShipRotation;
#include "Input.inl"

glm::vec3 operator/(const glm::vec3& v, const float& f)
{
	return glm::vec3(v.x / f, v.y / f, v.z / f);
}
glm::vec3 operator*(const glm::vec3& v, const float& f)
{
	return glm::vec3(v.x * f, v.y * f, v.z * f);
}
float RandomValue()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

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

	glfwSetCursorPosCallback(window, Input::OnMouseMoved);
	glfwSetScrollCallback(window, Input::OnScroll);
	//glfwSetKeyCallback(window, keyCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context (GLAD)" << std::endl;
		return -1;
	}

	// Get GPU info and supported OpenGL version
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "OpenGL version supported " << glGetString(GL_VERSION) << std::endl;

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClearColor(BACKGROUND_COLOR, BACKGROUND_COLOR, BACKGROUND_COLOR, 1);

	Shader theShader("res/shaders/default.vert", "res/shaders/default.frag");
	theShader.Bind();

	Camera theCamera(1.7777777777777, 95.0, 0.1, 1000.0);
	//theCamera.SetPosition(0.0, 3.0, 5.0);
	//theCamera.SendMatrixToShader(theShader);

	targetShipRotation = glm::fquat(1.0,0.0,0.0,0.0);

	Model ship;
	float time = 0.0;
	ship.LoadFromOBJ("user/64/Wolfen.obj", 0.3);
	Model* monkeys = new Model[100];
	for (int i = 0; i < 100; i++)
	{
		monkeys[i].LoadFromOBJ("user/64/Butterfly Fighter.obj", RandomValue() * 20.0 + 1.0);
		monkeys[i].SetPosition(glm::vec3((RandomValue() - 0.5) * 100.0, (RandomValue() - 0.5) * 50.0, (float)(i * 15)));
		monkeys[i].SetRotation(glm::fquat(glm::vec3(0.0, 0.0, glm::radians(RandomValue() * 360.0))));
	}

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		currentFrameTime = glfwGetTime();
		deltaTime = currentFrameTime - lastFrameTime;

		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//-------------//
		ship.SetPosition(ship.GetPosition() + ship.Forward() * shipSpeed * deltaTime);
		//ship.SetRotation(glm::mix(ship.GetRotation(), targetShipRotation, (float)(deltaTime * 3.0)));
		ship.SetRotation(glm::slerp(ship.GetRotation(), targetShipRotation, (float)(deltaTime * 3.0)));
		theCamera.SetPosition(glm::mix(theCamera.GetPosition(), ship.GetPosition() - (ship.Forward() * 4.0) + (ship.Up()), (float)(deltaTime * 2.0)));
		theCamera.LookAt(ship.GetPosition(), ship.Up());

		for (int i = 0; i < 100; i++)
		{
			monkeys[i].SetRotation(monkeys[i].GetRotation() * glm::fquat(glm::vec3(0, 0, sqrt(time) * 0.00001 * i)));
		}
		time += deltaTime;
		//-------------//
		Model::drawAll(theShader);
		theCamera.SendMatrixToShader(theShader);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		lastFrameTime = currentFrameTime;
	}

	glfwTerminate();
	delete[] monkeys;
	return 0;
}