#pragma once

#include <Material.h>
#include <MeshData.h>
#include <Components/Transform.h>
#include <glm/glm.hpp>

namespace sf
{
	struct ParticleSystem
	{
		const MeshData* meshData = nullptr;
		const Material* material = nullptr;
		uint32_t particleCount = 16U;
		float timeBetweenEmissions = 0.1f;
		uint32_t particlesPerEmission = 1U;
		bool emit = true;

		Transform(*initialTransform)() = nullptr;
	};
}