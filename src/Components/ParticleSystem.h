#pragma once

#include <MeshData.h>
#include <Components/Transform.h>
#include <glm/glm.hpp>

namespace sf
{
	struct ParticleSystem
	{
		const MeshData* meshData;
		uint32_t material;
		uint32_t particleCount;
		float timeBetweenEmissions;
		uint32_t particlesPerEmission = 1U;
		bool emit = true;

		Transform(*initialTransform)() = nullptr;
	};
}