#ifdef SF_USE_VULKAN

#include "VulkanShaderBuffer.h"
#include "VulkanDisplay.h"

void sf::Renderer::VulkanShaderBuffer::Create(VulkanShaderBufferType type, size_t bufferSize)
{
	this->bufferSize = bufferSize;
	this->bufferType = type;

	VulkanUtils::CreateBuffer(bufferSize,
		type == VulkanShaderBufferType::StorageBuffer ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		buffer, bufferMemory);
	vkMapMemory(VulkanDisplay::Instance->device.device, bufferMemory, 0, bufferSize, 0, &bufferMapped);
}

void sf::Renderer::VulkanShaderBuffer::Destroy()
{
	vkDestroyBuffer(VulkanDisplay::Instance->disp.device, buffer, nullptr);
	vkFreeMemory(VulkanDisplay::Instance->disp.device, bufferMemory, nullptr);
}

#endif