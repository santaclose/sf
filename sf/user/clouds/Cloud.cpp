#include "Cloud.h"
#include "../../Math.h"

#define PI 3.14159265358979323

namespace User
{
	void Cloud::Create(int modelCount, Model& model, const glm::vec3& position)
	{
		entity.SetPosition(position);
		models = new ModelReference[modelCount];

		float currentScale = 3.0;
		models[0].CreateFomModel(model);
		models[0].SetPosition(entity.GetPosition());
		models[0].SetScale(currentScale);
		currentScale -= 0.1;
		for (int i = 1; i < modelCount; i++)
		{
			int targetSphere = Math::RandomInt(i);
			float randomAngle = Math::Random() * PI + PI;
			glm::vec3 orientation = glm::vec3(cos(randomAngle), sin(randomAngle), 0.0);
			float offset = models[targetSphere].GetScale() / 2.0 + currentScale - Math::Random() * currentScale;

			models[i].CreateFomModel(model);
			models[i].SetPosition(models[targetSphere].GetPosition() + orientation * offset);
			models[i].SetScale(currentScale * (Math::Random() / 2.0 + 1.0));

			currentScale *= 0.99;

			joints.emplace_back();
			joints.back().a = &entity;
			joints.back().b = &models[i];
			joints.back().stiffness = 0.3;
			joints.back().distance = glm::distance(models[i].GetPosition(), entity.GetPosition());

			joints.emplace_back();
			joints.back().a = &models[targetSphere];
			joints.back().b = &models[i];
			joints.back().stiffness = 0.3;
			joints.back().distance = glm::distance(models[i].GetPosition(), entity.GetPosition());
		}
	}
	Cloud::~Cloud()
	{
		delete[] models;
	}
	void Cloud::OnUpdate(float deltaTime)
	{
		for (Joint& j : joints)
		{
			float currentDistance = glm::distance(j.a->GetPosition(), j.b->GetPosition());
			if (j.a != &entity)
				j.a->SetPosition(glm::mix(j.a->GetPosition(), j.b->GetPosition(), deltaTime * (currentDistance - j.distance) * j.stiffness));
			if (j.b != &entity)
				j.b->SetPosition(glm::mix(j.b->GetPosition(), j.a->GetPosition(), deltaTime * (currentDistance - j.distance) * j.stiffness));
			//j.a->SetPosition(j.a->GetPosition() + direction * (currentDistance - j.distance) * j.stiffness);
			//j.b->SetPosition(j.b->GetPosition() - direction * (currentDistance - j.distance) * j.stiffness);
		}
	}
	void Cloud::Move(const glm::vec3& newPosition)
	{
		entity.SetPosition(newPosition);
	}
	const glm::vec3& Cloud::GetPosition()
	{
		return entity.GetPosition();
	}
}