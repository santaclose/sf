#pragma once

namespace sf
{
	struct SphereCollider
	{
		float radius = 0.5f;
		uint8_t layer = 0;
		uint8_t isColliding = 0;
	};
}