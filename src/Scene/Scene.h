#pragma once

#include <entt.hpp>
#include <unordered_set>

namespace sf {

	class Entity;

	class Scene
	{
		friend Entity;

	public:
		static std::unordered_set<Scene*> scenes;
		entt::registry& GetRegistry();

		Scene();
		~Scene();

		Entity CreateEntity();
		void DestroyEntity(Entity entity);

		void SetRenderTargetVisibility(uint32_t value);
		uint32_t GetRenderTargetVisibility();

	private:
		entt::registry m_Registry;
	};

}