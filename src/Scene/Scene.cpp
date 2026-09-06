#include "Scene.h"

#include "Entity.h"
#include <Components/Base.h>

entt::registry& sf::Scene::GetRegistry()
{
	return m_Registry;
}

sf::Scene::Scene()
{
	std::unordered_set<Scene*>& scenes = GetScenes();
	scenes.insert(this);
}

sf::Scene::~Scene()
{
	std::unordered_set<Scene*>& scenes = GetScenes();
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

std::unordered_set<sf::Scene*>& sf::Scene::GetScenes()
{
	static std::unordered_set<Scene*> instances;
	return instances;
}