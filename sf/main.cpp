#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include "Model.h"
#include "Shader.h"
#include "Camera.h"
#include "Math.h"
#include "Texture.h"
#include "ModelReference.h"

#define BACKGROUND_COLOR 1.0
#define MSAA_COUNT 8

float time = 0.0;
double lastFrameTime = 0.0;
double currentFrameTime = 0.0;
double deltaTime = 0.0;

float currentZoom = 5.0;
glm::vec3 targetModelRotation(0.0, 0.0, 0.0);
Camera theCamera(1.7777777777777, 90.0, 0.1, 10000.0);
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

	Shader pbrShader("res/shaders/pbrV.shader", "res/shaders/gpbrF.shader");

	Texture alb("user/revolver/Cerberus_A.tga", Texture::Type::ColorData);
	Texture nor("user/revolver/Cerberus_N.tga", Texture::Type::NonColorData);
	Texture rou("user/revolver/Cerberus_R.tga", Texture::Type::NonColorData);
	Texture met("user/revolver/Cerberus_M.tga", Texture::Type::NonColorData);

	Material revMat(&pbrShader);
	revMat.SetUniform("albedoTexture", &alb, Material::UniformType::_Texture);
	revMat.SetUniform("normalTexture", &nor, Material::UniformType::_Texture);
	revMat.SetUniform("roughnessTexture", &rou, Material::UniformType::_Texture);
	revMat.SetUniform("metalnessTexture", &met, Material::UniformType::_Texture);

	Texture balb("user/64/butterfly/Butterfly Fighter_pink_BaseColor.png", Texture::Type::ColorData);
	Texture bnor("user/64/butterfly/Butterfly Fighter_pink_Normal.png", Texture::Type::NonColorData);
	Texture brou("user/64/butterfly/Butterfly Fighter_pink_Roughness.png", Texture::Type::NonColorData);
	Texture bmet("user/64/butterfly/Butterfly Fighter_pink_Metallic.png", Texture::Type::NonColorData);

	Material butMat(&pbrShader);
	butMat.SetUniform("albedoTexture", &balb, Material::UniformType::_Texture);
	butMat.SetUniform("normalTexture", &bnor, Material::UniformType::_Texture);
	butMat.SetUniform("roughnessTexture", &brou, Material::UniformType::_Texture);
	butMat.SetUniform("metalnessTexture", &bmet, Material::UniformType::_Texture);

	Model model;
	model.CreateFromOBJ("user/revolver/untitled.obj", 3.0, true);
	model.SetMaterial(&revMat);

	Model lBut;
	lBut.CreateFromOBJ("user/64/Butterfly Fighter.obj", 1.0, true);
	lBut.SetMaterial(&butMat);

	ModelReference rBut;
	rBut.CreateFomModel(lBut);

	lBut.SetPosition(-4.0, 0.0, 0.0);
	rBut.SetPosition( 4.0, 0.0, 0.0);

	theCamera.SetPosition(0.0, 0.0, 5.0);
	theCamera.SetRotation(glm::vec3(0.0, 0.0, 0.0));

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		currentFrameTime = glfwGetTime();
		deltaTime = currentFrameTime - lastFrameTime;

		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//-------------//
		glm::fquat targetRotationQuat = glm::slerp(model.GetRotation(), glm::fquat(targetModelRotation), (float)(deltaTime * 7.0));
		model.SetRotation(targetRotationQuat);
		lBut.SetRotation(glm::conjugate(targetRotationQuat));
		rBut.SetRotation(targetRotationQuat);
		theCamera.SetPosition(0.0, 0.0, currentZoom);
		//-------------//
		time += deltaTime;
		Model::DrawAll();
		ModelReference::DrawAll();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		lastFrameTime = currentFrameTime;
	}

	glfwTerminate();
	return 0;
}