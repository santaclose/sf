#pragma once
#include "../../src/ModelReference.h"
#include "../../src/Model.h"

namespace User
{
	struct Joint
	{
		float distance;
		float stiffness;
		Entity* a;
		Entity* b;
	};

	class Cloud
	{
	private:
		std::vector<Joint> joints;
		Entity entity;
		ModelReference* models;
	public:
		void Create(int modelCount, Model& model, const glm::vec3& position);
		~Cloud();
		void OnUpdate(float deltaTime);
		void Move(const glm::vec3& newPosition);
		const glm::vec3& GetPosition();
	};
}