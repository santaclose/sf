#include "Scene.h"

#include "Entity.h"
#include <Components/Base.h>

std::unordered_set<sf::Scene*> sf::Scene::scenes;

entt::registry& sf::Scene::GetRegistry()
{
	return m_Registry;
}

sf::Scene::Scene()
{
	scenes.insert(this);
}

sf::Scene::~Scene()
{
	scenes.erase(this);
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