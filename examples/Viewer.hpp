#pragma once

#include <glm/glm.hpp>
#include <imgui.h>

#include <Input.h>
#include <Math.hpp>
#include <Scene/Entity.h>

#include <Components/Transform.h>
#include <Components/Camera.h>

#define MOUSE_SENSITIVITY 0.003
#define SCROLL_SENSITIVITY 0.12
#define PAN_SENSITIVITY 0.1f
#define GIMBAL_MOVEMENT_SPEED 7.5
#define GIMBAL_ROTATION_SPEED 15.0f
#define MIN_CAMERA_DISTANCE 0.5f

namespace sf::ExampleViewer
{
	Entity gimbal, cameraObject;
	glm::vec3 targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);
	float cameraDistance = 3.0;

	void Initialize(Scene& scene)
	{
		gimbal = scene.CreateEntity();
		cameraObject = scene.CreateEntity();

		Transform& gimbalTransform = gimbal.AddComponent<Transform>();
		gimbalTransform.position = glm::vec3(0.0, 0.0, 0.0);
		gimbalTransform.LookAt(glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));

		Transform& cameraTransform = cameraObject.AddComponent<Transform>();
		cameraTransform.position = glm::vec3(0.0, 0.0, cameraDistance);
		cameraTransform.LookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		cameraObject.AddComponent<Camera>();
	}

	void UpdateCamera(float deltaTime)
	{
		if (Input::Key(Input::KeyCode::F))
			gimbal.GetComponent<Transform>().position = glm::vec3(0.0f);
		if (Input::Key(Input::KeyCode::E))
			gimbal.GetComponent<Transform>().position += cameraObject.GetComponent<Transform>().Up() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;
		if (Input::Key(Input::KeyCode::Q))
			gimbal.GetComponent<Transform>().position -= cameraObject.GetComponent<Transform>().Up() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;
		if (Input::Key(Input::KeyCode::W))
			gimbal.GetComponent<Transform>().position += cameraObject.GetComponent<Transform>().Forward() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;
		if (Input::Key(Input::KeyCode::S))
			gimbal.GetComponent<Transform>().position -= cameraObject.GetComponent<Transform>().Forward() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;
		if (Input::Key(Input::KeyCode::D))
			gimbal.GetComponent<Transform>().position += cameraObject.GetComponent<Transform>().Right() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;
		if (Input::Key(Input::KeyCode::A))
			gimbal.GetComponent<Transform>().position -= cameraObject.GetComponent<Transform>().Right() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;

		cameraDistance -= glm::sqrt(cameraDistance) * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * (Input::MouseScrollUp() ? SCROLL_SENSITIVITY : 0.0f);
		cameraDistance += glm::sqrt(cameraDistance) * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * (Input::MouseScrollDown() ? SCROLL_SENSITIVITY : 0.0f);
		cameraDistance = glm::max(MIN_CAMERA_DISTANCE, cameraDistance);

		if (Input::MouseButton(2))
		{
			if (Input::Key(Input::KeyCode::LeftShift))
				gimbal.GetComponent<Transform>().position +=
				-cameraObject.GetComponent<Transform>().Right() * GIMBAL_MOVEMENT_SPEED * MOUSE_SENSITIVITY * PAN_SENSITIVITY * Input::MousePosDeltaX() * glm::sqrt(cameraDistance) +
				cameraObject.GetComponent<Transform>().Up() * GIMBAL_MOVEMENT_SPEED * MOUSE_SENSITIVITY * PAN_SENSITIVITY * Input::MousePosDeltaY() * glm::sqrt(cameraDistance);
			else
			{
				targetGimbalRotation.y -= Input::MousePosDeltaX() * MOUSE_SENSITIVITY;
				targetGimbalRotation.x += Input::MousePosDeltaY() * MOUSE_SENSITIVITY;
			}
		}

		targetGimbalRotation.x = glm::clamp(targetGimbalRotation.x, -Math::Pi * 0.499f, Math::Pi * 0.499f);

		gimbal.GetComponent<Transform>().rotation = glm::slerp(gimbal.GetComponent<Transform>().rotation, glm::quat(targetGimbalRotation), deltaTime * GIMBAL_ROTATION_SPEED);
		cameraObject.GetComponent<Transform>().position = glm::vec3(gimbal.GetComponent<Transform>().position + gimbal.GetComponent<Transform>().Forward() * cameraDistance);
		cameraObject.GetComponent<Transform>().LookAt(gimbal.GetComponent<Transform>().position, glm::vec3(0.0, 1.0, 0.0));
	}

	void ImGuiCall()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Camera"))
			{
				if (ImGui::MenuItem("Center", "F"))
					gimbal.GetComponent<Transform>().position = glm::vec3(0.0f);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
}