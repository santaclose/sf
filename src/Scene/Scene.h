#pragma once

#include <entt.hpp>

namespace sf {

	class Entity;

	class Scene
	{
		friend Entity;

	public:
		static Scene* activeScene;
		entt::registry& GetRegistry();

		Scene();
		~Scene();

		Entity CreateEntity();
		void DestroyEntity(Entity entity);

		void SetActive();

	private:
		entt::registry m_Registry;
	};

}