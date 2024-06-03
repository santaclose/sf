#pragma once

#include <entt.hpp>
#include <vector>
#include <unordered_map>

#include "Scene.h"

#include <Renderer/Renderer.h>

namespace sf {

	class Entity
	{
	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;

	public:
		Entity() = default;
		Entity(entt::entity handle, Scene * scene);
		Entity(const Entity & other) = default;

		bool IsEnabled();
		void SetEnabled(bool value);

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			assert(!HasComponent<T>());
			T& component = m_Scene->GetRegistry().emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			Renderer::OnComponentAddedToEntity(*this); // so the camera can be detected
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			assert(HasComponent<T>());
			return m_Scene->GetRegistry().get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->GetRegistry().try_get<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			assert(HasComponent<T>());
			m_Scene->GetRegistry().remove<T>(m_EntityHandle);
		}

		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

		bool operator==(const Entity& other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}
	};

}