#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "VulkanUtils.h"

namespace sf::Renderer
{
	enum VulkanShaderBufferType
	{
		UniformBuffer = 0,
		StorageBuffer = 1
	};
	struct VulkanShaderBuffer
	{
		VkBuffer buffers[MAX_FRAMES_IN_FLIGHT];
		VkDeviceMemory buffersMemory[MAX_FRAMES_IN_FLIGHT];
		void* buffersMapped[MAX_FRAMES_IN_FLIGHT];
		size_t bufferSize;
		VulkanShaderBufferType bufferType;
		bool inflightAware;

		void Create(VulkanShaderBufferType type, bool inflightAware, size_t bufferSize);
		void Destroy();

		void Write(VkDescriptorSet* descriptorSetPerFrame, size_t binding)
		{
			for (size_t i = 0; i < VulkanDisplay::MaxFramesInFlight(); i++)
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = inflightAware ? buffers[i] : buffers[0];
				bufferInfo.offset = 0;
				bufferInfo.range = bufferSize;
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = descriptorSetPerFrame[i];
				descriptorWrite.dstBinding = binding;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = bufferType == VulkanShaderBufferType::UniformBuffer ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &bufferInfo;
				vkUpdateDescriptorSets(VulkanDisplay::Device(), 1, &descriptorWrite, 0, nullptr);
			}
		}

		template<typename T>
		inline void Update(const T& object)
		{
			memcpy(buffersMapped[inflightAware ? VulkanDisplay::CurrentFrameInFlight() : 0], &object, sizeof(object));
		}

		inline void Update(const void* buffer, size_t byteCount)
		{
			memcpy(buffersMapped[inflightAware ? VulkanDisplay::CurrentFrameInFlight() : 0], buffer, byteCount);
		}
	};
}