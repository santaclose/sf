#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include "Vertex.h"
#include "Model.h"
#include "Shader.h"
#include "Camera.h"
#include "Math.h"
#include "Texture.h"

#define BACKGROUND_COLOR 0.0
#define MSAA_COUNT 8

float time = 0.0;
double lastFrameTime = 0.0;
double currentFrameTime = 0.0;
double deltaTime = 0.0;

float shipSpeed = 5.0;
glm::fquat targetShipRotation(1.0, 0.0, 0.0, 0.0);
Camera theCamera(1.7777777777777, 95.0, 0.1, 1000.0);
Camera lookBackCamera(1.7777777777777, 95.0, 0.1, 1000.0);
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

	Shader theShader("res/shaders/defaultV.shader", "res/shaders/defaultF.shader");
	Shader noiseShader("user/noiseV.shader", "user/noiseF.shader");
	Shader colorShader("user/randomColorsV.shader", "user/randomColorsF.shader");

	Texture shipTexture("user/64/wolfen/Wolfen_plate_BaseColor.png", Texture::Type::Albedo);
	Texture shipNormalmap("user/64/wolfen/Wolfen_plate_Normal.png", Texture::Type::NormalMap);

	Texture butterflyTexture("user/64/butterfly/Butterfly Fighter_pink_AlbedoTransparency.png", Texture::Type::Albedo);
	Texture butterflyNormalmap("user/64/butterfly/Butterfly Fighter_pink_Normal.png", Texture::Type::NormalMap);

	Material shipMaterial(&theShader);
	shipMaterial.SetUniform("albedo", &shipTexture, Material::UniformType::_Texture);
	shipMaterial.SetUniform("normalMap", &shipNormalmap, Material::UniformType::_Texture);

	Material butterflyMaterial(&theShader);
	butterflyMaterial.SetUniform("albedo", &butterflyTexture, Material::UniformType::_Texture);
	butterflyMaterial.SetUniform("normalMap", &butterflyNormalmap, Material::UniformType::_Texture);

	Model ship;
	ship.CreateFromOBJ("user/64/wolfen/Wolfen.obj", 0.3, true);
	//ship.SetShader(&theShader);
	ship.SetMaterial(&shipMaterial);

	Material colorsMaterial(&colorShader);
	Material noiseMaterial(&noiseShader);

	Model* monkeys = new Model[400];
	for (int i = 0; i < 400; i++)
	{
		monkeys[i].CreateFromOBJ("user/64/Butterfly Fighter.obj", Math::Random() * 20.0 + 1.0, true);
		//monkeys[i].CreateFromOBJ("user/monkey.obj", RandomValue() * 20.0 + 1.0);
		monkeys[i].SetPosition(glm::vec3((Math::Random() - 0.5) * 100.0, (Math::Random() - 0.5) * 50.0, (float)(i * -15)));
		monkeys[i].SetRotation(glm::fquat(glm::vec3(0.0, glm::radians(180.0), glm::radians(Math::Random() * 360.0))));
		float choice = Math::Random();
		//if (choice < 0.333333)
			monkeys[i].SetMaterial(&butterflyMaterial);
		//else if (choice < 0.666666)
		//	monkeys[i].SetMaterial(&colorsMaterial);
		//else
		//	monkeys[i].SetMaterial(&noiseMaterial);
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
		ship.SetRotation(glm::slerp(ship.GetRotation(), targetShipRotation, (float)(deltaTime * 7.0)));
		theCamera.SetPosition(glm::mix(theCamera.GetPosition(), ship.GetPosition() - (ship.Forward() * 4.0) + (ship.Up()), (float)(deltaTime * 2.0)));
		theCamera.LookAt(ship.GetPosition(), ship.Up());
		lookBackCamera.SetPosition(ship.GetPosition() + ship.Forward() * 4.0);
		lookBackCamera.LookAt(ship.GetPosition(), ship.Up());

		for (int i = 0; i < 400; i++)
		{
			monkeys[i].SetRotation(monkeys[i].GetRotation() * glm::fquat(glm::vec3(0, 0, sqrt(time) * 0.00001 * i)));
		}
		time += deltaTime;
		//-------------//
		Model::drawAll();

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