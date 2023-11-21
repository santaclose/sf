#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "VulkanUtils.h"

namespace sf::Renderer
{
	struct VulkanUniformBuffer
	{
		VkBuffer buffers[MAX_FRAMES_IN_FLIGHT];
		VkDeviceMemory buffersMemory[MAX_FRAMES_IN_FLIGHT];
		void* buffersMapped[MAX_FRAMES_IN_FLIGHT];

		void Create(size_t bufferSize);
		void Destroy();

		template<typename T>
		inline void Update(const T& object)
		{
			memcpy(buffersMapped[VulkanDisplay::CurrentFrameInFlight()], &object, sizeof(object));
		}

		inline VkBuffer GetVkBuffer(uint32_t frame = ~0U)
		{
			return frame == ~0U ? buffers[VulkanDisplay::CurrentFrameInFlight()] : buffers[frame];
		}
	};
}