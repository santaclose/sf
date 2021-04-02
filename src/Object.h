#pragma once
#include "Entity.h"

namespace sf {

	class Object : public Entity
	{
		float m_scale = 1.0;

	protected:
		glm::mat4 m_transformMatrix = glm::mat4(1.0);
		void UpdateTransformMatrix();

	public:
		void SetScale(float newScale);
		float GetScale();
	};
}
