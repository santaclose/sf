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
		VkBuffer buffer;
		VkDeviceMemory bufferMemory;
		void* bufferMapped;
		size_t bufferSize;
		VulkanShaderBufferType bufferType;

		void Create(VulkanShaderBufferType type, size_t bufferSize);
		void Destroy();

		void Write(VkDescriptorSet descriptorSet, size_t binding)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = bufferSize;
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = bufferType == VulkanShaderBufferType::UniformBuffer ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			vkUpdateDescriptorSets(VulkanDisplay::Device(), 1, &descriptorWrite, 0, nullptr);
		}

		template<typename T>
		inline void Update(const T& object)
		{
			memcpy(bufferMapped, &object, sizeof(object));
		}

		inline void Update(const void* buffer, size_t byteCount)
		{
			memcpy(bufferMapped, buffer, byteCount);
		}
	};
}