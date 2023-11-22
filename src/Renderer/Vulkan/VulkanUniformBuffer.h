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
		size_t bufferSize;

		void Create(size_t bufferSize);
		void Destroy();

		void Write(VkDescriptorSet* descriptorSetPerFrame, size_t binding)
		{
			for (size_t i = 0; i < VulkanDisplay::MaxFramesInFlight(); i++)
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = buffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = bufferSize;
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = descriptorSetPerFrame[i];
				descriptorWrite.dstBinding = binding;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &bufferInfo;
				vkUpdateDescriptorSets(VulkanDisplay::Device(), 1, &descriptorWrite, 0, nullptr);
			}
		}

		template<typename T>
		inline void Update(const T& object)
		{
			memcpy(buffersMapped[VulkanDisplay::CurrentFrameInFlight()], &object, sizeof(object));
		}
	};
}