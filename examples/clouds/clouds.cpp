#include "../../user/Game.h"
#include "Cloud.h"
#include <GLFW/glfw3.h>
#include <vector>
#include "../../src/Texture.h"
#include "../../src/Material.h"
#include "../../src/Model.h"
#include "../../src/ModelReference.h"
#include "../../src/Math.h"
#include "../../src/Camera.h"
#include "../../src/Input.h"

#define PI 3.14159265358979323
#define SENSITIVITY 0.007

#define CLOUD_COUNT 20
#define MODEL_COUNT_PER_CLOUD 30
#define SPAWN_RANGE 400.0

#define MOUSE_SENSITIVITY 1.0

namespace User
{
	Camera* theCamera;

	Model sphereInstance;
	Shader theShader;

	Material theMaterial;
	ModelReference* spheres;

	int selectedCloud = 0;
	glm::vec3 cursor = glm::vec3(0.0, 0.0, 0.0);

	glm::vec3 materialColor = glm::vec4(0.6, 0.6, 0.6, 0.2);
	glm::vec3 windDirection = glm::vec3(1.0, 0.0, 0.0);

	Cloud* clouds;

	void Game::Initialize()
	{
		CameraSpecs cs;
		cs.aspectRatio = 1.77777777;
		cs.fieldOfView = glm::radians(40.0f);
		cs.nearClippingPlane = 0.1f;
		cs.farClippingPlane = 1000.0f;
		theCamera = new Camera(cs);

		glPolygonMode(GL_FRONT, GL_POINT);
		theShader.CreateFromFiles("examples/clouds/vertex.shader", "examples/clouds/fragment.shader");

		theMaterial.CreateFromShader(&theShader);
		theMaterial.SetUniform("icolor", &materialColor.x, Material::UniformType::_Color);

		sphereInstance.CreateFromFile("examples/clouds/icosphere.gltf");
		sphereInstance.SetMaterial(&theMaterial);
		sphereInstance.SetPosition(glm::vec3(0.0, 0.0, 200.0));

		clouds = new Cloud[CLOUD_COUNT];
		for (int i = 0; i < CLOUD_COUNT; i++)
		{
			clouds[i].Create(MODEL_COUNT_PER_CLOUD, sphereInstance, glm::vec3(-50.0 + i * 5.0, 0.0, 0.0));
		}

		theCamera->SetPosition(0.0, 0.0, 100.0);
		theCamera->LookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	}

	void Game::Terminate()
	{
		delete[] spheres;
		delete theCamera;
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		/*if (Math::Random() > 0.99)
		{
			int index = Math::RandomInt(CLOUD_COUNT);
			clouds[index].Move(glm::vec3(Math::Random() * 20.0 - 10.0, 0.0, 0.0));
		}*/

		if (Input::KeyDown(Input::KeyCode::M))
			glPolygonMode(GL_FRONT, GL_POINT);
		else if (Input::KeyDown(Input::KeyCode::N))
			glPolygonMode(GL_FRONT, GL_LINE);
		else if (Input::KeyDown(Input::KeyCode::B))
			glPolygonMode(GL_FRONT, GL_FILL);
		else if (Input::KeyDown(Input::KeyCode::Right))
		{
			selectedCloud++;
			if (selectedCloud == CLOUD_COUNT)
				selectedCloud = 0;
			cursor = clouds[selectedCloud].GetPosition();
		}
		else if (Input::KeyDown(Input::KeyCode::Left))
		{
			selectedCloud--;
			if (selectedCloud < 0)
				selectedCloud = CLOUD_COUNT - 1;
			cursor = clouds[selectedCloud].GetPosition();
		}


		cursor += glm::vec3(Input::MousePosDeltaX() * MOUSE_SENSITIVITY, -Input::MousePosDeltaY() * MOUSE_SENSITIVITY, 0.0);
		clouds[selectedCloud].Move(cursor);

		for (int i = 0; i < CLOUD_COUNT; i++)
		{
			clouds[i].OnUpdate(deltaTime);
		}
	}
}