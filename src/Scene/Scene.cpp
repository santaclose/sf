#include "Scene.h"

#include "Entity.h"
#include <Components/Base.h>

sf::Scene* sf::Scene::activeScene = nullptr;

entt::registry& sf::Scene::GetRegistry()
{
	return m_Registry;
}

sf::Scene::Scene()
{
	if (activeScene == nullptr)
		activeScene = this;
}

sf::Scene::~Scene()
{
}

sf::Entity sf::Scene::CreateEntity()
{
	Entity entity = { m_Registry.create(), this };
	auto& base = entity.AddComponent<Base>(entity);
	return entity;
}

void sf::Scene::DestroyEntity(Entity entity)
{
	m_Registry.destroy(entity);
}

void sf::Scene::SetActive()
{
	activeScene = this;
}
