#include "Entity.h"

#include <Components/Base.h>

sf::Entity::Entity(entt::entity handle, Scene* scene)
{
	m_EntityHandle = handle;
	m_Scene = scene;
}

void sf::Entity::SetEnabled(bool value)
{
	m_Scene->GetRegistry().get<Base>(m_EntityHandle).isEntityEnabled = value;
}
